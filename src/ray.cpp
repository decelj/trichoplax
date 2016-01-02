#include <iostream>
#include <limits>
#include <assert.h>

#include "ray.h"
#include "hit.h"
#include "iprimitive.h"
#include "material.h"
#include "raytracer.h"

Ray::Ray(const TYPE t)
: mType(t),
  mDepth(1),
  mIor(1.0f),
  mMinT(0.f),
  mMaxT(std::numeric_limits<float>::max()),
  mHitPrim(NULL)
{ }

Ray::Ray(const TYPE t, const glm::vec3& origin, const float ior)
: mType(t),
  mOrigin(origin),
  mDepth(1),
  mIor(ior),
  mMinT(0.f),
  mMaxT(std::numeric_limits<float>::max()),
  mHitPrim(NULL)
{ }

// Copy constructor
Ray::Ray(const Ray& r)
: mType(r.mType),
  mOrigin(r.mOrigin),
  mDir(r.mDir),
  mInverseDir(r.mInverseDir),
  mDepth(r.mDepth),
  mIor(r.mIor),
  mMinT(r.mMinT),
  mMaxT(r.mMaxT),
  mHitBack(r.mHitBack),
  mHitPrim(r.mHitPrim)
{ }

// Copy constructor w/ type change
Ray::Ray(const Ray& r, TYPE t)
: mType(t),
  mOrigin(r.mOrigin),
  mDir(r.mDir),
  mInverseDir(r.mInverseDir),
  mDepth(r.mDepth),
  mIor(r.mIor),
  mMinT(r.mMinT),
  mMaxT(r.mMaxT),
  mHitBack(r.mHitBack),
  mHitPrim(r.mHitPrim)
{ }

void Ray::transformed(const glm::mat4& m, Ray& outRay) const
{    
    // Apply translation to origin
    outRay.setOrigin(glm::vec3(m * glm::vec4(mOrigin, 1.0)));
    
    // Apply upper 3x3 rotation to direction
    outRay.setDir(glm::normalize(glm::mat3(m) * mDir));
}

/* Reflected ray = Iparallel - Iorthogonal
   Iparallel = I - IProjN = I - I . N * N
   Iorthogonal = IProjN = I . N * N
 */
void Ray::reflected(const Hit& hit, Ray& r) const
{
    r.mDepth = mDepth+1;
    r.mOrigin = hit.P;
    r.setDir(mDir - 2.f * glm::dot(mDir, hit.N) * hit.N);
}

bool Ray::refracted(const Hit& hit, Ray& outRay) const
{
    float nDotI = glm::dot(mDir, hit.N);
    
    float iorQuotient;
    if (nDotI <= 0.f)
    {
        iorQuotient = mIor / outRay.mIor;
        outRay.shouldHitBackFaces(true);
    }
    else
    {
        // Assume we're going back to air if we've hit a back face.
        iorQuotient = outRay.mIor;
        nDotI = glm::dot(mDir, -hit.N);
    }

    float k = 1.f - iorQuotient * iorQuotient * (1.f - nDotI * nDotI);

    // Total internal reflection
    if (k < 0.f)
        return false;
    
    outRay.setDir(iorQuotient * mDir - (iorQuotient * nDotI + sqrtf(k)) * hit.N);
    outRay.mDepth = mDepth + 1;

    return true;
}

void Ray::shade(const Raytracer* tracer, glm::vec4& result) const
{
    assert(mHitPrim != NULL);
    mHitPrim->material()->shadeRay(tracer, *this, result);
}

