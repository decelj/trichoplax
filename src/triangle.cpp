#include "triangle.h"
#include "ray.h"
#include "common.h"


Triangle::Triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, Material* m)
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
