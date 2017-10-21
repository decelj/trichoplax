#ifndef __ILIGHT_H__
#define __ILIGHT_H__

#include <glm/glm.hpp>
#include <algorithm>

class MultiSampleRay;
class Noise;
class ISampler;

class ILight
{
public:
    virtual ~ILight() { }
    virtual glm::vec3 directionToLight(const glm::vec3& p) const = 0;
    virtual void attenuate(const glm::vec3& P, glm::vec3& result) const = 0;
    virtual ISampler* generateSamplerForPoint(const glm::vec3& samplePoint) const = 0;
    virtual void setShadowRays(unsigned numRays) = 0;
    virtual unsigned shadowRays() const = 0;

    const glm::vec3& color() const { return mKd; }
    float bias() const { return mBias; }
    unsigned firstPassShadowRays() const;
    unsigned secondPassShadowRays() const;

    void setBias(float bias) { mBias = bias; }
    
protected:
    ILight(const glm::vec3& kd, float radius, float bias);

    ILight(const ILight&) = delete;
    ILight& operator=(const ILight&) = delete;

    glm::vec3 mKd;
    float mRadius;
    float mBias;
};


inline unsigned ILight::firstPassShadowRays() const
{
    return shadowRays() > 1 ? shadowRays() / 2 : shadowRays();
}

inline unsigned ILight::secondPassShadowRays() const
{
    return std::max(shadowRays() - firstPassShadowRays(), 0u);
}

#endif
