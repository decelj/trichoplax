#include <cmath>
#include <random>

#include "point_light.h"
#include "multi_sample_ray.h"

namespace {
    static const float gGoldenAngle = M_PI * (3.0f - sqrtf(5.0f));
}

PointLight::PointLight(glm::vec3 pos, glm::vec3 kd, float radius, float bias,
                       float constAtten, float linearAtten, float quadAtten) : 
    mPos(pos),
    mConstAtten(constAtten),
    mLinearAtten(linearAtten),
    mQuadAtten(quadAtten),
    mSqrtShadowSamples(1.0f)
{
    mHasAtten = mLinearAtten > 0.0 || mQuadAtten > 0.0 || mConstAtten != 1.f;
    mKd = kd;
    mBias = bias;
    mRadius = radius;
    mShadowRays = 1;
    mRandEngine.seed(glm::dot(pos, pos));
}

glm::vec3 PointLight::getDir(const glm::vec3& p) const
{
    return glm::normalize(mPos - p);
}

glm::vec3 PointLight::getHalf(const glm::vec3& dirToLgt, const glm::vec3& I) const
{
    return glm::normalize(dirToLgt + I);
}

void PointLight::attenuate(const glm::vec3& P, glm::vec3& result) const
{
    if (!mHasAtten) return;
    
    float distance = glm::length(P - mPos);
    result /= mConstAtten + mLinearAtten * distance + mQuadAtten * distance * distance;
}

bool PointLight::generateShadowRay(MultiSampleRay& r) const
{
    if (r.currentSample() <= 0) return false;
    
    glm::vec3 samplePoint;
    //pointOnDisk(r.origin(), r.currentSample(), samplePoint);
    randomPointOnDisk(r.origin(), samplePoint);
    
    glm::vec3 dirToLgtSample = samplePoint - r.origin();
    r.setDir(glm::normalize(dirToLgtSample));
    r.setMinDistance(glm::length(dirToLgtSample));
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

void PointLight::randomPointOnDisk(const glm::vec3& P, glm::vec3& result) const
{
    const float r = std::generate_canonical<float, 16>(mRandEngine) * mRadius;
    const float angle = std::generate_canonical<float, 16>(mRandEngine) * 360.f;
    const float x = r * cosf(angle);
    const float y = r * cosf(angle);
    
    result.x = x + mPos.x;
    result.y = y + mPos.y;
    
    const glm::vec3 dirToLgt = glm::normalize(P - mPos);
    result.z = ((dirToLgt.x * x + dirToLgt.y * y) / (-1.f * dirToLgt.z)) + mPos.z;
}

