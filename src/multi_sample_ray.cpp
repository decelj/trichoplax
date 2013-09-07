#include "multi_sample_ray.h"
#include "ray.h"


MultiSampleRay::MultiSampleRay(const Ray& r, unsigned int samples)
: Ray(r),
  mSamples(samples)
{ }
