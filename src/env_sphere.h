#ifndef __ENV_SPHERE_H__
#define __ENV_SPHERE_H__

#include <string>
#include <algorithm>
#include <glm/glm.hpp>


class Ray;

class EnvSphere
{
public:
    explicit EnvSphere(const std::string& file);
    ~EnvSphere();
    
    void sample(const Ray& r, glm::vec4& result) const;
    
private:
    EnvSphere(const EnvSphere&) = delete;
    EnvSphere& operator=(const EnvSphere&) = delete;

    const glm::vec3& getPixelClamped(unsigned x, unsigned y) const;
    
    glm::vec3*      mImg;
    unsigned        mWidth;
    unsigned        mHeight;
};


inline const glm::vec3& EnvSphere::getPixelClamped(unsigned x, unsigned y) const
{
    return mImg[std::min(y, mHeight) * mWidth + std::min(x, mWidth)];
}

#endif
