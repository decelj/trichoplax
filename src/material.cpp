#include <string.h> // For memcpy
#include <math.h>

#include "material.h"
#include "scene.h"
#include "ray.h"
#include "multi_sample_ray.h"
#include "ilight.h"
#include "common.h"
#include "hit.h"

Material::Material()
{
    mBrdf.Kd = mBrdf.Ks = mBrdf.Ke = glm::vec3(0,0,0);
    mBrdf.Ka = glm::vec3(.2,.2,.2);
    mBrdf.Kr = 0;
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

void Material::setShininess(float Kr)
{
    mBrdf.Kr = Kr;
}

glm::vec3 Material::shadePoint(const Hit& hit) const
{
    // Ambient and emissive
    glm::vec3 result = mBrdf.Ka + mBrdf.Ke;
    
    const Scene* s = Scene::instance();
    const bool hasDiffuse = !VEC3_IS_REL_ZERO(mBrdf.Kd);
    const bool hasSpecular = !VEC3_IS_REL_ZERO(mBrdf.Ks);
    
    for (std::vector<ILight*>::const_iterator it = s->lightsBegin(); 
         it != s->lightsEnd() && (hasDiffuse || hasSpecular); ++it) {
        const ILight* lgt = *it;
        glm::vec3 lightColor = lgt->getColor();
        lgt->attenuate(hit.p, lightColor);
        
        // Check for zero contribution
        if (VEC3_IS_REL_ZERO(lightColor))
            continue;
        
        float distToLgt = 0.0f;
        MultiSampleRay shadow(hit.p, lgt->shadowRays());
        while (lgt->generateShadowRay(shadow, distToLgt)) {
            if (!s->traceShadow(shadow, distToLgt)) {
                // Diffuse
                if (hasDiffuse)
                    result += mBrdf.Kd * MAX(glm::dot(hit.n, *shadow.dir()), 0.0f) * lightColor;
                // Specular
                if (hasSpecular)
                    result += mBrdf.Ks * powf(MAX(glm::dot(lgt->getHalf(*shadow.dir(), hit.I), hit.n), 0.0f), mBrdf.Kr) * lightColor;
            }
        }
        
        result /= lgt->shadowRays();
    }
    
    // Reflection
    if (hasSpecular) {
        glm::vec3 IProjN = hit.n * glm::dot(hit.I, hit.n);
        glm::vec3 IOrthogonalN = hit.I - IProjN;
        IOrthogonalN *= -1.0;
        Ray reflected(hit.p, IOrthogonalN + IProjN, hit.depth+1);
        reflected.bias(.01);
        
        glm::vec3 reflection(0,0,0);
        s->traceAndShade(reflected, &reflection);
        result += reflection * mBrdf.Ks;
    }
    
    return result;
}
    
