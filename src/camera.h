#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <glm/glm.hpp>
#include <math.h>

struct Sample;
class Ray;

class Camera
{
public:
    explicit Camera(const float fov, const glm::vec3& pos, const glm::vec3& lookAt,
                    const glm::vec3& up, const int width, const int height);
    void generateRay(const Sample& s, Ray* ray) const;

private:
    explicit Camera() :
        mPos(glm::vec3(0.f)), mLookAt(glm::vec3(0.f)), mUp(glm::vec3(0.f)),
        mWidth(0), mHeight(0), mFov(0.f)
    { } // Don't use this

    const glm::vec3 mPos, mLookAt, mUp;
    const unsigned short mWidth, mHeight;
    const float mFov;
    float mAlpha, mBeta;
    glm::vec3 mU, mV, mW;
};

#endif

