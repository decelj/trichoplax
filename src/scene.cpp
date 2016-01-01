#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <sstream>

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

namespace {
void joinThreads(std::vector<Raytracer*>& tracers, bool kill=false)
{
    if (kill) {
        for (auto it = tracers.begin(); it != tracers.end(); ++it)
            (*it)->cancel();
    }
    
    for (auto it = tracers.begin(); it != tracers.end(); ++it)
        (*it)->join();
}
    
void cleanupThreads(std::vector<Raytracer*>& tracers)
{
    for (auto it = tracers.begin(); it != tracers.end(); ++it)
        delete *it;
    tracers.clear();
}
} // annonymous namespace


Scene::RenderSettings::RenderSettings()
    : maxDepth(1)
    , GISamples(0)
{
}


// Global static pointer for singleton
Scene* Scene::sInstance = NULL;

Scene::Scene()
    : mCam(NULL)
    , mSampler(NULL)
    , mImgBuffer(NULL)
    , mKdTree(new KdTree)
    , mEnvSphere(NULL)
    , mLights()
    , mSettings()
{
}

Scene::~Scene()
{
    delete mCam;
    mCam = NULL;
    
    delete mSampler;
    mSampler = NULL;
    
    delete mImgBuffer;
    mImgBuffer = NULL;
    
    delete mEnvSphere;
    mEnvSphere = NULL;
    
    for (auto it = mLights.begin(); it != mLights.end(); ++it)
        delete *it;
    mLights.clear();
}

Scene& Scene::instance()
{
    return *sInstance;
}

void Scene::create()
{
    TP_ASSERT(sInstance == NULL);
    sInstance = new Scene;
}

void Scene::destroy()
{
    delete sInstance;
    sInstance = NULL;
}

void Scene::createBuffer()
{
    delete mSampler;
    delete mImgBuffer;
    
    mSampler = new Sampler(mCam->width(), mCam->height());
    mImgBuffer = new ImageBuffer(mCam->width(), mCam->height());
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
    TP_ASSERT(mCam != NULL);
    createBuffer();
    mKdTree->build();
    
    Timer t;
    StatsCollector collector;
    t.start();
    
    unsigned int numCpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (numCpus < 1) {
        std::stringstream ss;
        ss << "Error getting number of CPUs: " << strerror(numCpus);
        throw std::runtime_error(ss.str());
    }
    
    std::cout << "Using " << numCpus << " CPUs" << std::endl;
    
    std::vector<Raytracer*> tracers;
    tracers.reserve(numCpus);
    
    for (unsigned int i = 0; i < numCpus; ++i) {
        Raytracer* tracer = new Raytracer(mKdTree, mCam, mEnvSphere, mSampler,
                                          mImgBuffer, mSettings.maxDepth);
        tracers.emplace_back(tracer);
        tracer->registerStatsCollector(&collector);
        if (!tracer->start()) {
            joinThreads(tracers, true /*kill*/);
            cleanupThreads(tracers);
            throw std::runtime_error("Error creating threads!");
        }
    }
    
    joinThreads(tracers);
    mImgBuffer->write(filename);
    
    // Print stats
    collector.print();
    std::cout << "Render time: " << t.elapsed() << " seconds" << std::endl;
    
    // Must cleanup threads after we've printed the collected stats since each
    // thread object owns it's own stats block.
    cleanupThreads(tracers);
}

void Scene::setShadowRays(int num)
{
    for (auto it = mLights.begin(); it != mLights.end(); ++it)
        (*it)->setShadowRays(num);
}
