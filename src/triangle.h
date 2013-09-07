#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include <glm/glm.hpp>
#include <math.h>

#include "iprimitive.h"

class Triangle : public IPrimitive {
public:
    Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, Material *m);
    
    // IPrimitive
    bool intersect(Ray& ray, bool backfacing) const;
    glm::vec3 normal(const glm::vec3&) const { return mN; }
    void bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const;
    bool onLeftOfPlane(const float plane, const short axis) const;
    
private:
    Triangle();
    
    glm::vec3 mA, mB, mC, mN;
};

#endif

