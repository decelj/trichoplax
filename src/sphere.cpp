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

bool Sphere::intersect(Ray& ray) const
{
    // Transform the ray and extract the direction
    // and origin
    Ray transformed(ray.type());
    ray.transformed(mTInverse, transformed);
    const glm::vec3 ro = transformed.origin();
    const glm::vec3 rd = transformed.dir();
 
    // t^2(P1 . P1) + 2tP1 . (Po - C) + (Po - C) . (Po - C) - r^2 = 0
    const glm::vec3 rToCenter = ro - mCenter;
    const float a = glm::dot(rd, rd);
    const float b = 2.f * glm::dot(rd, rToCenter);
    const float c = glm::dot(rToCenter, rToCenter) - mRadius * mRadius;

    const float discriminat = b * b - 4.f * a * c;
    if (discriminat < 0.f) return false; // Non-real roots, doesn't hit sphere

    const float sqrtDisc = sqrtf(discriminat);
    const float sln1 = (-1.f * b + sqrtDisc) / (2.f * a);
    const float sln2 = (-1.f * b - sqrtDisc) / (2.f * a);

    // Two negative roots -> sphere behind point
    // One negative, one positive root -> point inside sphere, hit backface
    // Two positive roots -> point outside sphere
    float t;
    if (sln1 < 0.f) {
        if (!ray.shouldHitBackFaces() || sln2 < 0.f) return false;
        t = sln2;
    } else if (sln2 < 0.f) {
        if (!ray.shouldHitBackFaces()) return false;
        t = sln1;
    } else {
        t = sln1 < sln2 ? sln1 : sln2;
    }
    
    // TODO: This is bad. AKA, slow.
    //       Try to find transformation for mHitT by mTInverse;
    //       test against transformed t, if hit transform t by mT
    glm::vec3 hitP = glm::vec3(mT * glm::vec4(transformed.point(t), 1.f));
    t = ray.t(hitP);
    if (ray.hits(t)) {
        ray.hit(this, t, sln1 < 0.f || sln2 < 0.f);
        return true;
    }
    
    return false;
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

