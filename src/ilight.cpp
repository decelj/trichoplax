#include "ilight.h"

ILight::ILight(const glm::vec3& kd, float radius, float bias)
    : mKd(kd)
    , mRadius(radius)
    , mBias(bias)
{
}
