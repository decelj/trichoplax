#ifndef __AABBOX_H__
#define __AABBOX_H__

#include <glm/glm.hpp>

class Ray;

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

#endif
