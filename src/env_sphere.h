#ifndef __ENV_SPHERE_H__
#define __ENV_SPHERE_H__

#include <string>
#include <glm/glm.hpp>

#include "FreeImage.h"

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
    
    FIBITMAP* mImg;
    unsigned int mWidth, mHeight;
};

#endif
