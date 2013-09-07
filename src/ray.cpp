#include "ray.h"
#include "hit.h"

#include "iprimitive.h"
#include "material.h"

Ray::Ray()
: mDepth(1),
  mIor(1.0f),
  mHitT(MAXFLOAT),
  mHitPrim(NULL)
{ }

Ray::Ray(const glm::vec3& origin, const float ior, const short depth)
: mOrigin(origin),
  mDepth(depth),
  mIor(ior),
  mHitT(MAXFLOAT),
  mHitPrim(NULL)
{ }

// Copy constructor
Ray::Ray(const Ray& r)
: mOrigin(r.mOrigin),
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
}

/* Reflected ray = Iparallel - Iorthogonal
   Iparallel = I - IProjN = I - I . N * N
   Iorthogonal = IProjN = I . N * N
 */
void Ray::reflected(const Hit& hit, Ray& r) const
{
    r.mDepth = mDepth+1;
    r.mOrigin = hit.P;
    r.mDir = hit.I - 2.f * hit.N * glm::dot(hit.I, hit.N);
    r.mInverseDir = 1.f / mDir;
}

void Ray::refracted(const Hit& hit, Ray& r) const
{
    r.mDepth = mDepth + 1;
    r.mOrigin = hit.P;
    
    float ior_quotient = mIor / r.mIor;
    float cos_theta_in = glm::dot(hit.I, hit.N);
    float cos_theta_out = sqrtf(1.f - powf(ior_quotient, 2.f) * 
                                (1.f - powf(cos_theta_in, 2.f)));
    if (cos_theta_in > 0)
        r.mDir = ior_quotient * hit.I + 
                 (ior_quotient * cos_theta_in - cos_theta_out) * hit.N;
    else
        r.mDir = ior_quotient * hit.I +
                 (ior_quotient * cos_theta_in + cos_theta_out) * hit.N;

}

void Ray::shade(glm::vec4& result) const
{
    assert(mHitPrim != NULL);
    mHitPrim->material()->shadeRay(*this, result);
}

