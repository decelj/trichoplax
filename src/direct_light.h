#ifndef __DIRECT_LIGHT_H__
#define __DIRECT_LIGHT_H__

#include <glm/glm.hpp>

#include "multi_sample_ray.h"
#include "ilight.h"
#include "point_sampler.h"


class DirectLight : public ILight
{
public:
    DirectLight(const glm::vec3& dir, const glm::vec3& kd, float bias);
    
    // ILight
    glm::vec3 directionToLight(const glm::vec3& p) const override;
    void attenuate(const glm::vec3& P, glm::vec3& result) const override;
    ISampler* generateSamplerForPoint(const glm::vec3& samplePoint) const override;
    void setShadowRays(unsigned numRays) override;
    unsigned shadowRays() const override;
    
private:
    glm::vec3   mDir;
    unsigned    mShadowRays;
};


inline glm::vec3 DirectLight::directionToLight(const glm::vec3& /*p*/) const
{
    return mDir;
}

inline void DirectLight::attenuate(const glm::vec3& P, glm::vec3& result) const
{
    return;
}

inline ISampler* DirectLight::generateSamplerForPoint(const glm::vec3& samplePoint) const
{
    return new PointSampler(directionToLight(samplePoint));
}

inline void DirectLight::setShadowRays(unsigned numRays)
{
    mShadowRays = numRays;
}

inline unsigned DirectLight::shadowRays() const
{
    return mShadowRays;
}

#endif

