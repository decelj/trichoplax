#include "multi_sample_ray.h"
#include "ray.h"

MultiSampleRay::MultiSampleRay(unsigned int samples)
: Ray(),
  mSample(samples)
{
}

MultiSampleRay::MultiSampleRay(const glm::vec3& origin, const glm::vec3& dir, const int depth, unsigned int samples)
: Ray(origin, dir, depth),
  mSample(samples)
{
}

MultiSampleRay::MultiSampleRay(const glm::vec3& origin, unsigned int samples)
: Ray(origin),
  mSample(samples)
{
}
