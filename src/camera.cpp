#include "camera.h"
#include "sample.h"
#include "ray.h"

Camera::Camera(const float fov, const glm::vec3& pos, const glm::vec3& lookAt, const glm::vec3& up, const int width, const int height) : 
    mPos(pos), mLookAt(lookAt), mUp(up), mWidth(width), mHeight(height), mFov((fov / 180.f) * M_PI)
{
    float fovX = 2.f * atanf(tanf(mFov/2.f)*(width / static_cast<float>(height)));
    mAlpha = tanf(fovX/2.f);
    mBeta = tanf(mFov/2.f);
    mW = glm::normalize(pos - lookAt);
    mU = glm::normalize(glm::cross(up, mW));
    mV = glm::normalize(glm::cross(mU, mW));
}

void Camera::generateRay(const Sample& s, Ray* ray) const
{
    float alpha = mAlpha * ((s.x - (mWidth / 2.0f)) / (mWidth / 2.0f));
    float beta = mBeta * (((mHeight / 2.0f) - s.y) / (mHeight / 2.0f));
    
    ray->setOrigin(mPos);
    ray->setDir(glm::normalize(mU*alpha + mV*beta - mW));
}

