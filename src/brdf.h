#ifndef __BRDF_H__
#define __BRDF_H__

#include <glm/glm.hpp>

struct BRDF {
    BRDF (const glm::vec3& _Kd, const glm::vec3& _Ks, const glm::vec3& _Ka,
          const glm::vec3& _Ke, const glm::vec3& _Kt, float _Kr, float _ior)
    : Kd(_Kd), Ks(_Ks), Ka(_Ka), Ke(_Ke), Kt(_Kt), Kr(_Kr), ior(_ior)
    { }
    
    glm::vec3 Kd, Ks, Ka, Ke, Kt;
    float Kr, ior;
};

#endif

