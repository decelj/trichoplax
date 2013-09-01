#include "sphere.h"
#include "ray.h"
#include "hit.h"
#include "common.h"

Sphere::Sphere(const glm::vec3& center, const float radius, Material *m, glm::mat4 t) :
    mCenter(center), mRadius(radius), mT(t)
{
    mTInverse = glm::inverse(mT);
    mTInverseTranspose = glm::transpose(mTInverse);
    mMaterial = m;
}

bool Sphere::intersect(Ray& ray, glm::vec3& p, bool backfacing) const
{
    // Transform the ray and extract the direction
    // and origin
    Ray transformed(ray);
    transformed.transform(mTInverse);
    const glm::vec3* ro = transformed.origin();
    const glm::vec3* rd = transformed.dir();
 
    // t^2(P1 . P1) + 2tP1 . (Po - C) + (Po - C) . (Po - C) - r^2 = 0
    glm::vec3 rToCenter = *ro - mCenter;
    float a = glm::dot(*rd, *rd);
    float b = 2.0 * glm::dot(*rd, rToCenter);
    float c = glm::dot(rToCenter, rToCenter) - mRadius * mRadius;

    float discriminat = b * b - 4.0 * a * c;
    if (discriminat < 0) return false; // Non-real roots, doesn't hit sphere

    float sqrtDisc = sqrtf(discriminat);
    float sln1 = (-1.0 * b + sqrtDisc) / (2.0 * a);
    float sln2 = (-1.0 * b - sqrtDisc) / (2.0 * a);

    float t;
    if (sln1 < 0) {
        if (sln2 < 0) return false; // Negative roots, sphere behind point
        t = sln2;
    } else if (sln2 < 0) {
        t = sln1;
    } else {
        t = sln1 < sln2 ? sln1 : sln2;
    }

    p = glm::vec3(mT * glm::vec4(transformed.point(t), 1.0));
    
    return true;
}

glm::vec3 Sphere::normal(const glm::vec3& p) const
{
    // TODO: This is kinda bad, we should fix it
    // p is in world space, transform it to object space
    // find the normal, transform the normal back to world
    // space.
    return glm::normalize(glm::vec3(mTInverseTranspose * glm::vec4(glm::vec3(mTInverse * glm::vec4(p, 1.0f)) - mCenter, 0.0f)));
}

void Sphere::bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const
{
    lowerLeft = glm::vec3(mT * glm::vec4(mCenter - mRadius, 1.0f));
    upperRight = glm::vec3(mT * glm::vec4(mCenter + mRadius, 1.0f));
}

bool Sphere::onLeftOfPlane(const float plane, const short axis) const
{
    float transformedCoord = mCenter[axis] + mRadius;
    transformedCoord = (transformedCoord*mT[0][axis] + transformedCoord*mT[1][axis] + transformedCoord*mT[2][axis] + mT[3][axis]);
    return transformedCoord < plane;
}

