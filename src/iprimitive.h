#ifndef __IPrimitive_H__
#define __IPrimitive_H__

#include <glm/glm.hpp>
#include <assert.h>

#include "kdtree.h"

class Material;
class Ray;

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
    virtual glm::vec3 normal(const glm::vec3& p) const = 0;
    virtual void bounds(glm::vec3& lowerLeft, glm::vec3& upperRight) const = 0;
    virtual KdTree::PartitionResult partition(const float plane, const short axis) const = 0;
    inline Material* material() const { return mMaterial; }
    inline const size_t& id() const { return mId; }
};

#endif

