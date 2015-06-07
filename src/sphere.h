#ifndef __SPHERE_H__
#define __SPHERE_H__

#include <glm/glm.hpp>
#include <math.h>

#include "iprimitive.h"
#include "kdtree.h"

class Sphere: public IPrimitive
{
public:
    explicit Sphere(const glm::vec3& center, const float radius, Material *m, glm::mat4 t);

    // IPrimitive
    bool intersect(Ray& ray) const;
    glm::vec3 normal(const glm::vec3& p) const;
    void bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const;
    KdTree::PartitionResult partition(const float plane, const short axis) const;
    
private:
    glm::vec3 mCenter, mLL, mUR;
    float mRadius;
    glm::mat4 mT, mTInverse, mTInverseTranspose;
};

#endif
