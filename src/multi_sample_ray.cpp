#include "multi_sample_ray.h"
#include "ray.h"


MultiSampleRay::MultiSampleRay(const Ray::TYPE t, const Ray& r,
                               unsigned int samples)
: Ray(r, t),
  mSamples(samples)
{
}
