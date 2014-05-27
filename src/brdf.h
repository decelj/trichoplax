#ifndef __BRDF_H__
#define __BRDF_H__

#include <glm/glm.hpp>

struct BRDF {
    BRDF () { }
    BRDF (const glm::vec3& Kd, const glm::vec3& Ks, const glm::vec3& Ka,
          const glm::vec3& Ke, const glm::vec3& Kt, float Kr, float ior)
    : Kd(Kd), Ks(Ks), Ka(Ka), Ke(Ke), Kt(Kt), Kr(Kr), ior(ior)
    { }
    
    glm::vec3 Kd, Ks, Ka, Ke, Kt;
    float Kr, ior;
};

#endif

