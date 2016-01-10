#include <cmath>

#include "point_light.h"
#include "multi_sample_ray.h"
#include "noise.h"
#include "common.h"


namespace
{
static const float gGoldenAngle = static_cast<float>(M_PI) * (3.0f - sqrtf(5.0f));
}

PointLight::PointLight(const glm::vec3& pos, const glm::vec3& kd, float radius,
                       float bias, float constAtten, float linearAtten, float quadAtten)
    : ILight(kd, radius, bias, 1)
    , mPos(pos)
    , mConstAtten(constAtten)
    , mLinearAtten(linearAtten)
    , mQuadAtten(quadAtten)
{
}

void PointLight::attenuate(const glm::vec3& P, glm::vec3& result) const
{
    if (!(mLinearAtten > 0.0 || mQuadAtten > 0.0 || mConstAtten != 1.f)) return;
    
    float distance = glm::length(P - mPos);
    result /= mConstAtten + mLinearAtten * distance + mQuadAtten * distance * distance;
}

bool PointLight::generateShadowRay(Noise& noise, MultiSampleRay& outRay) const
{
    if (outRay.currentSample() == 0) return false;

    glm::vec3 samplePoint;
    if (mRadius == 0.f)
    {
        TP_ASSERT(outRay.numberOfSamples() == 1);
        samplePoint = mPos;
    }
    else
    {
        randomPointOnDisk(noise, outRay.origin(), outRay.numberOfSamples(),
                          outRay.currentSample(), samplePoint);
    }
    
    glm::vec3 dirToLgtSample = samplePoint - outRay.origin();
    outRay.setDir(glm::normalize(dirToLgtSample));
    outRay.setMaxDistance(glm::length(dirToLgtSample));
    outRay.bias(mBias);
    --outRay;

    return  true;
}

void PointLight::randomPointOnDisk(Noise& noise, const glm::vec3& P,
                                   unsigned numSamples, unsigned currentSample,
                                   glm::vec3& result) const
{
    // Pick strata for current sample
    const float sqrtSamples = std::sqrtf((float)numSamples);
    const float numRStratas = std::max(1.f, std::floorf(sqrtSamples));
    const float numTheataStratas = std::ceilf(sqrtSamples);

    const float rStrata = std::floorf((float)currentSample / numRStratas);
    const float rStrataSize = mRadius / numRStratas;

    const float thetaStrata = (float)(currentSample % (unsigned)numTheataStratas);
    const float thetaStrataSize = (2.f * (float)M_PI) / numTheataStratas;

    // Generate a sample within that strata
    float r = noise.generateNormalizedFloat() * rStrataSize;
    r += rStrataSize * rStrata;

    float theta = noise.generateNormalizedFloat() * thetaStrataSize;
    theta += thetaStrataSize * thetaStrata;

    const float x = r * cosf(theta);
    const float y = r * sinf(theta);
    
    result.x = x + mPos.x;
    result.y = y + mPos.y;
    
    const glm::vec3 dirToLgt = glm::normalize(P - mPos);
    result.z = ((dirToLgt.x * x + dirToLgt.y * y) / (-1.f * dirToLgt.z)) + mPos.z;
}

