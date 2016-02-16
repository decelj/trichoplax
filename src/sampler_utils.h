#ifndef sampler_utils_h
#define sampler_utils_h

#include "common.h"

#include <cmath>
#include <glm/glm.hpp>

inline void mapSquareToDisk(glm::vec2& sample)
{
    float phi;
    float r;

    float a = sample.x * 2.f - 1.f;
    float b = sample.y * 2.f - 1.f;

    if (a * a > b * b)
    {
        r = a;
        phi = (float)M_PI_4 * (b / a);
    }
    else
    {
        r = b;
        phi = (float)M_PI_2 - (float)M_PI_4 * (a / b);
    }

    sample.x = r * cosf(phi);
    sample.y = r * sinf(phi);
}

inline glm::vec3 cosineSampleHemisphere(const glm::vec2& sample)
{
    TP_ASSERT(sample.x <= 1.f && sample.x >= 0.f);
    TP_ASSERT(sample.y <= 1.f && sample.y >= 0.f);

    glm::vec2 mappedSample = sample;
    mapSquareToDisk(mappedSample);

    float z = sqrtf(glm::clamp(1.f - glm::dot(mappedSample, mappedSample), 0.f, 1.f));
    return glm::vec3(mappedSample.x, mappedSample.y, z);
}

#endif /* sampler_utils_h */
