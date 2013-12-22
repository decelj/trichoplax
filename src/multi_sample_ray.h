#ifndef __MULTI_SAMPLE_RAY_H__
#define __MULTI_SAMPLE_RAY_H__

#include <glm/glm.hpp>
#include "ray.h"

class MultiSampleRay : public Ray {
public:
    explicit MultiSampleRay(const Ray::TYPE t, const Ray& r,
                            unsigned int samples);
    
    inline unsigned int currentSample() { return mSamples; }
    inline void decrementSampleCount() { mSamples--; }
    inline void zeroSampleCount() { mSamples = 0; }
    
private:
    MultiSampleRay(const MultiSampleRay&) { }
    
    unsigned int mSamples;
};

#endif
