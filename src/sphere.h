#ifndef __SPHERE_H__
#define __SPHERE_H__

#include <glm/glm.hpp>
#include <algorithm>

#include "iprimitive.h"


class Sphere: public IPrimitive
{
public:
    explicit Sphere(const glm::vec3& center, const float radius, Material *m, glm::mat4 t);

    // IPrimitive
    bool intersect(Ray& ray) const;
    glm::vec3 normal(const glm::vec3& p) const;
    void bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const;
    bool isCoplaner(const float plane, const unsigned aaAxis) const;
    bool isOrthognalToAxis(const unsigned axis) const;
    void aaBoxClip(const float start, const float end, const unsigned aaAxis, float* outStart, float* outEnd) const;
    
private:
    glm::vec3 mCenter, mLL, mUR;
    float mRadius;
    glm::mat4 mT, mTInverse, mTInverseTranspose;
};


inline void Sphere::bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const
{
    lowerLeft = mLL;
    upperRight = mUR;
}

inline bool Sphere::isCoplaner(const float /*plane*/, const unsigned int /*aaAxis*/) const
{
    return false;
}

inline bool Sphere::isOrthognalToAxis(const unsigned axis) const
{
    return false;
}

inline void Sphere::aaBoxClip(const float start, const float end, const unsigned aaAxis, float* outStart, float* outEnd) const
{
    *outStart = std::max(start, mLL[aaAxis]);
    *outEnd = std::min(end, mUR[aaAxis]);
}

#endif
