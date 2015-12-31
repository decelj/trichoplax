#ifndef __POINT_LIGHT_H__
#define __POINT_LIGHT_H__

#include <glm/glm.hpp>
#include <random>

#include "ilight.h"

class PointLight : public ILight
{
public:
    PointLight(const glm::vec3& pos, const glm::vec3& kd, float radius, float bias,
               float constAtten, float linearAtten, float quadAtten);
    ~PointLight() { }
    
    // ILight
    glm::vec3 getDir(const glm::vec3& p) const;
    glm::vec3 getHalf(const glm::vec3& dirToLgt, const glm::vec3& I) const;
    glm::vec3 getColor() const { return mKd; }
    void attenuate(const glm::vec3& P, glm::vec3& result) const;
    bool generateShadowRay(MultiSampleRay& r, Noise& noise) const;
    void setShadowRays(unsigned int numRays); // reimplemented
    
private:
    void pointOnDisk(const glm::vec3& P,
                     const unsigned int currentSample,
                     glm::vec3& result) const;
    void randomPointOnDisk(Noise& noise, const glm::vec3& P, glm::vec3& result) const;
    
    glm::vec3 mPos;
    float mConstAtten, mLinearAtten, mQuadAtten;
    float mSqrtShadowSamples;
};


inline glm::vec3 PointLight::getDir(const glm::vec3& p) const
{
    return glm::normalize(mPos - p);
}

inline glm::vec3 PointLight::getHalf(const glm::vec3& dirToLgt, const glm::vec3& I) const
{
    return glm::normalize(dirToLgt + I);
}

inline void PointLight::setShadowRays(unsigned int numRays) {
    mShadowRays = numRays;
    mSqrtShadowSamples = sqrtf(numRays);
}

#endif

