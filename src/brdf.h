#ifndef __BRDF_H__
#define __BRDF_H__

#include <glm/glm.hpp>

struct BRDF {
    glm::vec3 Kd, Ks, Ka, Ke, Kt;
    float Kr, ior;
};

#endif

