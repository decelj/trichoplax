#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

#include "iprimitive.h"
#include "common.h"


class Triangle : public IPrimitive {
public:
    Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, Material *m);
    
    // IPrimitive
    bool intersect(Ray& ray) const;
    glm::vec3 normal(const glm::vec3&) const { return mN; }
    void bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const;
    bool isCoplaner(const float plane, const unsigned aaAxis) const;
    bool isOrthognalToAxis(const unsigned axis) const;
    void aaBoxClip(const float start, const float end, const unsigned aaAxis, float* outStart, float* outEnd) const;
    
private:
    Triangle(const Triangle&) = delete;
    Triangle& operator=(const Triangle&) = delete;
    
    glm::vec3 mA, mB, mC, mN, mCA, mBA;
    float mDotAN;
};


inline void Triangle::bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const
{
    lowerLeft[0] = std::min(std::min(mA[0], mB[0]), mC[0]);
    lowerLeft[1] = std::min(std::min(mA[1], mB[1]), mC[1]);
    lowerLeft[2] = std::min(std::min(mA[2], mB[2]), mC[2]);
    
    upperRight[0] = std::max(std::max(mA[0], mB[0]), mC[0]);
    upperRight[1] = std::max(std::max(mA[1], mB[1]), mC[1]);
    upperRight[2] = std::max(std::max(mA[2], mB[2]), mC[2]);
}

inline bool Triangle::isCoplaner(const float plane, const unsigned aaAxis) const
{
    if (relEq(fabsf(mN[aaAxis]), 1.0))
    {
        return relEq(mA[aaAxis] - plane, 0.f);
    }
    
    return false;
}

inline bool Triangle::isOrthognalToAxis(const unsigned axis) const
{
    return relEq(fabsf(mN[(axis + 1) % 3]), 1.0);
}

inline void Triangle::aaBoxClip(const float start, const float end, const unsigned aaAxis, float* outStart, float* outEnd) const
{
    TP_ASSERT(!isCoplaner(start, aaAxis));
    TP_ASSERT(!isCoplaner(end, aaAxis));
    
    *outStart = std::max(start, std::min(std::min(mA[aaAxis], mB[aaAxis]), mC[aaAxis]));
    *outEnd = std::min(end, std::max(std::max(mA[aaAxis], mB[aaAxis]), mC[aaAxis]));
}

#endif

