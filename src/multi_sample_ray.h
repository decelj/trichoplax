#ifndef __MULTI_SAMPLE_RAY_H__
#define __MULTI_SAMPLE_RAY_H__

#include <glm/glm.hpp>
#include "ray.h"

class MultiSampleRay : public Ray {
public:
    explicit MultiSampleRay(unsigned int samples);
    explicit MultiSampleRay(const glm::vec3& orign, const glm::vec3& dir, const int depth, unsigned int samples);
    explicit MultiSampleRay(const glm::vec3& origin, unsigned int samples);
    
    unsigned int mSample;
private:
    MultiSampleRay(const MultiSampleRay&) {}
};

#endif
