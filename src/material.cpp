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


namespace
{

inline glm::vec3 computeSurfaceLighting(const Ray& ray, const Hit& hit, const BRDF& brdf,
                                        const glm::vec3& lightColor, const float nDotL,
                                        bool hasSpecLobe)
{
    glm::vec3 result = lightColor * nDotL;
    return result;
}

} // anonymous namespace


Material::Material()
    : mBrdf(glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f),
            glm::vec3(0.f), 0.f, 1.f, 0.f)
{
}

Material::Material(const glm::vec3& Ka, const glm::vec3& Ke,
         const glm::vec3& Kd, const glm::vec3& Kt,
         float Kr, float roughness, float ior)
    : mBrdf(Ka, Ke, Kd, Kt, Kr, roughness, ior)
{
}

Material::Material(const Material& other)
    : mBrdf(other.mBrdf)
{
}

glm::vec4 Material::shadeRay(const Raytracer& tracer, const Ray& r) const
{
    glm::vec4 result(0.f);

    if (r.depth() >= tracer.maxDepth())
    {
        return result;
    }

    const Scene& scene = Scene::instance();
    const Hit hit(r);
    Noise& noiseGen = tracer.getNoiseGenerator();

    glm::vec3 giRadiance(0.f);
    if (scene.renderSettings().GISamples > 0)
    {
        Ray giRay(Ray::GI);
        giRay.setDepth(r.depth() + 1);
        giRay.setOrigin(hit.P);
        giRay.bias(scene.renderSettings().bias);
        giRay.shouldHitBackFaces(false);

        const unsigned seqNumber = noiseGen.generateGISequenceNumber();
        for (unsigned i = 0; i < scene.renderSettings().GISamples; ++i)
        {
            const glm::vec3& giSample = noiseGen.getGISample(seqNumber, i);
            giRay.setDir(hit.toWorld(giSample));
            giRay.setMaxDistance(std::numeric_limits<float>::max());

            glm::vec4 rayResult;
            tracer.traceAndShade(giRay, rayResult);
            giRadiance.r += rayResult.r * giSample.z;
            giRadiance.g += rayResult.g * giSample.z;
            giRadiance.b += rayResult.b * giSample.z;
        }
        giRadiance = 2.f * (giRadiance / static_cast<float>(scene.renderSettings().GISamples));
    }

    glm::vec3 directLighting(0.f);
    const bool hasSpecLobe = mBrdf.roughness() < .998f;
    for (Scene::ConstLightIter it = scene.lightsBegin(); it != scene.lightsEnd(); ++it)
    {
        directLighting += sampleLight(**it, hit, tracer, r.depth(), hasSpecLobe);
    }

    directLighting *= 1.f / PI;

    result += glm::vec4((directLighting + giRadiance) * mBrdf.Kd(), 0.f);
    result += glm::vec4(mBrdf.Ke(), 0.f);

    result.a = 1.f;
    return result;
}

glm::vec3 Material::sampleLight(const ILight& light, const Hit& hit,
                                const Raytracer& tracer,
                                unsigned rayDepth,
                                bool hasSpecLobe) const
{
    glm::vec3 result(0.f);
    glm::vec3 lightColor = light.color();
    light.attenuate(hit.P, lightColor);

    // Check for zero contribution
    if (VEC3_IS_REL_ZERO(lightColor))
    {
        return result;
    }

    // TODO: We don't support transparent shadow rays :(
    MultiSampleRay shadowRay(Ray::SHADOW, light.shadowRays());
    shadowRay.setDepth(rayDepth);
    shadowRay.setOrigin(hit.P);
    shadowRay.bias(light.bias());
    ISampler* lightSampler = light.generateSamplerForPoint(hit.P);

    do
    {
        lightSampler->generateSample(tracer.getNoiseGenerator(), shadowRay);
        --shadowRay;

        float nDotL = glm::dot(hit.N, shadowRay.dir());
        if (nDotL <= 0.f)
        {
            continue;
        }

        if (tracer.traceShadow(shadowRay))
        {
            continue;
        }

        result += computeSurfaceLighting(
                shadowRay, hit, mBrdf, lightColor, nDotL, hasSpecLobe);
    } while (shadowRay.currentSample());

    delete lightSampler;
    return result / (float)light.shadowRays();
}

