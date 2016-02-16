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
#include "isampler.h"
#include "sampler_info.h"

#define GI_ENABLED 1

namespace
{

inline glm::vec3 estimateFresnel(const glm::vec3& reflectance, float cosTheta, float power=5.f)
{
    return reflectance + (1.f - reflectance) * powf(1.f - cosTheta, power);
}

inline glm::vec3 computeSurfaceLighting(const Ray& ray, const Hit& hit, const BRDF& brdf,
                                        const glm::vec3& lightColor, const float nDotL,
                                        bool isSpecular)
{
    glm::vec3 specularColor(0.f);
    if(isSpecular)
    {
        const glm::vec3 H = glm::normalize(ray.dir() + hit.V);
        float nDotH = std::max(glm::dot(hit.N, H), 0.f);
        float specularPower = powf(nDotH, brdf.Kr);
        specularColor = brdf.Ks * specularPower * lightColor;
    }

    glm::vec3 diffuseWeight = 1.f - specularColor;

    glm::vec3 outColor = brdf.Kd * lightColor * nDotL * diffuseWeight;
    outColor += specularColor;

    return outColor;
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

void Material::shadeRay(const Raytracer& tracer, const Ray& r, glm::vec4& result) const
{
    // Ambient and emissive
    glm::vec3 opaqueColor = mBrdf.Ka + mBrdf.Ke;
    
    const Scene& scene = Scene::instance();
    const Hit hit(r);

    const float nDotV = glm::dot(hit.N, hit.V);
    glm::vec3 fresnelFactor = estimateFresnel(mBrdf.Ks, nDotV);
    glm::vec3 transmissionColor = (1.f - fresnelFactor) * mBrdf.Kt;
    float transmissionWeight = luminance(transmissionColor);

    // Transmission
#if 1
    if (transmissionWeight > 0.f && r.type() != Ray::GI)
    {
        Ray refractedRay(Ray::REFRACTED, hit.P, mBrdf.ior);
        if (refractedRay.refract(hit, r.ior()))
        {
            refractedRay.bias(scene.renderSettings().bias);
            refractedRay.setDepth(r.depth() + 1);

            glm::vec4 refractionColor(0.f);
            tracer.traceAndShade(refractedRay, refractionColor);

            // TODO: Beer's law
            result += refractionColor * glm::vec4(transmissionColor, transmissionWeight);
        }
        else
        {
            fresnelFactor = glm::vec3(1.f);
            transmissionWeight = 0.f;
        }
    }
#endif

    float opaqueWeight = 1.f - transmissionWeight;

    // Reflection
#if 1
    if (opaqueWeight > 0.f && r.type() != Ray::GI)
    {
        Ray reflected(r, Ray::REFLECTED);
        reflected.reflect(hit, hit.I);
        reflected.bias(scene.renderSettings().bias);
        reflected.setDepth(r.depth() + 1);
        
        glm::vec4 reflection(0.f);
        tracer.traceAndShade(reflected, reflection);
        result += reflection * glm::vec4(fresnelFactor, 0.f);
    }
#endif

    // Diffuse/spec
    if (opaqueWeight > 0.f)
    {
        const bool isSpecular = luminance(mBrdf.Ks) > 0.f;
        for (Scene::ConstLightIter it = scene.lightsBegin(); it != scene.lightsEnd(); ++it)
        {
            opaqueColor += sampleLight(**it, hit, tracer, r.depth(), isSpecular);
        }
    }

#if GI_ENABLED
    if (opaqueWeight > 0.f &&
        scene.renderSettings().GISamples > 0 &&
        r.depth() < tracer.maxDepth())
    {
        Noise& noiseGen = tracer.getNoiseGenerator();
        glm::vec3 giColor(0.f);

        MultiSampleRay giRay(Ray::GI, scene.renderSettings().GISamples);
        giRay.setOrigin(hit.P);
        giRay.shouldHitBackFaces(false);
        giRay.setDepth(r.depth() + 1);
        giRay.bias(scene.renderSettings().bias);

        const unsigned giSequenceNumber = noiseGen.generateGISequenceNumber();
        const glm::mat3 toWorld = generateBasis(hit.N);
        while (giRay.currentSample())
        {
            glm::vec3 cosSample = noiseGen.getGISample(giSequenceNumber, giRay.currentSample() - 1);
            giRay.setDir(toWorld * cosSample);
            giRay.setMaxDistance(std::numeric_limits<float>::max());
            TP_ASSERT(glm::dot(hit.N, giRay.dir()) > 0.0);

            glm::vec4 tmp(0.f);
            tracer.traceAndShade(giRay, tmp);
            
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

glm::vec3 Material::sampleLight(const ILight& light, const Hit& hit,
                                const Raytracer& tracer,
                                unsigned rayDepth, bool isSpecular) const
{
    glm::vec3 result(0.f);
    glm::vec3 lightColor = light.color();
    light.attenuate(hit.P, lightColor);

    // Check for zero contribution
    if (VEC3_IS_REL_ZERO(lightColor))
        return result;

    // TODO: We don't support transparent shadow rays :(
    MultiSampleRay shadowRay(Ray::SHADOW, light.firstPassShadowRays());
    shadowRay.setDepth(rayDepth);
    shadowRay.setOrigin(hit.P);
    shadowRay.bias(light.bias());
    ISampler* lightSampler = light.generateSamplerForPoint(hit.P);

    unsigned lastLitSample = light.firstPassShadowRays();
    while (shadowRay.currentSample())
    {
        lightSampler->generateSample(tracer.getNoiseGenerator(), shadowRay);
        --shadowRay;

        float nDotL = std::max(glm::dot(hit.N, shadowRay.dir()), 0.f);
        if (nDotL == 0.f)
        {
            continue;
        }

        if (tracer.traceShadow(shadowRay))
        {
            continue;
        }

        lastLitSample = shadowRay.currentSample() + 1;
        result += computeSurfaceLighting(
                shadowRay, hit, mBrdf, lightColor, nDotL, isSpecular);
    }

    if (lastLitSample == light.firstPassShadowRays())
    {
        // First pass shadow sample consistent, early out
        result /= (float)light.firstPassShadowRays();
    }
    else
    {
        // Inconsistent samples, must be in penumbra
        // Finish shooting rays to resolve lighting
        shadowRay.setNumberOfSamples(light.secondPassShadowRays());
        while (shadowRay.currentSample())
        {
            lightSampler->generateSample(tracer.getNoiseGenerator(), shadowRay);
            --shadowRay;

            float nDotL = std::max(glm::dot(hit.N, shadowRay.dir()), 0.f);
            if (nDotL == 0.f)
            {
                continue;
            }

            if (tracer.traceShadow(shadowRay))
            {
                continue;
            }

            result += computeSurfaceLighting(
                shadowRay, hit, mBrdf, lightColor, nDotL, isSpecular);
        }

        result /= light.shadowRays();
    }

    delete lightSampler;

    return result;
}

