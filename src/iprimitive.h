#ifndef __IPrimitive_H__
#define __IPrimitive_H__

#include <glm/glm.hpp>
#include <assert.h>

#include "kdtree.h"

class Material;
class Ray;
class AABBox;

class IPrimitive
{
private:
    size_t mId;
    
protected:
    Material* mMaterial;
    
public:
    IPrimitive() {
        static size_t id = 0;
        mId = id++;
    }
    
    virtual ~IPrimitive() { }
    virtual bool intersect(Ray& ray) const = 0;
    virtual glm::vec3 normal(const glm::vec3& p, const glm::vec2& barycentrics) const = 0;
    virtual glm::vec2 uv(const glm::vec2& barycentrics) const = 0;
    virtual void positionPartials(const glm::vec3& N, glm::vec3& dPdU, glm::vec3& dPdV) const = 0;
    virtual void bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const = 0;
    virtual bool isCoplaner(const float plane, const unsigned aaAxis) const = 0;
    virtual bool isOrthognalToAxis(const unsigned axis) const = 0;
    virtual void aaBoxClip(float start, float end, unsigned aaAxis, float* outStart, float* outEnd) const = 0;
    
    inline Material* material() const { return mMaterial; }
    inline const size_t& id() const { return mId; }
};

#endif

