#ifndef __AABBOX_H__
#define __AABBOX_H__

#include <glm/glm.hpp>
#include "ray.h"

class AABBox {
public:
    AABBox() { };
    
    const inline glm::vec3& ll() const
    { return mBounds[0]; }
    const inline glm::vec3& ur() const
    { return mBounds[1]; }
    
    inline void update(const glm::vec3& ll, const glm::vec3& ur)
    { mBounds[0] = ll; mBounds[1] = ur; }
    bool intersect(const Ray& ray) const;
    
    short longestAxis() const;
    inline float split(const float axis) const
    {
        return ((mBounds[1][axis] - mBounds[0][axis]) / 2.f) + mBounds[0][axis];
    }
private:
    glm::vec3 mBounds[2];
};

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

#endif
