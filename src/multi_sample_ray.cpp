#include "multi_sample_ray.h"
#include "ray.h"


MultiSampleRay::MultiSampleRay(const Ray::TYPE t, unsigned samples)
    : Ray(t)
    , mCurrentSample(samples)
    , mSamples(samples)
{
}
