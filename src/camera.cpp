#include <glm/glm.hpp>
#include <cmath>

#include "camera.h"
#include "sample.h"
#include "ray.h"

Camera::Camera(const float fov, const glm::vec3& pos, const glm::vec3& lookAt, const glm::vec3& up, const unsigned width, const unsigned height)
    : mPos(pos)
    , mLookAt(lookAt)
    , mUp(up)
    , mWidth(0)
    , mHeight(0)
    , mFov(DEGREES_TO_RADIANS(fov))
{
    mW = glm::normalize(pos - lookAt);
    mU = glm::normalize(glm::cross(up, mW));
    mV = glm::normalize(glm::cross(mU, mW));

    setWidthHeight(width, height);
}

void Camera::setWidthHeight(unsigned width, unsigned height)
{
    mWidth = width;	
    mHeight = height;

    float fovX = 2.f * std::atanf(std::tanf(mFov/2.f)*(width / static_cast<float>(height)));
    mAlpha = std::tanf(fovX/2.f);
    mBeta = std::tanf(mFov/2.f);
}

void Camera::generateRay(const Sample& s, Ray* ray) const
{
    float alpha = mAlpha * ((s.x - (mWidth / 2.0f)) / (mWidth / 2.0f));
    float beta = mBeta * (((mHeight / 2.0f) - s.y) / (mHeight / 2.0f));
    
    ray->setOrigin(mPos);
    ray->setDir(glm::normalize(mU*alpha + mV*beta - mW));
}

