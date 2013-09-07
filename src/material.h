#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <glm/glm.hpp>

#include "brdf.h"
#include "common.h"

class Ray;

class Material {
public:
    Material();
    Material* clone() const;
    
    void setAmbient(glm::vec3 Ka);
    void setEmissive(glm::vec3 Ke);
    void setDiffuse(glm::vec3 Kd);
    void setSpecular(glm::vec3 Ks);
    void setShininess(float Kr);
    void setIor(float Ior);
    
    void shadeRay(const Ray& r, glm::vec4& result) const;
    
private:
    Material(const Material& other);
    
    BRDF mBrdf;
};

#endif

