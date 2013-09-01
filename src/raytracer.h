#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

#include <glm/glm.hpp>
#include <vector>
#include <pthread.h>

#include "kdtree.h"

class Ray;
class IPrimitive;
struct Hit;

class Raytracer
{
public:
	explicit Raytracer();
	~Raytracer();
	
    bool traceAndShade(Ray& ray, glm::vec3* result);
    bool traceShadow(Ray& ray, const float distToLight);
    
	inline void addPrimitive(IPrimitive* p) { mKdTree->addPrimitive(p); }
    inline void setMaxDepth(int depth) { mMaxDepth = depth; }
    inline void finalize() { mKdTree->build(); }
    
    void printStats() const;
	
private:
    bool trace(Ray& ray, Hit& final, bool backfacing = false, float minDist = MAXFLOAT, bool firstHit = false) const;
    
    KdTree* mKdTree;
    int mMaxDepth;
    unsigned int mShadowRays, mShadingRays;
    pthread_mutex_t mStatsMutex;
};

#endif


