#ifndef __COMMON_H__
#define __COMMON_H__

#include <math.h>
#include <glm/glm.hpp>

#define EPSILON .000001f
#define VEC3_IS_REL_ZERO(v) (v[0] < EPSILON && v[1] < EPSILON && v[2] < EPSILON)
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

inline bool relEq(const float a, const float b, const float tol=EPSILON)
{
    return fabs(a - b) < tol;
}

#endif

