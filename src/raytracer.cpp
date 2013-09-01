#include <iostream>

#include "raytracer.h"
#include "ray.h"
#include "hit.h"
#include "iprimitive.h"
#include "material.h"

Raytracer::Raytracer()
 :  mKdTree(new KdTree()),
    mMaxDepth(5),
    mShadowRays(0),
    mShadingRays(0)
{ 
    pthread_mutex_init(&mStatsMutex, NULL);
}

Raytracer::~Raytracer()
{
    pthread_mutex_destroy(&mStatsMutex);
    delete mKdTree;
}

void Raytracer::printStats() const
{
    std::cout << "Shading rays: " << mShadingRays << std::endl;
    std::cout << "Shadow rays: " << mShadowRays << std::endl;
}

bool Raytracer::traceShadow(Ray& ray, const float distToLight)
{
    pthread_mutex_lock(&mStatsMutex);
    mShadowRays++;
    pthread_mutex_unlock(&mStatsMutex);
    
    // minDist is distToLight^2 because comparisions are to the square of the
    // distance to avoid sqrt in computation.
    Hit h;
    return trace(ray, h, true, distToLight*distToLight, true);
}

bool Raytracer::trace(Ray& ray, Hit& final, bool backfacing, float minDist, bool firstHit) const
{
    // Base case
    if (ray.depth() > mMaxDepth) return false;
    
    bool didHit = mKdTree->trace(ray, final, backfacing, minDist, firstHit);   
 
    if (didHit) {
        final.n = final.prim->normal(final.p);
        final.depth = ray.depth();
        final.I = -(*ray.dir());
    }
 
    return didHit;
}

bool Raytracer::traceAndShade(Ray& ray, glm::vec3* result)
{
    pthread_mutex_lock(&mStatsMutex);
    mShadingRays++;
    pthread_mutex_unlock(&mStatsMutex);
    
    Hit final;
    if (trace(ray, final)) {
        *result = final.prim->material()->shadePoint(final);
        return true;
    }
    
    return false;
}
