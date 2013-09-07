#ifndef __DIRECT_LIGHT_H__
#define __DIRECT_LIGHT_H__

#include <glm/glm.hpp>

#include "ilight.h"

class DirectLight : public ILight {
public:
    DirectLight(glm::vec3 dir, glm::vec3 kd, float bias);
    ~DirectLight() { };
    
    // ILight
    glm::vec3 getDir(const glm::vec3& p) const;
    glm::vec3 getHalf(const glm::vec3& dirToLgt, const glm::vec3& I) const;
    glm::vec3 getColor() const { return mKd; }
    void attenuate(const glm::vec3& P, glm::vec3& result) const;
    bool generateShadowRay(MultiSampleRay& r) const;
    
private:
    DirectLight() { }
    
    glm::vec3 mDir;
};

#endif

