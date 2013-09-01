
#include "object.h"
#include "debug.h"

Object::Object(glm::mat4& t, Material* m)
    : mMaterial(m), mT(t)
{
    mPrimitives.clear();
    print4x4(t);
    mTInverse = glm::inverse(mT);
    mTInverseTranspose = glm::transpose(mTInverse);
}

Object::~Object()
{
    delete mMaterial;
    for (std::vector<IPrimitive*>::iterator it = mPrimitives.begin(); it != mPrimitives.end(); ++it)
        delete *it;
}

void Object::insertPrimitive(IPrimitive* p)
{
    mPrimitives.push_back(p);
}

bool Object::intersect(Ray& ray)
{
    // TODO: Optimize, return as soon as we hit an object
    // Maybe not, when we get to arealights...
    return intersect(ray, NULL, false);
}

bool Object::intersect(Ray& ray, Hit* hit, bool backfacing)
{
    Hit h;
    float minDist = MAXFLOAT;
    
    // Transform ray into this object's space
    // TODO: Don't pass ray by reference
    Ray rayObjectSpace(ray);
    rayObjectSpace.transform(mTInverse);
    
    for (std::vector<IPrimitive*>::iterator it = mPrimitives.begin(); it != mPrimitives.end(); ++it) {
        if ((*it)->intersect(rayObjectSpace, &h, backfacing) && h.t < minDist) {
            *hit = h;
            minDist = h.t;
        }
    }
    
    // Transform hit point to world space
    if (minDist < MAXFLOAT) {
        hit->p = glm::vec3(mT * glm::vec4(hit->p, 1));
        hit->obj = this;
        return true;
    }
    
    return false;
}

void Object::computeNormal(Hit* hit)
{
    printf("HERE!\n");
    //hit->prim->computeNormal(hit->p, &(hit->n));
    hit->n = glm::vec3(mTInverseTranspose * glm::vec4(hit->n, 0));
}

glm::vec3 Object::shadePoint(const glm::vec3& p, const glm::vec3& n, const glm::vec3& I, Raytracer* const rt, const int depth) const
{
    // Transform normal to world space
    glm::vec3 nWS = glm::normalize(glm::vec3(mTInverseTranspose * glm::vec4(n, 0)));
    return mMaterial->shadePoint(rt, p, nWS, I, depth);
}
