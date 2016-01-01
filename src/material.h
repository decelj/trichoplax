#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <glm/glm.hpp>

#include "brdf.h"
#include "common.h"

class Ray;
class Raytracer;

class Material {
public:
    explicit Material();
    explicit Material(const glm::vec3& Ka, const glm::vec3& Ke,
                      const glm::vec3& Kd, const glm::vec3& Ks,
                      const glm::vec3& Kt, const float Kr, const float ior);
    Material* clone() const;
    
    void setAmbient(glm::vec3 Ka);
    void setEmissive(glm::vec3 Ke);
    void setDiffuse(glm::vec3 Kd);
    void setSpecular(glm::vec3 Ks);
    void setTransparency(glm::vec3 Kt);
    void setShininess(float Kr);
    void setIor(float Ior);
    
    void shadeRay(const Raytracer* tracer, const Ray& r, glm::vec4& result) const;
    
private:
    Material(const Material& other);
    
    BRDF mBrdf;
};

#endif

