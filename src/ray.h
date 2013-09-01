#ifndef __RAY_H__
#define __RAY_H__

#include <glm/glm.hpp>

class Ray
{
public:
    explicit Ray() : mOrigin(0,0,0), mDir(0,0,0), mDepth(1) { }
    explicit Ray(const glm::vec3& origin, const glm::vec3& dir, const int depth)
    : mOrigin(origin), mDir(dir), mDepth(depth) { mInverseDir = 1.0f / dir; }
    explicit Ray(const glm::vec3& origin) : mOrigin(origin), mDir(0,0,0), mDepth(1) { }
    explicit Ray(const Ray& r)
    : mOrigin(r.mOrigin), mDir(r.mDir), mDepth(r.mDepth), mInverseDir(r.mInverseDir) { }

    inline void setDir(glm::vec3 d) { mDir = d; mInverseDir = 1.0f / d; }
    inline void setOrigin(glm::vec3 o) { mOrigin = o; }

    inline const glm::vec3* origin() const { return &mOrigin; }
    inline const glm::vec3* dir() const { return &mDir; }
    inline const glm::vec3* inverseDir() const { return &mInverseDir; }
    inline glm::vec3 point(const float t) const { return mOrigin + mDir * t; }
    inline int depth() const { return mDepth; }
    
    void transform(const glm::mat4& m);
    inline void bias(const float bias) { mOrigin += mDir * bias; }

private:
    glm::vec3 mOrigin;
    glm::vec3 mDir, mInverseDir;
    int mDepth;
};

#endif

