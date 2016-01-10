#ifndef __COMMON_H__
#define __COMMON_H__

#include <cmath>
#include <cassert>
#include <glm/glm.hpp>

#define EPSILON .000001f
#define VEC3_IS_REL_ZERO(v) (v[0] < EPSILON && v[1] < EPSILON && v[2] < EPSILON)
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#if DEBUG
#define TP_ASSERT(_exp) assert(_exp)
#else
#define TP_ASSERT(_exp)
#endif

#define TP_UNUSED(_val) ((void)(_val))

inline bool relEq(const float a, const float b, const float tol=EPSILON)
{
    return fabsf(a - b) < tol;
}

inline float gammaToLinear(float x)
{
    return powf(x, 1.f / 2.2f);
}

inline glm::vec3 gammaToLinear(const glm::vec3& v)
{
    return glm::pow(v, glm::vec3(1.f / 2.2f));
}

inline float linearToGamma(float x)
{
    return powf(x, 2.2f);
}

inline glm::vec3 linearToGamma(const glm::vec3& v)
{
    return glm::pow(v, glm::vec3(2.2f));
}

inline float luminance(const glm::vec3& color)
{
    return glm::dot(color, glm::vec3(0.2126f, 0.7152f, 0.0722f)) / 3.f;
}

#endif

