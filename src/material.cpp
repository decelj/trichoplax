#include <string.h> // For memcpy
#include <math.h>

#include "material.h"
#include "scene.h"
#include "ray.h"
#include "multi_sample_ray.h"
#include "ilight.h"
#include "common.h"
#include "hit.h"
#include "raytracer.h"

Material::Material()
{
    mBrdf.Kd = mBrdf.Ks = mBrdf.Ke = glm::vec3(0.f, 0.f, 0.f);
    mBrdf.Ka = glm::vec3(.2f, .2f, .2f);
    mBrdf.Kt = glm::vec3(0.f, 0.f, 0.f);
    mBrdf.Kr = 0.0f;
    mBrdf.ior = 1.0f;
}

Material::Material(const Material& other)
{
    memcpy(&mBrdf, &other.mBrdf, sizeof(BRDF));
}

Material* Material::clone() const
{
    return new Material(*this);
}

void Material::setAmbient(glm::vec3 Ka)
{
    mBrdf.Ka = Ka;
}

void Material::setEmissive(glm::vec3 Ke)
{
    mBrdf.Ke = Ke;
}

void Material::setDiffuse(glm::vec3 Kd)
{
    mBrdf.Kd = Kd;
}

void Material::setSpecular(glm::vec3 Ks)
{
    mBrdf.Ks = Ks;
}

void Material::setTransparency(glm::vec3 Kt)
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
    if (transparencyFactor > 0.f) {
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
    if (hasSpecular && opaqueFactor > 0.f) {
        Ray reflected(Ray::REFLECTED);
        r.reflected(hit, reflected);
        reflected.bias(.01f);
        
        glm::vec4 reflection(0.f, 0.f, 0.f, 0.f);
        tracer->traceAndShade(reflected, reflection);
        result += reflection * glm::vec4(mBrdf.Ks * opaqueFactor,
                                         specularFactor * opaqueFactor * .5f);
    }
    
    if (hasDiffuse || hasSpecular) {
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
            while (lgt->generateShadowRay(shadow)) {
                if (!tracer->traceShadow(shadow)) {
                    float specularPower = 0.f;
                    if(hasSpecular) {
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
                    if (hasDiffuse) {
                        color += mBrdf.Kd * lightColor *
                                (1.f - std::max<float>((specularFactor / 2.f + specularPower + transparencyFactor),0.f)) *
                                 std::max<float>(glm::dot(hit.N, shadow.dir()), 0.f);
                    }
                }
            }
            
            color *= (1.f / lgt->shadowRays()) * opaqueFactor;
        }
    }
    
    result += glm::vec4(color, opaqueFactor);
    result.a = std::min<float>(result.a, 1.f);
}
    
