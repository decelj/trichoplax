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

#define NUM_GI_SAMPLES 50.f
#define DO_GI 1

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
    glm::vec3 color = mBrdf.Ka + mBrdf.Ke;
    
    const Scene* s = Scene::instance();
    const float specularFactor = (mBrdf.Ks.r + mBrdf.Ks.g + mBrdf.Ks.b) / 3.f;
    const bool hasDiffuse = !relEq(mBrdf.Kd.r + mBrdf.Kd.g + mBrdf.Kd.b, 0.f);
    const bool hasSpecular = !relEq(specularFactor, 0.f);
    const Hit hit(r);
    
    float transparencyFactor = (mBrdf.Kt.r + mBrdf.Kt.g + mBrdf.Kt.b) / 3.f;
    float opaqueFactor = 1.f - transparencyFactor;
    
    // Refraction
    if (transparencyFactor > 0.f && r.type() != Ray::GI)
    {
        Ray refracted(Ray::REFRACTED, hit.P, mBrdf.ior);
        if (r.refracted(hit, refracted)) {
            refracted.bias(.01f);
            
            glm::vec4 refraction(0.f, 0.f, 0.f, 0.f);
            tracer->traceAndShade(refracted, refraction);
            result += refraction * glm::vec4(mBrdf.Kt, transparencyFactor);
        } else {
            transparencyFactor = 0.f;
            opaqueFactor = 1.f;
        }
    }

    // Reflection
    if (hasSpecular && opaqueFactor > 0.f && r.type() != Ray::GI)
    {
        Ray reflected(Ray::REFLECTED);
        r.reflected(hit, reflected);
        reflected.bias(.01f);
        
        glm::vec4 reflection(0.f, 0.f, 0.f, 0.f);
        tracer->traceAndShade(reflected, reflection);
        result += reflection * glm::vec4(mBrdf.Ks * opaqueFactor,
                                         specularFactor * opaqueFactor * .5f);
    }
    
    if (hasDiffuse || hasSpecular)
    {
        for (std::vector<ILight*>::const_iterator it = s->lightsBegin(); 
             it != s->lightsEnd(); ++it) {
            const ILight* lgt = *it;
            glm::vec3 lightColor = lgt->getColor();
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
            MultiSampleRay shadow(Ray::SHADOW, r, lgt->shadowRays());
            shadow.setOrigin(hit.P);
            shadow.shouldHitBackFaces(true);
            while (lgt->generateShadowRay(shadow, tracer->getNoiseGenerator()))
            {
                if (!tracer->traceShadow(shadow))
                {
                    float specularPower = 0.f;
                    if(hasSpecular)
                    {
                        specularPower = powf(
                                std::max<float>(glm::dot(lgt->getHalf(shadow.dir(),
                                                                      hit.I),
                                                         hit.N),
                                                0.f),
                                               mBrdf.Kr);
                        specularPower *= specularFactor * .5f;
                        color += mBrdf.Ks * specularPower * lightColor;
                    }
                    
                    // Diffuse
                    if (hasDiffuse)
                    {
                        color += mBrdf.Kd * lightColor *
                                (1.f - std::max<float>((specularFactor / 2.f + specularPower + transparencyFactor),0.f)) *
                                 std::max<float>(glm::dot(hit.N, shadow.dir()), 0.f);
                    }
                }
            }
            
            color *= opaqueFactor / lgt->shadowRays();
        }
    }
    
#if DO_GI
    if (hasDiffuse && r.depth() < tracer->maxDepth())
    {
        glm::vec3 v = hit.N;
        glm::vec3 u;
        if (fabs(v.x) > fabs(v.y))
            u = glm::normalize(glm::vec3(-v.z, 0.f, v.x));
        else
            u = glm::normalize(glm::vec3(0.f, -v.z, v.y));
        glm::vec3 w = glm::normalize(glm::cross(u, v));
        
        Noise& noiseGen = tracer->getNoiseGenerator();
        glm::vec4 giColor(0.f);
        MultiSampleRay giRay(Ray::GI, r, NUM_GI_SAMPLES);
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
            
            giColor += tmp;
            --giRay;
        }
        
        result.r += (giColor.r / NUM_GI_SAMPLES) * mBrdf.Kd.r;
        result.g += (giColor.g / NUM_GI_SAMPLES) * mBrdf.Kd.g;
        result.b += (giColor.b / NUM_GI_SAMPLES) * mBrdf.Kd.b;
    }
#endif
    
    result += glm::vec4(color, opaqueFactor);
    result.a = std::min(result.a, 1.f);
}
    
