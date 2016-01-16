#include "spherical_sampler.h"
#include "common.h"
#include "sampler_info.h"
#include "multi_sample_ray.h"
#include "noise.h"

SphericalSampler::SphericalSampler(const glm::vec3& position,
                                   float radius, const FixedDomainSamplerInfo2D* info,
                                   const glm::vec3& samplePoint)
    : mSampleToWorld(1.f, 0.f, 0.f,
                     0.f, 1.f, 0.f,
                     0.f, 0.f, 1.f)
    , mPosition(position)
    , mRadius(radius)
    , mDistanceSquaredToSample(0.f)
    , mDistanceToSample(0.f)
    , mCosThetaMax(0.f)
    , mSamplerInfo(info)
{
    glm::vec3 centerVector = mPosition - samplePoint;
    mDistanceSquaredToSample = distanceSquared(centerVector);
    mDistanceToSample = std::sqrtf(mDistanceSquaredToSample);

    if (mDistanceToSample > mRadius)
    {
        mSampleToWorld = generateBasis(centerVector / mDistanceToSample);
        mCosThetaMax = std::sqrtf(1.f - (mRadius * mRadius) / mDistanceSquaredToSample);
    }
    else
    {
        mCosThetaMax = -1.f;
    }
}

void SphericalSampler::generateSample(Noise& noise, MultiSampleRay& outRay) const
{
    float cosTheta = lerp(mCosThetaMax, 1.f,
        mSamplerInfo->computeYValue(outRay.currentSample(), noise.generateNormalizedFloat()));
    float sinTheta = std::sqrtf(1.f - cosTheta * cosTheta);
    float phi = mSamplerInfo->computeXValue(
        outRay.currentSample(), noise.generateNormalizedFloat());

    outRay.setDir(mSampleToWorld * glm::vec3(std::cosf(phi) * sinTheta,
                                             std::sinf(phi) * sinTheta,
                                             cosTheta));
    outRay.setMaxDistance(mDistanceToSample);
}
