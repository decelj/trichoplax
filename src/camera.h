#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <glm/glm.hpp>

struct Sample;
class Ray;

class Camera
{
public:
    explicit Camera(const float fov, const glm::vec3& pos, const glm::vec3& lookAt,
                    const glm::vec3& up, const unsigned width, const unsigned height);

    void setWidthHeight(unsigned width, unsigned height);

    void generateRay(const Sample& s, Ray* ray) const;
    unsigned width() const { return mWidth; }
    unsigned height() const { return mHeight; }

private:
    Camera() = delete;
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    const glm::vec3 mPos, mLookAt, mUp;
    unsigned mWidth, mHeight;
    float mFov;
    float mAlpha, mBeta;
    glm::vec3 mU, mV, mW;
};

#endif

