#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include <glm/glm.hpp>

#include "iprimitive.h"

class Vertex;


class Triangle : public IPrimitive {
public:
    Triangle(const Vertex* a, const Vertex* b, const Vertex* c, Material* material);
    
    // IPrimitive
    bool intersect(Ray& ray) const override;
    glm::vec3 normal(const glm::vec3& p, const glm::vec2& barycentrics) const override;
    void bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const override;
    bool isCoplaner(const float plane, const unsigned aaAxis) const override;
    bool isOrthognalToAxis(const unsigned axis) const override;
    void aaBoxClip(float start, float end, unsigned aaAxis, float* outStart, float* outEnd) const override;
    
private:
    Triangle(const Triangle&) = delete;
    Triangle& operator=(const Triangle&) = delete;
    
    const Vertex*   mA;
    const Vertex*   mB;
    const Vertex*   mC;
    glm::vec3       mNg;
};

#endif

