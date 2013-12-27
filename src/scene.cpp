#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>

#include "scene.h"
#include "image_buffer.h"
#include "sampler.h"
#include "camera.h"
#include "raytracer.h"
#include "ilight.h"
#include "common.h"
#include "timer.h"
#include "stats_collector.h"
#include "env_sphere.h"

// Global static pointer for singleton
Scene* Scene::mInstance = NULL;

void Scene::setup()
{
    mCam = NULL;
    mSampler = NULL;
    mImgBuffer = NULL;
    mEnvSphere = NULL;
	mKdTree = new KdTree();
    mLights.clear();
    mTracers.clear();
    mInstance = this;
}

Scene::~Scene()
{
    if (mCam != NULL)
        delete mCam;
    if (mSampler != NULL)
        delete mSampler;
    if (mImgBuffer != NULL)
        delete mImgBuffer;
    if (mEnvSphere != NULL)
        delete mEnvSphere;
    
    for (auto it = mLights.begin(); it != mLights.end(); ++it)
        delete *it;
    mLights.clear();
    
    for (auto it = mTracers.begin(); it != mTracers.end(); ++it)
        delete *it;
    mTracers.clear();
}

Scene* Scene::instance()
{
    if (mInstance == NULL) {
        mInstance = new Scene;
        mInstance->setup();
    }
    return mInstance;
}

void Scene::destroy()
{
    if (mInstance != NULL) {
        delete mInstance;
        mInstance = NULL;
    }
}

void Scene::createBuffer(const int width, const int height)
{
    mSampler = new Sampler(width, height);
    mImgBuffer = new ImageBuffer(width, height);
}

void Scene::setEnvSphereImage(const std::string& file)
{
    if (mEnvSphere != NULL) {
        std::cerr << "Warning: overriding existing env sphere" << std::endl;
        delete mEnvSphere;
    }
    mEnvSphere = new EnvSphere(file);
}

void Scene::render(const std::string& filename)
{
    assert(mCam != NULL);
    mKdTree->build();
    
    Timer t;
    StatsCollector collector;
    t.start();
    
    unsigned int numCpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (numCpus < 1) {
        std::cout << "Error getting number of CPUs: " << strerror(numCpus)
                  << std::endl;
        exit(-1);
    }
    std::cout << "Using " << numCpus << " CPUs" << std::endl;
    
    for (unsigned int i = 0; i < numCpus; ++i) {
        Raytracer* tracer = new Raytracer(mKdTree, mCam, mEnvSphere, mSampler,
                                          mImgBuffer, mMaxTraceDepth);
        mTracers.push_back(tracer);
        tracer->registerStatsCollector(&collector);
        if (!tracer->start()) {
            cleanupThreads(true);
            return;
        }
    }
    
    cleanupThreads();
    
    mImgBuffer->write(filename);
    collector.print();
    std::cout << "Render time: " << t.elapsed() << " seconds" << std::endl;
}

void Scene::cleanupThreads(bool force)
{
    if (force) {
        for (auto it = mTracers.begin(); it != mTracers.end(); ++it)
            (*it)->cancel();
    }
    
    for (auto it = mTracers.begin(); it != mTracers.end(); ++it)
        (*it)->join();
}

void Scene::setShadowRays(int num)
{
    for (auto it = mLights.begin(); it != mLights.end(); ++it)
        (*it)->setShadowRays(num);
}
