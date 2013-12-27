#ifndef __ILIGHT_H__
#define __ILIGHT_H__

#include <glm/glm.hpp>

class MultiSampleRay;

class ILight
{
public:
    virtual ~ILight() { }
    virtual glm::vec3 getDir(const glm::vec3& p) const = 0;
    virtual glm::vec3 getHalf(const glm::vec3& dirToLgt, const glm::vec3& I) const = 0;
    virtual glm::vec3 getColor() const = 0;
    virtual void attenuate(const glm::vec3& P, glm::vec3& result) const = 0;
    virtual bool generateShadowRay(MultiSampleRay& r) const = 0;
    
    inline float bias() const { return mBias; }
    inline unsigned int shadowRays() const { return mRadius > 0.f ? mShadowRays : 1; }
    virtual inline void setShadowRays(unsigned int numRays) { mShadowRays = numRays; }
    
protected:
    glm::vec3 mKd;
    float mBias;
    float mRadius;
    unsigned int mShadowRays;
};
    
#endif