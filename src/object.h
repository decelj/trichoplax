#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <vector>
#include <glm/glm.hpp>

#include "iprimitive.h"
#include "material.h"
#include "ray.h"
#include "hit.h"
#include "raytracer.h"

class Raytracer;
class Material;

class Object {
public:
    explicit Object(glm::mat4& t, Material* m);
    ~Object();
    
    // IIntersection
    bool intersect(Ray& ray);
    bool intersect(Ray& ray, Hit* hit, bool backfacing);
    
    void computeNormal(Hit* hit);
    void insertPrimitive(IPrimitive* p);
    
    glm::vec3 shadePoint(const glm::vec3& p, const glm::vec3& n, const glm::vec3& I, Raytracer* const rt, const int depth) const;
    
private:
    std::vector<IPrimitive*> mPrimitives;
    Material* mMaterial;
    glm::mat4 mT, mTInverse, mTInverseTranspose;
};

#endif

