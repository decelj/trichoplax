#ifndef __DIRECT_LIGHT_H__
#define __DIRECT_LIGHT_H__

#include <glm/glm.hpp>

#include "ilight.h"

class DirectLight : public ILight {
public:
    DirectLight(const glm::vec3& dir, const glm::vec3& kd, float bias);
    ~DirectLight() { };
    
    // ILight
    glm::vec3 getDir(const glm::vec3& p) const;
    glm::vec3 getHalf(const glm::vec3& dirToLgt, const glm::vec3& I) const;
    glm::vec3 getColor() const { return mKd; }
    void attenuate(const glm::vec3& P, glm::vec3& result) const;
    bool generateShadowRay(MultiSampleRay& r, Noise& noise) const;
    
private:
    glm::vec3 mDir;
};


inline glm::vec3 DirectLight::getDir(const glm::vec3& p) const
{
    return mDir;
}

inline glm::vec3 DirectLight::getHalf(const glm::vec3& dirToLgt, const glm::vec3& I) const
{
    return glm::normalize(dirToLgt + I);
}

inline void DirectLight::attenuate(const glm::vec3& P, glm::vec3& result) const
{
    return;
}

#endif

