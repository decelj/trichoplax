#ifndef __HIT_H__
#define __HIT_H__

#include <glm/glm.hpp>

class IPrimitive;

struct Hit
{
    const IPrimitive *prim;
    glm::vec3 p;
    glm::vec3 I;
    glm::vec3 n;
    short int depth;
};

#endif

