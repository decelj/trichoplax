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
#include "env_sphere.h"


Raytracer::Raytracer(const KdTree* tree, const Camera* cam, const EnvSphere* env,
                     Sampler* const sampler, ImageBuffer* const imgBuffer,
                     const unsigned int maxDepth)
 :  mKdTree(tree),
    mCamera(cam),
    mEnv(env),
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
        std::cerr << "Error creating new thread: " << err << std::endl;
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
        glm::vec4 packetResult(0.f, 0.f, 0.f, 0.f);
        while (!mIsCanceled && packet.nextSample(sample)) {
            Ray primary(Ray::PRIMARY);
            mCamera->generateRay(sample, &primary);
            glm::vec4 rayColor(0.f, 0.f, 0.f, 0.f);
            traceAndShade(primary, rayColor);
            packetResult += rayColor;
        }
        mImgBuffer->commit(sample, packetResult / Sampler::sSamplesPerPixel);
    }
}

bool Raytracer::join() const
{
    void* status;
    int err = pthread_join(mThreadId, &status);
    if (err) {
        std::cerr << "Error joining threads: " << err << std::endl;
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
    
    if (mEnv != NULL && ray.type() != Ray::PRIMARY && ray.type() != Ray::SHADOW)
        mEnv->sample(ray, result);
    
    return false;
}
