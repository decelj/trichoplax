#include <algorithm>

#include "triangle.h"
#include "ray.h"
#include "hit.h"
#include "common.h"
#include "kdtree.h"

Triangle::Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, Material* m)
    : IPrimitive(),
    mA(a),
    mB(b),
    mC(c),
    mN(glm::normalize(glm::cross(mA - mB, mA - mC))),
    mCA(mC - mA),
    mBA(mB - mA),
    mDotAN(glm::dot(mA, mN))
{
    mMaterial = m;
}

bool Triangle::intersect(Ray& ray) const
{
    const float denom = glm::dot(ray.dir(), mN);
    
    // > 0 -> backface
    if (denom > EPSILON) {
        if (!ray.shouldHitBackFaces()) return false;
        
    // 0 -> parallel
    } else if (denom > -EPSILON) {
        return false;
    }
    
    const float numerator = mDotAN - glm::dot(ray.origin(), mN);
    const float t = numerator / denom;
    if (t < 0.0f || !ray.hits(t)) return false;
    
    // Check if intersection with plane is inside triangle
    // using Barycentric Coordinates
    // http://www.blackpawn.com/texts/pointinpoly/default.html
    
    const glm::vec3 P = ray.point(t);
    const glm::vec3 vPA = P - mA;
    
    const float dot00 = glm::dot(mCA, mCA);
    const float dot01 = glm::dot(mCA, mBA);
    const float dot02 = glm::dot(mCA, vPA);
    const float dot11 = glm::dot(mBA, mBA);
    const float dot12 = glm::dot(mBA, vPA);
    
    const float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    const float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    const float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    
    if (u < 0.0f || v < 0.0f || (u + v) > 1.0f)
        return false;
    
    ray.hit(this, t, denom > EPSILON);
    return true;
}

void Triangle::bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const
{
    lowerLeft[0] = MIN(MIN(mA[0], mB[0]), mC[0]);
    lowerLeft[1] = MIN(MIN(mA[1], mB[1]), mC[1]);
    lowerLeft[2] = MIN(MIN(mA[2], mB[2]), mC[2]);
    
    upperRight[0] = MAX(MAX(mA[0], mB[0]), mC[0]);
    upperRight[1] = MAX(MAX(mA[1], mB[1]), mC[1]);
    upperRight[2] = MAX(MAX(mA[2], mB[2]), mC[2]);
}

KdTree::PartitionResult Triangle::partition(const float plane, const short axis) const
{
    if (mA[axis] <= plane && mB[axis] <= plane && mC[axis] <= plane)
        return KdTree::LEFT;
    if (mA[axis] >= plane && mB[axis] >= plane && mC[axis] >= plane)
        return KdTree::RIGHT;
    return KdTree::BOTH;
}
