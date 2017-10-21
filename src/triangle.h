#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include <glm/glm.hpp>
#include "common.h"

class Ray;
class Vertex;
class Material;
class Raytracer;


class Triangle
{
public:
    Triangle(const Vertex* a, const Vertex* b, const Vertex* c, const Material* material);

    void SetID(size_t id);
    size_t id() const;

    bool intersect(Ray& ray) const;
    const Material& material() const;

    const glm::vec3& normal() const;
    glm::vec3 interpolateNormal(const glm::vec3& p, const glm::vec2& barycentrics) const;
    glm::vec2 uv(const glm::vec2& barycentrics) const;
    void positionPartials(const glm::vec3& N, glm::vec3& dPdU, glm::vec3& dPdV) const;

    void bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const;
    bool isCoplaner(const float plane, const unsigned aaAxis) const;
    bool isOrthognalToAxis(const unsigned axis) const;
    void aaBoxClip(float start, float end, unsigned aaAxis, float* outStart, float* outEnd) const;
    
private:
    Triangle(const Triangle&) = delete;
    Triangle& operator=(const Triangle&) = delete;

    size_t          mID;
    const Vertex*   mA;
    const Vertex*   mB;
    const Vertex*   mC;
    glm::vec3       mNg;
    const Material* mMaterial;
};


inline void Triangle::SetID(size_t id)
{
    TP_ASSERT(mID == 0);
    mID = id;
}

inline size_t Triangle::id() const
{
    return mID;
}

inline const Material& Triangle::material() const
{
    return *mMaterial;
}

inline const glm::vec3& Triangle::normal() const
{
    return mNg;
}

#endif

