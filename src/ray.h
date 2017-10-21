#ifndef __RAY_H__
#define __RAY_H__

#include <glm/glm.hpp>
#include "hit.h"
#include "material.h"
#include "triangle.h"
#include "common.h"


class Raytracer;


class Ray
{
public:
    enum TYPE
    {
        PRIMARY = 0,
        REFLECTED,
        REFRACTED,
        SHADOW,
        GI,
        TYPE_COUNT
    };
    
    explicit Ray(const TYPE t);
    explicit Ray(const Ray& r);
    explicit Ray(const Ray& r, const TYPE t);
    explicit Ray(const TYPE t, const glm::vec3& origin, const float ior);
    
    void setDir(const glm::vec3& d)     { mDir = d; mInverseDir = 1.0f / d; }
    void setOrigin(const glm::vec3& o)  { mOrigin = o; }
    void setDepth(unsigned depth)       { mDepth = depth; }
    void bias(const float bias)         { mMinT = bias; }
    void setMaxDistance(float dist)     { mMaxT = dist; }
    void shouldHitBackFaces(bool value) { mShouldHitBack = value; }
    void incrementDepth()               { ++mDepth; }

    const glm::vec3& origin() const         { return mOrigin; }
    const glm::vec3& dir() const            { return mDir; }
    const glm::vec3& inverseDir() const     { return mInverseDir; }
    glm::vec3 point(const float t) const    { return mOrigin + mDir * t; }
    unsigned depth() const                  { return mDepth; }
    bool shouldHitBackFaces() const         { return mShouldHitBack; }
    bool didHitBackFace() const             { return mDidHitBack; }
    float ior() const                       { return mIor; }
    TYPE type() const                       { return mType; }
    
    void hit(const Triangle* primitive, float t,
             const glm::vec2& barycentrics, bool hitBackFace);
    bool hits(const float t) const { return t < mMaxT && t > mMinT; }
    
    // This method is bad and should go away. Assumes we already know p
    // lies on the ray.
    float t(const glm::vec3& p)  { return glm::length(p - mOrigin); }
    float maxT() const           { return mMaxT; }
    float minT() const           { return mMinT; }
    
    void transformed(const glm::mat4& m, Ray& outRay) const;
    void reflected(const Hit& h, Ray& r) const;
    bool refract(const Hit& hit, float destIOR);
    void reflect(const Hit& hit, const glm::vec3& I);
    
    inline glm::vec4 shade(const Raytracer& tracer) const;

    friend class Hit;
protected:
    Ray() : mType(PRIMARY) {}
    
private:
    const TYPE          mType;
    glm::vec3           mOrigin;
    glm::vec3           mDir;
    glm::vec3           mInverseDir;
    unsigned            mDepth;
    float               mIor;
    float               mMinT;
    float               mMaxT;
    glm::vec2           mHitBarycentrics;
    const Triangle*     mHitPrim;
    bool                mDidHitBack;
    bool                mShouldHitBack;
};


inline void Ray::hit(const Triangle* primitive, float t,
                     const glm::vec2& barycentrics, bool hitBackFace)
{
    TP_ASSERT(t <= mMaxT);

    mMaxT = t;
    mHitBarycentrics = barycentrics;
    mHitPrim = primitive;
    mDidHitBack = hitBackFace;
}

inline void Ray::reflect(const Hit& hit, const glm::vec3& I)
{
    setDir(I - 2.f * glm::dot(I, hit.N) * hit.N);
    setOrigin(hit.P);
}

inline void Ray::transformed(const glm::mat4& m, Ray& outRay) const
{
    // Apply translation to origin
    outRay.setOrigin(glm::vec3(m * glm::vec4(mOrigin, 1.0)));

    // Apply upper 3x3 rotation to direction
    outRay.setDir(glm::normalize(glm::mat3(m) * mDir));
}

inline glm::vec4 Ray::shade(const Raytracer& tracer) const
{
    return mHitPrim->material().shadeRay(tracer, *this);
}

#endif

