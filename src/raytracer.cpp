#include <iostream>

#include "raytracer.h"
#include "ray.h"
#include "iprimitive.h"
#include "material.h"
#include "hit.h"

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

bool Raytracer::traceShadow(Ray& ray)
{
    pthread_mutex_lock(&mStatsMutex);
    mShadowRays++;
    pthread_mutex_unlock(&mStatsMutex);
    
    return trace(ray, true);
}

bool Raytracer::trace(Ray& ray, bool firstHit) const
{
    // Base case
    if (ray.depth() > mMaxDepth) return false;
    
    return mKdTree->trace(ray, firstHit);
}

bool Raytracer::traceAndShade(Ray& ray, glm::vec4& result)
{
    pthread_mutex_lock(&mStatsMutex);
    mShadingRays++;
    pthread_mutex_unlock(&mStatsMutex);
    
    if (trace(ray)) {
        ray.shade(result);
        return true;
    }
    
    return false;
}
