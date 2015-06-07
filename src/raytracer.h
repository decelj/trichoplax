#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

#include <glm/glm.hpp>
#include <pthread.h>

class Ray;
class KdTree;
class Camera;
class Sampler;
class ImageBuffer;
class Stats;
class StatsCollector;
class EnvSphere;
class IPrimitive;

class Raytracer
{
public:
	explicit Raytracer(const KdTree* tree, const Camera* cam, const EnvSphere* env,
                       Sampler* const sampler, ImageBuffer* const imgBuffer,
                       const unsigned int maxDepth);
	~Raytracer();
    
    void registerStatsCollector(StatsCollector* c) const;
    bool start();
    bool join() const;
    void cancel();
    
    bool traceAndShade(Ray& ray, glm::vec4& result) const;
    bool traceShadow(Ray& ray) const;
	
private:
    explicit Raytracer(); // Don't use the default constructor!
    
    // Not copyable
    Raytracer(const Raytracer&) = delete;
    Raytracer& operator=(const Raytracer&) = delete;
    
    static void* _run(void* arg);
    void run() const;
    
    bool trace(Ray& ray, bool firstHit=false) const;
    
    const KdTree* mKdTree;
    const Camera* mCamera;
    const EnvSphere* mEnv;
    ImageBuffer* const mImgBuffer;
    Sampler* const mSampler;
    Stats* mStats;
    const unsigned int mMaxDepth;
    pthread_t mThreadId;
    bool mIsCanceled;
    
    mutable bool* mPrimBuckets;
};

#endif


