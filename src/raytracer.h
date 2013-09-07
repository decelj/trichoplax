#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

#include <glm/glm.hpp>
#include <vector>
#include <pthread.h>

#include "kdtree.h"

class Ray;
class IPrimitive;

class Raytracer
{
public:
	explicit Raytracer();
	~Raytracer();
	
    bool traceAndShade(Ray& ray, glm::vec4& result);
    bool traceShadow(Ray& ray);
    
	inline void addPrimitive(IPrimitive* p) { mKdTree->addPrimitive(p); }
    inline void setMaxDepth(int depth) { mMaxDepth = depth; }
    inline void finalize() { mKdTree->build(); }
    
    void printStats() const;
	
private:
    bool trace(Ray& ray, bool backfacing = false, bool firstHit = false) const;
    
    KdTree* mKdTree;
    int mMaxDepth;
    unsigned int mShadowRays, mShadingRays;
    pthread_mutex_t mStatsMutex;
};

#endif


