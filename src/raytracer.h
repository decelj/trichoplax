#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

#include <glm/glm.hpp>
#include <vector>
#include <pthread.h>

class Ray;
class IPrimitive;
class KdTree;
class Camera;
class Sampler;
class ImageBuffer;
class Stats;
class StatsCollector;
class EnvSphere;

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
    static void* _run(void* arg);
    void run() const;
    
    bool trace(Ray& ray, bool firstHit = false) const;
    
    const KdTree* mKdTree;
    const Camera* mCamera;
    const EnvSphere* mEnv;
    ImageBuffer* const mImgBuffer;
    Sampler* const mSampler;
    Stats* mStats;
    const unsigned int mMaxDepth;
    pthread_t mThreadId;
    bool mIsCanceled;
};

#endif


