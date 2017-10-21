#include <glm/glm.hpp>
#include <cmath>

#include "point_light.h"


PointLight::PointLight(const glm::vec3& pos, const glm::vec3& kd, float radius,
                       float bias, float constAtten, float linearAtten, float quadAtten)
    : ILight(kd, radius, bias)
    , mPos(pos)
    , mConstAtten(constAtten)
    , mLinearAtten(linearAtten)
    , mQuadAtten(quadAtten)
    , mSamplesInfo(radius > 0.f ? 32 : 1, TWO_PI, 1.f)
{
}

void PointLight::attenuate(const glm::vec3& P, glm::vec3& result) const
{
    if (!(mLinearAtten > 0.0 || mQuadAtten > 0.0 || mConstAtten != 1.f)) return;
    
    float distance = glm::length(P - mPos);
    result /= mConstAtten + mLinearAtten * distance + mQuadAtten * distance * distance;
}
