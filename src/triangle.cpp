#include <algorithm>
#include <cmath>

#include "triangle.h"
#include "ray.h"
#include "common.h"
#include "vertex.h"


Triangle::Triangle(const Vertex* a, const Vertex* b, const Vertex* c, Material* material)
    : IPrimitive()
    , mA(a)
    , mB(b)
    , mC(c)
    , mNg(glm::normalize(glm::cross(mA->position - mB->position, mA->position - mC->position)))
{
    mMaterial = material;
}

bool Triangle::intersect(Ray& ray) const
{
    const float denom = glm::dot(ray.dir(), mNg);
    
    // > 0 -> backface
    if (denom > EPSILON)
    {
        if (!ray.shouldHitBackFaces()) return false;
    }
    // 0 -> parallel
    else if (denom > -EPSILON)
    {
        return false;
    }

    const float aDotN = glm::dot(mA->position, mNg);
    const float numerator = aDotN - glm::dot(ray.origin(), mNg);
    const float t = numerator / denom;
    if (t < 0.0f || !ray.hits(t)) return false;
    
    // Check if intersection with plane is inside triangle
    // using Barycentric Coordinates
    // http://www.blackpawn.com/texts/pointinpoly/default.html
    
    const glm::vec3 P = ray.point(t);
    const glm::vec3 vPA = P - mA->position;
    const glm::vec3 vCA = mC->position - mA->position;
    const glm::vec3 vBA = mB->position - mA->position;
    
    const float dot00 = glm::dot(vCA, vCA);
    const float dot01 = glm::dot(vCA, vBA);
    const float dot02 = glm::dot(vCA, vPA);
    const float dot11 = glm::dot(vBA, vBA);
    const float dot12 = glm::dot(vBA, vPA);
    
    const float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    const glm::vec2 barycentrics(
        (dot11 * dot02 - dot01 * dot12) * invDenom,
        (dot00 * dot12 - dot01 * dot02) * invDenom);
    
    if (barycentrics.x < 0.0f || barycentrics.y < 0.0f
        || (barycentrics.x + barycentrics.y) > 1.0f)
    {
        return false;
    }
    
    ray.hit(this, t, barycentrics, denom > EPSILON);
    return true;
}

glm::vec3 Triangle::normal(const glm::vec3& p, const glm::vec2& barycentrics) const
{
    // Interpolate the shading normal using the barycentrics
    glm::vec3 normal = mB->normal * barycentrics.x
        + mC->normal * barycentrics.y
        + mA->normal * (1.f - (barycentrics.x + barycentrics.y));

    return glm::normalize(normal);
}

inline void Triangle::bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const
{
    lowerLeft[0] = std::min(std::min(mA->position[0], mB->position[0]), mC->position[0]);
    lowerLeft[1] = std::min(std::min(mA->position[1], mB->position[1]), mC->position[1]);
    lowerLeft[2] = std::min(std::min(mA->position[2], mB->position[2]), mC->position[2]);

    upperRight[0] = std::max(std::max(mA->position[0], mB->position[0]), mC->position[0]);
    upperRight[1] = std::max(std::max(mA->position[1], mB->position[1]), mC->position[1]);
    upperRight[2] = std::max(std::max(mA->position[2], mB->position[2]), mC->position[2]);
}

bool Triangle::isCoplaner(const float plane, const unsigned aaAxis) const
{
    if (relEq(fabsf(mNg[aaAxis]), 1.0))
    {
        return relEq(mA->position[aaAxis] - plane, 0.f);
    }

    return false;
}

bool Triangle::isOrthognalToAxis(const unsigned axis) const
{
    return relEq(fabsf(mNg[(axis + 1) % 3]), 1.0);
}

void Triangle::aaBoxClip(float start, float end, unsigned aaAxis, float* outStart, float* outEnd) const
{
    TP_ASSERT(!isCoplaner(start, aaAxis));
    TP_ASSERT(!isCoplaner(end, aaAxis));

    *outStart = std::max(start,
                         std::min(std::min(mA->position[aaAxis], mB->position[aaAxis]), mC->position[aaAxis]));
    *outEnd = std::min(end,
                       std::max(std::max(mA->position[aaAxis], mB->position[aaAxis]), mC->position[aaAxis]));
}
