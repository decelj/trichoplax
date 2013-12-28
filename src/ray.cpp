#include <iostream>

#include "ray.h"
#include "hit.h"
#include "iprimitive.h"
#include "material.h"
#include "raytracer.h"

Ray::Ray(const TYPE t)
: mType(t),
  mDepth(1),
  mIor(1.0f),
  mHitT(MAXFLOAT),
  mHitPrim(NULL)
{ }

Ray::Ray(const TYPE t, const glm::vec3& origin, const float ior)
: mType(t),
  mOrigin(origin),
  mDepth(1),
  mIor(ior),
  mHitT(MAXFLOAT),
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
  mHitT(r.mHitT),
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
mHitT(r.mHitT),
mHitBack(r.mHitBack),
mHitPrim(r.mHitPrim)
{ }

void Ray::transformed(const glm::mat4& m, Ray& outRay) const
{    
    // Apply translation to origin
    outRay.mOrigin = glm::vec3(m * glm::vec4(mOrigin, 1.0));
    
    // Apply upper 3x3 rotation to direction
    outRay.mDir = glm::normalize(glm::vec3(m * glm::vec4(mDir, 0.0)));
    
    // Scale hitT
    if (mHitT < MAXFLOAT)
        outRay.mHitT = mHitT * 
                       glm::length(glm::vec3(m[0][0], m[1][1], m[2][2]));
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

bool Ray::refracted(const Hit& h, Ray& r) const
{
    float cosThetaIn = glm::dot(mDir, h.N);
    
    float iorQuotient;
    if (cosThetaIn > 0.f) {
        iorQuotient = r.mIor / mIor;
        r.shouldHitBackFaces(true);
    } else {
        // Assume we're going back to air if we've hit
        // a back face.
        iorQuotient = mIor;
    }
    float sinSqrdThetaOut = powf(iorQuotient, 2.f) *
                            (1.f - powf(cosThetaIn, 2.f));
    
    // Total internal reflection
    // TODO: Handle this
    if (sinSqrdThetaOut > 1.f)
        return false;
    
    float cosThetaOut = sqrtf(1.f - sinSqrdThetaOut);
    r.setDir(iorQuotient * mDir +
             (iorQuotient * cosThetaIn - cosThetaOut) * h.N);
    
    r.setDir(glm::normalize(r.dir()));
    
    r.mDepth = mDepth+1;
    return true;
}

void Ray::shade(const Raytracer* tracer, glm::vec4& result) const
{
    assert(mHitPrim != NULL);
    mHitPrim->material()->shadeRay(tracer, *this, result);
}

