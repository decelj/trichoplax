#ifndef __DIRECT_LIGHT_H__
#define __DIRECT_LIGHT_H__

#include <glm/glm.hpp>

#include "ilight.h"

class DirectLight : public ILight {
public:
    DirectLight(const glm::vec3& dir, const glm::vec3& kd, float bias);
    ~DirectLight() { };
    
    // ILight
    glm::vec3 directionToLight(const glm::vec3& p) const;
    void attenuate(const glm::vec3& P, glm::vec3& result) const;
    bool generateShadowRay(MultiSampleRay& r, Noise& noise) const;
    
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

#endif

