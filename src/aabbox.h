#ifndef __AABBOX_H__
#define __AABBOX_H__

#include <glm/glm.hpp>

class Ray;

class AABBox {
public:
    AABBox() { };
    
    inline void update(glm::vec3& ll, glm::vec3 ur) { mBounds[0] = ll; mBounds[1] = ur; }
    bool intersect(const Ray& ray) const;
    
    short longestAxis() const;
    float split(const short axis) const;
private:
    glm::vec3 mBounds[2];
};

#endif
