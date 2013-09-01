#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <glm/glm.hpp>

#include "brdf.h"
#include "common.h"

class Raytracer;
struct Hit;

class Material {
public:
    Material();
    Material* clone() const;
    
    void setAmbient(glm::vec3 Ka);
    void setEmissive(glm::vec3 Ke);
    void setDiffuse(glm::vec3 Kd);
    void setSpecular(glm::vec3 Ks);
    void setShininess(float Kr);
    
    glm::vec3 shadePoint(const Hit& hit) const;
    
private:
    Material(const Material& other);
    
    BRDF mBrdf;
};

#endif

