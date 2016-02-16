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
    return glm::dot(color, glm::vec3(0.2126f, 0.7152f, 0.0722f));
}

inline float distanceSquared(const glm::vec3& v)
{
    return glm::dot(v, v);
}

inline glm::mat3 generateBasis(const glm::vec3& w)
{
    TP_ASSERT(relEq(glm::length(w), 1.f));

    glm::vec3 u(0.f);
    if (fabsf(w.x) > fabsf(w.y))
    {
        float inverseLength = 1.f / sqrtf(w.x * w.x + w.z * w.z);
        u.x = -w.z * inverseLength;
        u.z = w.x * inverseLength;
    }
    else
    {
        float inverseLength = 1.f / sqrtf(w.y * w.y + w.z * w.z);
        u.y = -w.z * inverseLength;
        u.z = w.y * inverseLength;
    }

    glm::vec3 v = glm::cross(w, u);
    return glm::mat3(u, v, w);
}

template <typename T>
inline T lerp(const T& x, const T& y, float u)
{
    return x * (1.f - u) + u * y;
}

#endif

