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
#include "mesh.h"

class Triangle;


namespace
{
void joinThreads(std::vector<std::unique_ptr<Raytracer> >& tracers, bool kill=false)
{
    if (kill)
    {
        for (auto& tracer : tracers)
            tracer->cancel();
    }
    
    for (auto& tracer : tracers)
        tracer->join();
}
} // annonymous namespace


Scene::RenderSettings::RenderSettings()
    : maxDepth(1)
    , GISamples(50)
    , bias(0.001f)
    , lightRadius(0.f)
{
}


// Global static pointer for singleton
Scene* Scene::sInstance = nullptr;

Scene::Scene()
    : mCam(nullptr)
    , mSampler(nullptr)
    , mImgBuffer(nullptr)
    , mKdTree(new KdTree)
    , mEnvSphere(nullptr)
    , mLights()
    , mSettings()
    , mMeshes()
{
}

Scene::~Scene()
{
    delete mCam;
    mCam = nullptr;
    
    delete mSampler;
    mSampler = nullptr;
    
    delete mImgBuffer;
    mImgBuffer = nullptr;
    
    delete mEnvSphere;
    mEnvSphere = nullptr;

    delete mKdTree;
    mKdTree = nullptr;

    for (ILight* light : mLights)
    {
        delete light;
    }
    mLights.clear();

    for (Mesh* mesh : mMeshes)
    {
        delete mesh;
    }
    mMeshes.clear();
}

Scene& Scene::instance()
{
    return *sInstance;
}

void Scene::create()
{
    TP_ASSERT(sInstance == nullptr);
    sInstance = new Scene;
}

void Scene::destroy()
{
    delete sInstance;
    sInstance = nullptr;
}

void Scene::createBuffer()
{
    delete mSampler;
    delete mImgBuffer;
    
    mSampler = new Sampler(mCam->width(), mCam->height());
    mImgBuffer = new ImageBuffer(mCam->width(), mCam->height());
}

Mesh& Scene::allocateMesh(uint32_t numberOfVerticies)
{
    mMeshes.emplace_front(new Mesh(numberOfVerticies));
    return *mMeshes.front();
}

void Scene::setImageSize(uint32_t width, uint32_t height)
{
    mCam->setWidthHeight(width, height);
}

void Scene::setEnvSphereImage(const std::string& file)
{
    if (mEnvSphere != nullptr)
    {
        std::cerr << "Warning: overriding existing env sphere" << std::endl;
        delete mEnvSphere;
    }
    mEnvSphere = new EnvSphere(file);
}

void Scene::prepareForRendering()
{
    size_t triangleID = 0;
    for (const Mesh* mesh : mMeshes)
    {
        for (Triangle* triangle : *mesh)
        {
            triangle->SetID(triangleID++);
            mKdTree->addPrimitive(triangle);
        }
    }

    for (ILight* light : mLights)
    {
        light->setBias(mSettings.bias);
    }
}

void Scene::render(const std::string& filename, uint32_t maxThreads)
{
    TP_ASSERT(mCam != nullptr);
    createBuffer();
    mKdTree->build();
    
    Timer t;
    StatsCollector collector;
    t.start();
    
    uint32_t numCpus = (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
    if (numCpus < 1)
    {
        std::stringstream ss;
        ss << "Error getting number of CPUs: " << strerror(numCpus);
        throw std::runtime_error(ss.str());
    }

    numCpus = std::min(numCpus, maxThreads);
    std::cout << "Using " << numCpus << " CPUs" << std::endl;
    
    std::vector<std::unique_ptr<Raytracer> > tracers;
    tracers.reserve(numCpus);
    
    for (uint32_t i = 0; i < numCpus; ++i)
    {
        tracers.emplace_back(
            std::make_unique<Raytracer>(*mKdTree, *mCam, mEnvSphere, mSampler, mImgBuffer, mSettings.maxDepth));

        std::unique_ptr<Raytracer>& tracer = tracers.back();
        tracer->registerStatsCollector(collector);
        if (!tracer->start())
        {
            joinThreads(tracers, true /*kill*/);
            throw std::runtime_error("Error creating threads!");
        }
    }
    
    joinThreads(tracers);
    mImgBuffer->write(filename);
    
    // Print stats
    std::cout << std::endl;
    collector.print();
    std::cout << std::endl;
    std::cout << "Render time: " << t.elapsedToString(t.elapsed()) << std::endl;
}

void Scene::setShadowRays(uint32_t num)
{
    for (ILight* light : mLights)
        light->setShadowRays(num);
}
