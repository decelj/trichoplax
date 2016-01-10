#ifndef __MULTI_SAMPLE_RAY_H__
#define __MULTI_SAMPLE_RAY_H__

#include <glm/glm.hpp>
#include "ray.h"

class MultiSampleRay : public Ray {
public:
    explicit MultiSampleRay(const Ray::TYPE t, unsigned samples);

    unsigned numberOfSamples() const { return mSamples; }
    unsigned currentSample() const { return mCurrentSample; }
    void decrementSampleCount() { --mCurrentSample; }
    void zeroSampleCount() { mCurrentSample = 0; }
    void setNumberOfSamples(unsigned numSamples);
    
    MultiSampleRay& operator--() { --mCurrentSample; return *this; }
    
private:
    MultiSampleRay(const MultiSampleRay&) = delete;
    MultiSampleRay& operator=(const MultiSampleRay&) = delete;

    unsigned mCurrentSample;
    unsigned mSamples;
};


inline void MultiSampleRay::setNumberOfSamples(unsigned numSamples)
{
    mCurrentSample = numSamples;
    mSamples = numSamples;
}

#endif
