#ifndef __IPrimitive_H__
#define __IPrimitive_H__

#include <glm/glm.hpp>
#include <assert.h>

class Material;
class Ray;
struct Hit;

class IPrimitive
{
protected:
    Material* mMaterial;
    
public:
    virtual bool intersect(Ray& ray, glm::vec3& p, bool backfacing) const = 0;
    virtual glm::vec3 normal(const glm::vec3& p) const = 0;
    virtual void bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const = 0;
    virtual bool onLeftOfPlane(const float plane, const short axis) const = 0;
    inline Material* material() const { return mMaterial; }
};

#endif

