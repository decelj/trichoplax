#include "direct_light.h"
#include "multi_sample_ray.h"

DirectLight::DirectLight(glm::vec3 dir, glm::vec3 kd, float bias) 
{ 
    mKd = kd;
    mDir = glm::normalize(dir);
    mBias = bias;
    mShadowRays = 1;
}

glm::vec3 DirectLight::getDir(const glm::vec3& p) const
{
    return mDir;
}

glm::vec3 DirectLight::getHalf(const glm::vec3& dirToLgt, const glm::vec3& I) const
{
    return glm::normalize(dirToLgt + I);
}

void DirectLight::attenuate(const glm::vec3& P, glm::vec3& result) const
{
    return;
}

bool DirectLight::generateShadowRay(MultiSampleRay& r, float& distTolgt) const
{
    if (r.mSample <= 0) return false;
    
    r.setDir(this->getDir(*r.origin()));
    r.bias(mBias);
    
    distTolgt = MAXFLOAT;
    
    r.mSample--;
    
    return true;
}
