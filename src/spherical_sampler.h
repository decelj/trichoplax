#ifndef sphereical_sampler_h
#define sphereical_sampler_h

#include <glm/glm.hpp>

#include "isampler.h"

class Noise;
class MultiSampleRay;
class FixedDomainSamplerInfo2D;

class SphericalSampler : public ISampler
{
public:
    SphericalSampler(const glm::vec3& position, float radius,
                     const FixedDomainSamplerInfo2D* info, const glm::vec3& samplePoint);

    void generateSample(Noise& noise, MultiSampleRay& outRay) const override;

private:
    glm::mat3                           mSampleToWorld;
    glm::vec3                           mPosition;
    float                               mRadius;
    float                               mDistanceToSample;
    float                               mDistanceSquaredToSample;
    float                               mCosThetaMax;
    const FixedDomainSamplerInfo2D*     mSamplerInfo;
};

#endif /* sphereical_sampler_h */
