#include "point_sampler.h"

PointSampler::PointSampler(const glm::vec3& direction, float maxDistance)
    : ISampler()
    , mDirection(direction)
    , mMaxDistance(maxDistance)
{
}
