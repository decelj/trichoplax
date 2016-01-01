#include <cmath>

#include "point_light.h"
#include "multi_sample_ray.h"
#include "noise.h"

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
    , mSqrtShadowSamples(1.0f)
{
}

void PointLight::attenuate(const glm::vec3& P, glm::vec3& result) const
{
    if (!(mLinearAtten > 0.0 || mQuadAtten > 0.0 || mConstAtten != 1.f)) return;
    
    float distance = glm::length(P - mPos);
    result /= mConstAtten + mLinearAtten * distance + mQuadAtten * distance * distance;
}

bool PointLight::generateShadowRay(MultiSampleRay& r, Noise& noise) const
{
    if (r.currentSample() <= 0) return false;
    
    glm::vec3 samplePoint;
    //pointOnDisk(r.origin(), r.currentSample(), samplePoint);
    randomPointOnDisk(noise, r.origin(), samplePoint);
    
    glm::vec3 dirToLgtSample = samplePoint - r.origin();
    r.setDir(glm::normalize(dirToLgtSample));
    r.setMaxDistance(glm::length(dirToLgtSample));
    r.bias(mBias);
    
    --r;
    
    return true;
}

void PointLight::pointOnDisk(const glm::vec3& P, const unsigned int currentSample, glm::vec3& result) const
{
    // Spread points on disk
    // http://blog.marmakoide.org/?p=1
    const float r = sqrtf(static_cast<float>(currentSample)) * mRadius / mSqrtShadowSamples;
    const float theta = currentSample * gGoldenAngle;
    
    const float x = r * cosf(theta);
    const float y = r * sinf(theta);
    
    result.x = x + mPos.x;
    result.y = y + mPos.y;
    
    // Plane eqn, solve for z
    // Nx(X-X0) + Ny(Y-Y0) + Nz(Z-Z0) = 0
    // (Nx(X-X0) + Ny(Y-Y0)) / -Nz + Z0 = Z
    const glm::vec3 dirToLgt = glm::normalize(P - mPos);
    result.z = ((dirToLgt.x * x + dirToLgt.y * y) / (-1.0f * dirToLgt.z)) + mPos.z;
}

void PointLight::randomPointOnDisk(Noise& noise, const glm::vec3& P, glm::vec3& result) const
{
    const float r = noise.generateNormalizedFloat() * mRadius;
    const float angle = noise.generateNormalizedFloat() * 2.f * static_cast<float>(M_PI);
    const float x = r * cosf(angle);
    const float y = r * sinf(angle);
    
    result.x = x + mPos.x;
    result.y = y + mPos.y;
    
    const glm::vec3 dirToLgt = glm::normalize(P - mPos);
    result.z = ((dirToLgt.x * x + dirToLgt.y * y) / (-1.f * dirToLgt.z)) + mPos.z;
}

