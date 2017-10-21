#include "sphere.h"
#include "ray.h"
#include "hit.h"
#include "common.h"

#include <algorithm>
#include <limits>

Sphere::Sphere(const glm::vec3& center, const float radius, Material *m, glm::mat4 t)
    : IPrimitive()
    , mCenter(center)
    , mLL(std::numeric_limits<float>::max())
    , mUR(std::numeric_limits<float>::lowest())
    , mRadius(radius)
    , mT(t)
{
    mTInverse = glm::inverse(mT);
    mTInverseTranspose = glm::transpose(mTInverse);
    mMaterial = m;
    
    // Precompute the AABB bonds for this sphere
    glm::vec3 utll = mCenter - mRadius;
    
    glm::vec3 points[8];
    points[0] = glm::vec3(mT * glm::vec4(utll, 1.f));
    points[1] = glm::vec3(mT * glm::vec4(utll + glm::vec3(0, mRadius*2, 0), 1.f));
    points[2] = glm::vec3(mT * glm::vec4(utll + glm::vec3(0, mRadius*2, mRadius*2), 1.f));
    points[3] = glm::vec3(mT * glm::vec4(utll + glm::vec3(0, 0, mRadius*2), 1.f));
    points[4] = glm::vec3(mT * glm::vec4(utll + glm::vec3(mRadius*2, 0, 0), 1.f));
    points[5] = glm::vec3(mT * glm::vec4(utll + glm::vec3(mRadius*2, mRadius*2, 0), 1.f));
    points[6] = glm::vec3(mT * glm::vec4(utll + glm::vec3(mRadius*2, 0, mRadius*2), 1.f));
    points[7] = glm::vec3(mT * glm::vec4(utll + glm::vec3(mRadius*2, mRadius*2, mRadius*2), 1.f));
    
    for (unsigned i = 0; i < 8; ++i)
    {
        for (unsigned int j = 0; j < 3; ++j)
        {
            mLL[j] = std::min(mLL[j], points[i][j]);
            mUR[j] = std::max(mUR[j], points[i][j]);
        }
    }
}

bool Sphere::intersect(Ray& ray) const
{
    // Transform the ray and extract the direction
    // and origin
    Ray transformed(ray.type());
    ray.transformed(mTInverse, transformed);
    const glm::vec3& ro = transformed.origin();
    const glm::vec3& rd = transformed.dir();
 
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
    if (sln1 < 0.f)
    {
        if (!ray.shouldHitBackFaces() || sln2 < 0.f) return false;
        t = sln2;
    }
    else if (sln2 < 0.f)
    {
        if (!ray.shouldHitBackFaces()) return false;
        t = sln1;
    }
    else
    {
        t = sln1 < sln2 ? sln1 : sln2;
    }
    
    // TODO: This is bad. AKA, slow.
    //       Try to find transformation for mHitT by mTInverse;
    //       test against transformed t, if hit transform t by mT
    glm::vec3 hitP = glm::vec3(mT * glm::vec4(transformed.point(t), 1.f));
    t = ray.t(hitP);
    if (ray.hits(t))
    {
        ray.hit(this, t, glm::vec2(0.f), sln1 < 0.f || sln2 < 0.f);
        return true;
    }
    
    return false;
}

glm::vec3 Sphere::normal(const glm::vec3& p, const glm::vec2& /*barycentrics*/) const
{
    // TODO: This is kinda bad, we should fix it
    // p is in world space, transform it to object space
    // find the normal, transform the normal back to world
    // space.
    return glm::normalize(glm::mat3(mTInverseTranspose) * glm::vec3(mTInverse * glm::vec4(p, 1.0f)) - mCenter);
}

glm::vec2 Sphere::uv(const glm::vec2& /*barycentrics*/) const
{
    return glm::vec2(0.f);
}

void Sphere::positionPartials(const glm::vec3& N, glm::vec3& dPdU, glm::vec3& dPdV) const
{
    generateTangents(N, dPdU, dPdV);
}

void Sphere::bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const
{
    lowerLeft = mLL;
    upperRight = mUR;
}

bool Sphere::isCoplaner(const float /*plane*/, const unsigned int /*aaAxis*/) const
{
    return false;
}

bool Sphere::isOrthognalToAxis(const unsigned axis) const
{
    return false;
}

void Sphere::aaBoxClip(float start, float end, unsigned aaAxis, float* outStart, float* outEnd) const
{
    *outStart = std::max(start, mLL[aaAxis]);
    *outEnd = std::min(end, mUR[aaAxis]);
}
