#include "direct_light.h"
#include "multi_sample_ray.h"

DirectLight::DirectLight(const glm::vec3& dir, const glm::vec3& kd, float bias)
    : ILight(kd, 0.f, bias)
    , mDir(glm::normalize(dir))
{
}
