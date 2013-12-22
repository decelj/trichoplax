#include <iostream>

#include "raytracer.h"
#include "ray.h"
#include "iprimitive.h"
#include "material.h"
#include "hit.h"
#include "stats.h"
#include "stats_collector.h"
#include "kdtree.h"
#include "sampler.h"
#include "sample.h"
#include "image_buffer.h"
#include "camera.h"


Raytracer::Raytracer(const KdTree* tree, const Camera* cam, Sampler* sampler,
                     ImageBuffer* imgBuffer, const unsigned int maxDepth)
 :  mKdTree(tree),
    mCamera(cam),
    mImgBuffer(imgBuffer),
    mSampler(sampler),
    mStats(new Stats),
    mMaxDepth(5),
    mThreadId(0),
    mIsCanceled(false)
{
}

Raytracer::~Raytracer()
{
    delete mStats;
}

void Raytracer::registerStatsCollector(StatsCollector* c) const
{
    c->addStats(mStats);
}

bool Raytracer::start()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    int err = pthread_create(&mThreadId, &attr, _run, this);
    pthread_attr_destroy(&attr);
    if (err) {
        printf("Error creating new thread: %d\n", err);
        return false;
    }
    
    return true;
}

void* Raytracer::_run(void *arg) {
    Raytracer* tracer = static_cast<Raytracer*>(arg);
    tracer->run();
    
    pthread_exit(NULL);
    return NULL;
}

void Raytracer::run() const
{
    SamplePacket packet;
    Sample sample;
    while (!mIsCanceled && mSampler->buildSamplePacket(&packet)) {
        while (!mIsCanceled && packet.nextSample(sample)) {
            Ray primary(Ray::PRIMARY);
            mCamera->generateRay(sample, &primary);
            glm::vec4 result(0.f, 0.f, 0.f, 0.f);
            if(traceAndShade(primary, result))
                mImgBuffer->commit(sample, result);
        }
    }
}

bool Raytracer::join() const
{
    void* status;
    int err = pthread_join(mThreadId, &status);
    if (err) {
        printf("Error joining threads: %d\n", err);
        return false;
    }
    
    return true;
}

void Raytracer::cancel()
{
    mIsCanceled = true;
}

bool Raytracer::traceShadow(Ray& ray) const
{
    return trace(ray, true);
}

bool Raytracer::trace(Ray& ray, bool firstHit) const
{
    // Base case
    if (ray.depth() > mMaxDepth) return false;
    
    mStats->increment(ray.type());
    return mKdTree->trace(ray, firstHit);
}

bool Raytracer::traceAndShade(Ray& ray, glm::vec4& result) const
{
    if (trace(ray)) {
        ray.shade(this, result);
        return true;
    }
    
    return false;
}
