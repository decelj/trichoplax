#include <cmath>
#include <iostream>

#include "material.h"
#include "scene.h"
#include "ray.h"
#include "multi_sample_ray.h"
#include "ilight.h"
#include "common.h"
#include "hit.h"
#include "raytracer.h"
#include "noise.h"

#define GI_ENABLED 1

namespace
{

inline glm::vec3 estimateFresnel(const glm::vec3& reflectance, float cosTheta, float power=5.f)
{
    return reflectance + (1.f - reflectance) * powf(1.f - cosTheta, power);
}

} // anonymous namespace


Material::Material()
    : mBrdf(glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f),
            glm::vec3(0.f), glm::vec3(0.f), 0.f, 1.f)
{
}

Material::Material(const glm::vec3& Ka, const glm::vec3& Ke,
         const glm::vec3& Kd, const glm::vec3& Ks,
         const glm::vec3& Kt, const float Kr, const float ior)
    : mBrdf(Ka, Ke, Kd, Ks, Kt, Kr, ior)
{
}

Material::Material(const Material& other)
    : mBrdf(other.mBrdf)
{
}

Material* Material::clone() const
{
    return new Material(*this);
}

void Material::setAmbient(const glm::vec3& Ka)
{
    mBrdf.Ka = Ka;
}

void Material::setEmissive(const glm::vec3& Ke)
{
    mBrdf.Ke = Ke;
}

void Material::setDiffuse(const glm::vec3& Kd)
{
    mBrdf.Kd = Kd;
}

void Material::setSpecular(const glm::vec3& Ks)
{
    mBrdf.Ks = Ks;
}

void Material::setTransparency(const glm::vec3& Kt)
{
    mBrdf.Kt = Kt;
}

void Material::setShininess(float Kr)
{
    mBrdf.Kr = Kr;
}

void Material::setIor(float Ior)
{
    mBrdf.ior = Ior;
}

void Material::shadeRay(const Raytracer* tracer, const Ray& r, glm::vec4& result) const
{
    // Ambient and emissive
    glm::vec3 opaqueColor = mBrdf.Ka + mBrdf.Ke;
    
    const Scene& scene = Scene::instance();
    const Hit hit(r);

    // TODO: Hit.I is actually V not I, rename
    const glm::vec3& I = hit.I;
    glm::vec3 shadingNormal = hit.N;
    float nDotI = glm::dot(shadingNormal, I);
    const bool didHitBackface = nDotI < 0.f;

    if (didHitBackface)
    {
        shadingNormal = -shadingNormal;
        nDotI = glm::dot(shadingNormal, I);
    }

    glm::vec3 fresnelFactor = estimateFresnel(mBrdf.Ks, nDotI);
    glm::vec3 transmissionColor = (1.f - fresnelFactor) * mBrdf.Kt;
    float transmissionWeight = luminance(transmissionColor);

    // Transmission
    if (transmissionWeight > 0.f && r.type() != Ray::GI)
    {
        Ray refractedRay(Ray::REFRACTED, hit.P, mBrdf.ior);
        if (refractedRay.refract(hit, r.ior()))
        {
            refractedRay.bias(0.001f);
            refractedRay.setDepth(r.depth() + 1);

            glm::vec4 refractionColor(0.f);
            tracer->traceAndShade(refractedRay, refractionColor);

            // TODO: Beer's law
            result += refractionColor * glm::vec4(transmissionColor, transmissionWeight);
        }
        else
        {
            fresnelFactor = glm::vec3(1.f);
            transmissionWeight = 0.f;
        }
    }

    float opaqueWeight = 1.f - transmissionWeight;

    // Reflection
    if (opaqueWeight > 0.f && r.type() != Ray::GI)
    {
        Ray reflected(Ray::REFLECTED);
        r.reflected(hit, reflected);
        reflected.bias(0.001f);
        
        glm::vec4 reflection(0.f, 0.f, 0.f, 0.f);
        tracer->traceAndShade(reflected, reflection);
        result += reflection * glm::vec4(fresnelFactor, 0.f);
    }

    // Simple diffuse + specular
    if (opaqueWeight > 0.f)
    {
        const bool isSpecular = luminance(mBrdf.Ks) > 0.f;
        for (Scene::ConstLightIter it = scene.lightsBegin(); it != scene.lightsEnd(); ++it)
        {
            const ILight* lgt = *it;
            glm::vec3 lightColor = lgt->color();
            // TODO: Technically, for area lighting we should attenuate based on the
            // distance to the sample point on the light, not the center of the light.
            lgt->attenuate(hit.P, lightColor);
            
            // Check for zero contribution
            if (VEC3_IS_REL_ZERO(lightColor))
                continue;
            
            // TODO: This should be more elegant. Maybe construct a ShadowRay type
            //       that's a subclass of MultiSampleRay?
            // TODO: Can we early out if say half the shadow rays are coherent?
            //       Won't work with current non-random area sampling...
            // TODO: We don't support transparent shadow rays :(
            MultiSampleRay shadowRay(Ray::SHADOW, r, lgt->shadowRays());
            shadowRay.setOrigin(hit.P);
            shadowRay.shouldHitBackFaces(true);
            while (lgt->generateShadowRay(shadowRay, tracer->getNoiseGenerator()))
            {
                float nDotL = std::max(glm::dot(hit.N, shadowRay.dir()), 0.f);
                if (nDotL == 0.f)
                {
                    continue;
                }

                if (tracer->traceShadow(shadowRay))
                {
                    continue;
                }

                glm::vec3 specularColor(0.f);
                if(isSpecular)
                {
                    const glm::vec3 H = glm::normalize(shadowRay.dir() + hit.I);
                    float nDotH = std::max(glm::dot(hit.N, H), 0.f);
                    float specularPower = powf(nDotH, mBrdf.Kr);
                    specularColor = mBrdf.Ks * specularPower * lightColor;
                }

                glm::vec3 diffuseWeight = 1.f - specularColor;
                opaqueColor += mBrdf.Kd * lightColor * nDotL * diffuseWeight;
                opaqueColor += specularColor;
            }
            
            opaqueColor /= lgt->shadowRays();
        }
    }
    
#if GI_ENABLED
    if (opaqueWeight > 0.f &&
        scene.renderSettings().GISamples > 0 &&
        r.depth() < tracer->maxDepth())
    {
        glm::vec3 v = hit.N;
        glm::vec3 u;
        if (fabs(v.x) > fabs(v.y))
            u = glm::normalize(glm::vec3(-v.z, 0.f, v.x));
        else
            u = glm::normalize(glm::vec3(0.f, -v.z, v.y));
        glm::vec3 w = glm::normalize(glm::cross(u, v));
        
        Noise& noiseGen = tracer->getNoiseGenerator();
        glm::vec3 giColor(0.f);
        MultiSampleRay giRay(Ray::GI, r, scene.renderSettings().GISamples);
        giRay.setOrigin(hit.P);
        giRay.shouldHitBackFaces(false);
        giRay.incrementDepth();
        giRay.bias(0.001f);
        while (giRay.currentSample())
        {
            float Xi1 = noiseGen.generateNormalizedFloat();
            float Xi2 = noiseGen.generateNormalizedFloat();
            Xi1 = std::min(Xi1, 0.9999f);
            Xi2 = 1.f - glm::clamp(Xi2, 0.0001f, 0.9999f) * 2.f;
            
            float theta = static_cast<float>(M_PI) * Xi1;
            float sqrtU = sqrtf(1.f - Xi2 * Xi2);

            float xs = sqrtU * cosf(theta);
            float ys = sqrtU * sinf(theta);
            float zs = Xi2;
            
            glm::vec4 tmp(0.f);
            giRay.setDir(glm::normalize(xs * u + ys * v + zs * w));
            giRay.setMaxDistance(std::numeric_limits<float>::max());
            TP_ASSERT(glm::dot(hit.N, giRay.dir()) > 0.0);
            tracer->traceAndShade(giRay, tmp);
            
            giColor += glm::vec3(tmp);
            --giRay;
        }

        float invNumSamples = 1.f / static_cast<float>(scene.renderSettings().GISamples);
        giColor *= invNumSamples;
        giColor *= mBrdf.Kd;

        opaqueColor += giColor;
    }
#endif
    
    result += glm::vec4(opaqueColor * opaqueWeight, opaqueWeight);
    result.a = std::min(result.a, 1.f);
}
    
