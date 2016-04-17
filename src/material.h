#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <glm/glm.hpp>

#include "brdf.h"
#include "common.h"

class Ray;
class Raytracer;
class ILight;
class Hit;


class Material {
public:
    explicit Material();
    explicit Material(const glm::vec3& Ka, const glm::vec3& Ke,
                      const glm::vec3& Kd, const glm::vec3& Kt,
                      float Kr, float roughness, float ior);
    Material* clone() const;
    
    void setAmbient(const glm::vec3& Ka);
    void setEmissive(const glm::vec3& Ke);
    void setDiffuse(const glm::vec3& Kd);
    void setTransparency(const glm::vec3& Kt);
    void setReflectivity(float Kr);
    void setRoughness(float roughness);
    void setIor(float Ior);
    
    void shadeRay(const Raytracer& tracer, const Ray& r, glm::vec4& result) const;
    
private:
    Material(const Material& other);

    glm::vec3 sampleLight(const ILight& light, const Hit& hit,
                          const Raytracer& tracer, unsigned rayDepth,
                          bool isSpecular) const;
    
    BRDF mBrdf;
};


inline Material* Material::clone() const
{
    return new Material(*this);
}

inline void Material::setAmbient(const glm::vec3& Ka)
{
    mBrdf.setKa(Ka);
}

inline void Material::setEmissive(const glm::vec3& Ke)
{
    mBrdf.setKe(Ke);
}

inline void Material::setDiffuse(const glm::vec3& Kd)
{
    mBrdf.setKd(Kd);
}

inline void Material::setTransparency(const glm::vec3& Kt)
{
    mBrdf.setKt(Kt);
}

inline void Material::setRoughness(float roughness)
{
    mBrdf.setRoughness(roughness);
}

inline void Material::setReflectivity(float Kr)
{
    mBrdf.setKr(Kr);
}

inline void Material::setIor(float ior)
{
    mBrdf.setIOR(ior);
}

#endif

