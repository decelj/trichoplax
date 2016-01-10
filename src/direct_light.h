#ifndef __DIRECT_LIGHT_H__
#define __DIRECT_LIGHT_H__

#include <glm/glm.hpp>

#include "multi_sample_ray.h"
#include "ilight.h"

class DirectLight : public ILight {
public:
    DirectLight(const glm::vec3& dir, const glm::vec3& kd, float bias);
    
    // ILight
    glm::vec3 directionToLight(const glm::vec3& p) const;
    void attenuate(const glm::vec3& P, glm::vec3& result) const;
    bool generateShadowRay(Noise& noise, MultiSampleRay& outRay) const;
    
private:
    glm::vec3 mDir;
};


inline glm::vec3 DirectLight::directionToLight(const glm::vec3& p) const
{
    return mDir;
}

inline void DirectLight::attenuate(const glm::vec3& P, glm::vec3& result) const
{
    return;
}

inline bool DirectLight::generateShadowRay(Noise& /*noise*/, MultiSampleRay& outRay) const
{
    if (outRay.currentSample() == 0) return false;

    outRay.zeroSampleCount(); // No soft shadows for direct lights
    outRay.setDir(directionToLight(outRay.origin()));
    outRay.bias(mBias);

    return true;
}

#endif

