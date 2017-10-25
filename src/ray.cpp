#include <limits>
#include <glm/glm.hpp>

#include "ray.h"

Ray::Ray(const TYPE t)
    : mType(t)
    , mOrigin(0.f)
    , mDir(0.f)
    , mDepth(1)
    , mIor(1.0f)
    , mMinT(0.f)
    , mMaxT(std::numeric_limits<float>::max())
    , mHitPrim(nullptr)
    , mDidHitBack(false)
    , mShouldHitBack(false)
{
}

Ray::Ray(const TYPE t, const glm::vec3& origin, const float ior)
    : mType(t)
    , mOrigin(origin)
    , mDir(0.f)
    , mDepth(1)
    , mIor(ior)
    , mMinT(0.f)
    , mMaxT(std::numeric_limits<float>::max())
    , mHitPrim(nullptr)
    , mDidHitBack(false)
    , mShouldHitBack(false)
{
}

// Copy constructor
Ray::Ray(const Ray& r)
    : mType(r.mType)
    , mOrigin(r.mOrigin)
    , mDir(r.mDir)
    , mDepth(r.mDepth)
    , mIor(r.mIor)
    , mMinT(r.mMinT)
    , mMaxT(r.mMaxT)
    , mHitPrim(r.mHitPrim)
    , mDidHitBack(r.mDidHitBack)
    , mShouldHitBack(r.mShouldHitBack)
{
}

// Copy constructor w/ type change
Ray::Ray(const Ray& r, TYPE t)
    : mType(t)
    , mOrigin(r.mOrigin)
    , mDir(r.mDir)
    , mDepth(r.mDepth)
    , mIor(r.mIor)
    , mMinT(r.mMinT)
    , mMaxT(r.mMaxT)
    , mHitPrim(r.mHitPrim)
    , mDidHitBack(r.mDidHitBack)
    , mShouldHitBack(r.mShouldHitBack)
{
}

bool Ray::refract(const Hit& hit, float inIOR)
{
    glm::vec3 N = hit.N;
    float nDotI = glm::dot(-hit.V, N);

    float eta;
    if (hit.hitBackFace)
    {
        // Assume we're going back to air if we've hit a back face.
        eta = mIor;
    }
    else
    {
        eta = inIOR / mIor;
        shouldHitBackFaces(true);
    }

    float k = 1.f - eta * eta * (1.f - nDotI * nDotI);
    if (k < 0.f)
    {
        // Total internal reflection
        return false;
    }

    setDir(glm::normalize(eta * -hit.V - (eta * nDotI + sqrtf(k)) * N));

    return true;
}
