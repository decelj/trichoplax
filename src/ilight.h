#ifndef __ILIGHT_H__
#define __ILIGHT_H__

#include <glm/glm.hpp>
#include <algorithm>

class MultiSampleRay;
class Noise;

class ILight
{
public:
    virtual ~ILight() { }
    virtual glm::vec3 directionToLight(const glm::vec3& p) const = 0;
    virtual void attenuate(const glm::vec3& P, glm::vec3& result) const = 0;
    virtual bool generateShadowRay(Noise& noise, MultiSampleRay& outRay) const = 0;
    virtual void setShadowRays(unsigned numRays) { mShadowRays = numRays; }

    const glm::vec3& color() const { return mKd; }
    float bias() const { return mBias; }
    unsigned shadowRays() const { return mRadius > 0.f ? mShadowRays : 1; }
    unsigned firstPassShadowRays() const;
    unsigned secondPassShadowRays() const;
    
protected:
    ILight(const glm::vec3& kd, float radius, float bias, unsigned shadowRays);

    ILight(const ILight&) = delete;
    ILight& operator=(const ILight&) = delete;

    glm::vec3 mKd;
    float mRadius;
    float mBias;
    unsigned mShadowRays;
};


inline unsigned ILight::firstPassShadowRays() const
{
    return mShadowRays > 1 ? mShadowRays / 2 : mShadowRays;
}

inline unsigned ILight::secondPassShadowRays() const
{
    return std::max(mShadowRays - firstPassShadowRays(), 0u);
}

#endif
