#ifndef __RAY_H__
#define __RAY_H__

#include <glm/glm.hpp>

class Hit;
class IPrimitive;
class Raytracer;

class Ray
{
public:
    enum TYPE {
        PRIMARY = 0,
        REFLECTED,
        REFRACTED,
        SHADOW,
        TYPE_COUNT
    };
    
    explicit Ray(const TYPE t);
    explicit Ray(const Ray& r);
    explicit Ray(const Ray& r, const TYPE t);
    explicit Ray(const TYPE t, const glm::vec3& origin, const float ior);
    
    inline void setDir(glm::vec3 d) { mDir = d; mInverseDir = 1.0f / d; }
    inline void setOrigin(glm::vec3 o) { mOrigin = o; }
    inline void bias(const float bias) { mOrigin += mDir * bias; }
    inline void setMaxDistance(const float dist) { mMaxT = dist; }
    inline void shouldHitBackFaces(bool value) { mHitBack = value; }

    inline const glm::vec3& origin() const { return mOrigin; }
    inline const glm::vec3& dir() const { return mDir; }
    inline const glm::vec3& inverseDir() const { return mInverseDir; }
    inline glm::vec3 point(const float t) const { return mOrigin + mDir * t; }
    inline short depth() const { return mDepth; }
    inline bool shouldHitBackFaces() const { return mHitBack; }
    inline float ior() const { return mIor; }
    inline TYPE type() const { return mType; }
    
    inline void hit(const IPrimitive* prim, const float t, bool hit_back)
    { mMaxT = t; mHitPrim = prim; mHitBack = hit_back; }
    inline bool hits(const float t) const { return t < mMaxT && t > mMinT; }
    
    // This method is bad and should go away. Assumes we already know p
    // lies on the ray.
    inline float t(const glm::vec3& p) { return glm::length(p - mOrigin); }
    inline float maxT() const { return mMaxT; }
    inline float minT() const { return mMinT; }
    
    void transformed(const glm::mat4& m, Ray& outRay) const;
    void reflected(const Hit& h, Ray& r) const;
    bool refracted(const Hit& h, Ray& r) const;
    
    void shade(const Raytracer* tracer, glm::vec4& result) const;

    friend class Hit;
protected:
    Ray() : mType(PRIMARY) {}
    
private:
    const TYPE mType;
    glm::vec3 mOrigin;
    glm::vec3 mDir, mInverseDir;
    short mDepth;
    float mIor;
    float mMinT, mMaxT;
    bool mHitBack;
    bool mShouldHitBack;
    const IPrimitive *mHitPrim;
};

#endif

