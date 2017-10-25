#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

#include <glm/glm.hpp>
#include <pthread.h>

#include "stats.h"
#include "mailboxer.h"
#include "noise.h"
#include "kdtree.h"

class Ray;
class Camera;
class Sampler;
class ImageBuffer;
class Stats;
class StatsCollector;
class EnvSphere;


class Raytracer
{
public:
	explicit Raytracer(const KdTree& tree, const Camera& cam, const EnvSphere* env,
                       Sampler* const sampler, ImageBuffer* const imgBuffer,
                       const unsigned int maxDepth);
	~Raytracer();
    
    void registerStatsCollector(StatsCollector& c) const;
    bool start();
    bool join() const;
    void cancel();
    
    bool traceAndShade(Ray& ray, glm::vec4& result) const;
    inline bool traceShadow(Ray& ray) const
    {
        return trace(ray, true);
    }
    
    Noise& getNoiseGenerator() const { return mNoiseGen; }
    unsigned maxDepth() const { return mMaxDepth; }
	
private:
    // Not copyable
    Raytracer(const Raytracer&) = delete;
    Raytracer& operator=(const Raytracer&) = delete;
    
    static void* _run(void* arg);
    void run() const;
    
    bool trace(Ray& ray, bool visibilityTest) const;

    mutable Noise                   mNoiseGen;
    mutable Mailboxer               mMailboxes;
    const KdTree&                   mKdTree;
    mutable KdTree::TraversalBuffer mTraversalStack;
    const Camera&                   mCamera;
    const EnvSphere*                mEnv;
    ImageBuffer* const              mImgBuffer;
    Sampler* const                  mSampler;

    mutable Stats                   mStats;
    const unsigned int              mMaxDepth;
    pthread_t                       mThreadId;
    bool                            mIsCanceled;
};

#endif


