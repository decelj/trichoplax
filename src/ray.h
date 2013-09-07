#ifndef __RAY_H__
#define __RAY_H__

#include <glm/glm.hpp>

class Hit;
class IPrimitive;

class Ray
{
public:
    explicit Ray();
    explicit Ray(const Ray& r);
    explicit Ray(const glm::vec3& origin, const float ior, const short depth);
    
    inline void setDir(glm::vec3 d) { mDir = d; mInverseDir = 1.0f / d; }
    inline void setOrigin(glm::vec3 o) { mOrigin = o; }
    inline void bias(const float bias) { mOrigin += mDir * bias; }
    inline void setMinDistance(const float dist) { mHitT = dist; }

    inline const glm::vec3& origin() const { return mOrigin; }
    inline const glm::vec3& dir() const { return mDir; }
    inline const glm::vec3& inverseDir() const { return mInverseDir; }
    inline glm::vec3 point(const float t) const { return mOrigin + mDir * t; }
    inline short depth() const { return mDepth; }
    
    inline void hit(const IPrimitive* prim, const float t, bool hit_back)
    { mHitT = t; mHitPrim = prim; mHitBack = hit_back; }
    inline bool hits(const float t) const { return t < mHitT; }
    
    void transformed(const glm::mat4& m, Ray& outRay) const;
    void reflected(const Hit& h, Ray& r) const;
    void refracted(const Hit& h, Ray& r) const;
    
    void shade(glm::vec4& result) const;

    friend class Hit;
private:
    glm::vec3 mOrigin;
    glm::vec3 mDir, mInverseDir;
    short mDepth;
    float mIor;
    float mHitT;
    bool mHitBack;
    const IPrimitive *mHitPrim;
};

#endif

