#ifndef __AABBOX_H__
#define __AABBOX_H__

#include <glm/glm.hpp>
#include <limits>
#include "ray.h"

class AABBox {
public:
    AABBox();
    AABBox(const glm::vec3& ll, const glm::vec3& ur);
    
    const glm::vec3& ll() const { return mBounds[0]; }
    const glm::vec3& ur() const { return mBounds[1]; }
    bool isValid() const;
    
    float width() const;
    float height() const;
    float depth() const;
    
    void update(const glm::vec3& ll, const glm::vec3& ur)  { mBounds[0] = ll; mBounds[1] = ur; }
    void setLL(const glm::vec3& ll)                        { mBounds[0] = ll; }
    void setUR(const glm::vec3& ur)                        { mBounds[1] = ur; }
    void encompass(const glm::vec3& point);
    
    bool intersect(const Ray& ray) const;
    AABBox intersection(const AABBox& other) const;
    
    short longestAxis() const;
    float split(const unsigned axis) const
    {
        return ((mBounds[1][axis] - mBounds[0][axis]) / 2.f) + mBounds[0][axis];
    }
    
    float surfaceArea() const;
    void split(AABBox* outLeft, AABBox* outRight, unsigned axis, float plane) const;
private:
    glm::vec3 mBounds[2];
};


inline bool AABBox::isValid() const
{
    return glm::all(glm::notEqual(ll(), glm::vec3(std::numeric_limits<float>::max())))
        && glm::all(glm::notEqual(ur(), glm::vec3(std::numeric_limits<float>::max())));
}

inline bool AABBox::intersect(const Ray& ray) const
{
    const glm::vec3 llToOrigin = mBounds[0] - ray.origin();
    const glm::vec3 urToOrigin = mBounds[1] - ray.origin();
    const glm::vec3 nearTs = llToOrigin * ray.inverseDir();
    const glm::vec3 farTs = urToOrigin * ray.inverseDir();
    float tMin = ray.minT();
    float tMax = ray.maxT();
    
    for (int i = 0; i < 3; ++i) {
        float nearT = nearTs[i];
        float farT = farTs[i];
        if (nearT > farT) {
            float tmp = nearT;
            nearT = farT;
            farT = tmp;
        }
        
        tMin = std::max(nearT, tMin);
        tMax = std::min(farT, tMax);
        if (tMin > tMax)
            return false;
    }
    
    return true;
}

inline float AABBox::width() const
{
    return mBounds[1].x - mBounds[0].x;
}

inline float AABBox::height() const
{
    return mBounds[1].y - mBounds[0].y;
}

inline float AABBox::depth() const
{
    return mBounds[1].z - mBounds[0].z;
}

inline float AABBox::surfaceArea() const
{
    return (
            width() * height() * 2.f +
            depth() * height() * 2.f +
            width() * depth() * 2.f
            );
}

inline AABBox AABBox::intersection(const AABBox& other) const
{
    glm::bvec3 overlaps = glm::not_(glm::lessThan(ur(), other.ll()));
    if (!glm::all(overlaps))
    {
        return AABBox();
    }
    
    glm::vec3 outUR = glm::min(ur(), other.ur());
    glm::vec3 outLL = glm::max(ll(), other.ll());
    return AABBox(outLL, outUR);
}

inline void AABBox::encompass(const glm::vec3& point)
{
    mBounds[0] = glm::min(mBounds[0], point);
    mBounds[1] = glm::max(mBounds[1], point);
}

#endif
