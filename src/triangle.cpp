#include "triangle.h"
#include "ray.h"
#include "hit.h"
#include "common.h"

Triangle::Triangle()
{
    mA = mB = mC = mN = glm::vec3(0.0f,0.0f,0.0f);
}

Triangle::Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, Material* m)
    : mA(a), mB(b), mC(c)
{
    mN = glm::normalize(glm::cross(mA - mB, mA - mC));
    mMaterial = m;
}

bool Triangle::intersect(Ray& ray, glm::vec3& p, bool backfacing) const
{
    float denom = glm::dot(*ray.dir(), mN);
    
    // > 0 -> backface
    if (denom > EPSILON)
        if (!backfacing) return false;
        
    // 0 -> parallel
    else if (denom > -EPSILON)
        return false;
    
    float numerator = glm::dot(mA, mN) - glm::dot(*ray.origin(), mN);
    float t = numerator / denom;
    if (t < 0.0f) return false;
    
    // Check if intersection with plane is inside triangle
    // using Barycentric Coordinates
    // http://www.blackpawn.com/texts/pointinpoly/default.html
    
    glm::vec3 P = ray.point(t);
    glm::vec3 v0 = mC - mA, v1 = mB - mA, v2 = P - mA;
    
    float dot00 = glm::dot(v0, v0);
    float dot01 = glm::dot(v0, v1);
    float dot02 = glm::dot(v0, v2);
    float dot11 = glm::dot(v1, v1);
    float dot12 = glm::dot(v1, v2);
    
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    
    if (u < 0.0f || v < 0.0f || (u + v) > 1.0f)
        return false;
    
    p = P;
    
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

bool Triangle::onLeftOfPlane(const float plane, const short axis) const
{
    return mA[axis] < plane && mB[axis] < plane && mC[axis] < plane;
}
