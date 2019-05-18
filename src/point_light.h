#ifndef __POINT_LIGHT_H__
#define __POINT_LIGHT_H__

#include <glm/glm.hpp>

#include "ilight.h"
#include "sampler_info.h"
#include "spherical_sampler.h"
#include "point_sampler.h"


class PointLight : public ILight
{
public:
    PointLight(const glm::vec3& pos, const glm::vec3& kd, float radius, float bias,
               float constAtten, float linearAtten, float quadAtten);

    // ILight
    glm::vec3 directionToLight(const glm::vec3& p) const override;
    void attenuate(const glm::vec3& P, glm::vec3& result) const override;
    ISampler* generateSamplerForPoint(const glm::vec3& samplePoint) const override;
    void setShadowRays(unsigned numRays) override;
    unsigned shadowRays() const override;
    
private:
    glm::vec3                   mPos;
    float                       mConstAtten;
    float                       mLinearAtten;
    float                       mQuadAtten;
    FixedDomainSamplerInfo2D    mSamplesInfo;
};


inline glm::vec3 PointLight::directionToLight(const glm::vec3& p) const
{
    return glm::normalize(mPos - p);
}

inline ISampler* PointLight::generateSamplerForPoint(const glm::vec3& samplePoint) const
{
    if (shadowRays() > 1)
    {
        return new SphericalSampler(mPos, mRadius, &mSamplesInfo, samplePoint);
    }
    else
    {
        glm::vec3 dir = mPos - samplePoint;
        float distance = glm::length(dir);
        dir = dir / distance;
        return new PointSampler(dir, distance);
    }
}

inline void PointLight::setShadowRays(unsigned numRays)
{
    mSamplesInfo.setNumberOfSamples(numRays);
}

inline unsigned PointLight::shadowRays() const
{
    return mSamplesInfo.numberOfSamples();
}

#endif
