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
    explicit Camera() { } // Don't use this

    float mFov;
    float mAlpha, mBeta;
    int mWidth, mHeight;
    glm::vec3 mPos, mLookAt, mUp;
    glm::vec3 mU, mV, mW;
};

#endif

