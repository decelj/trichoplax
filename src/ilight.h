#ifndef __ILIGHT_H__
#define __ILIGHT_H__

#include <glm/glm.hpp>

class MultiSampleRay;
class Noise;

class ILight
{
public:
    virtual ~ILight() { }
    virtual glm::vec3 getDir(const glm::vec3& p) const = 0;
    virtual glm::vec3 getHalf(const glm::vec3& dirToLgt, const glm::vec3& I) const = 0;
    virtual glm::vec3 getColor() const = 0;
    virtual void attenuate(const glm::vec3& P, glm::vec3& result) const = 0;
    virtual bool generateShadowRay(MultiSampleRay& r, Noise& noise) const = 0;
    
    float bias() const { return mBias; }
    unsigned int shadowRays() const { return mRadius > 0.f ? mShadowRays : 1; }
    virtual void setShadowRays(unsigned int numRays) { mShadowRays = numRays; }
    
protected:
    ILight(const glm::vec3& kd, float radius, float bias, unsigned shadowRays);

    ILight(const ILight&) = delete;
    ILight& operator=(const ILight&) = delete;

    glm::vec3 mKd;
    float mRadius;
    float mBias;
    unsigned int mShadowRays;
};

#endif
