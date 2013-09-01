#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include "scene.h"
#include "image_buffer.h"
#include "sampler.h"
#include "sample.h"
#include "camera.h"
#include "ray.h"
#include "ilight.h"
#include "common.h"

// Global static pointer for singleton
Scene* Scene::mInstance = NULL;

void *worker(void*)
{
    Scene* scene = Scene::instance();
    Sample s;
    while (scene->sampler()->getSample(&s)) {
        Ray primary;
        scene->camera()->generateRay(s, &primary);
		glm::vec3 result(0,0,0);
		if(scene->raytracer()->traceAndShade(primary, &result))
            scene->imageBuffer()->commit(s, result);
    }
    pthread_exit(NULL);
    return NULL;
}

void Scene::setup()
{
    mCam = NULL;
    mSampler = NULL;
    mImgBuffer = NULL;
	mRaytracer = new Raytracer();
    mLights.clear();
    mInstance = this;
}

Scene::~Scene()
{
    printf("Destroy scene\n");
    delete mRaytracer;

    if (mCam != NULL)
        delete mCam;
    if (mSampler != NULL)
        delete mSampler;
    if (mImgBuffer != NULL)
        delete mImgBuffer;
    
    std::vector<ILight*>::iterator it = mLights.begin();
    for (; it != mLights.end(); ++it)
        delete *it;
    mLights.clear();
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
    if (mInstance != NULL) delete mInstance;
}

void Scene::createBuffer(const int width, const int height)
{
    mSampler = new Sampler(width, height);
    mImgBuffer = new ImageBuffer(width, height);
}

void Scene::render(const std::string& filename)
{
    assert(mCam != NULL);
    mRaytracer->finalize();
/*    Sample s;
    while (mSampler->getSample(&s)) {
        Ray primary;
        mCam->generateRay(s, &primary);
		glm::vec3 result(0,0,0);
		if(mRaytracer->traceAndShade(primary, &result))
            mImgBuffer->commit(s, result);
    } */
    int err = 0;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    //err = pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);
    //if (err) printf("Error setting stacksize: %d\n", err);
    
    unsigned numCpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (numCpus < 1) {
        printf("Error getting number of CPUs: %s\n", strerror(numCpus));
        exit(-1);
    }
    printf("Using %i cpus\n", numCpus);
    
    std::vector<pthread_t> threads;
    for (int i = 0; i < numCpus; i++) {
        threads.push_back((pthread_t)0);
        err = pthread_create(&threads.back(), &attr, worker, NULL);
        if (err) {
            printf("Error creating new thread: %d\n", err);
            exit(-1);
        }
    }
    
    pthread_attr_destroy(&attr);
    void* status;
    for (std::vector<pthread_t>::iterator it = threads.begin(); it != threads.end(); ++it) {
        err = pthread_join(*it, &status);
        if (err) {
            printf("Error joining threads: %d\n", err);
            exit(-1);
        }
    }
    
    mImgBuffer->write(filename);
    mRaytracer->printStats();
    pthread_exit(NULL); // Needed?
}

void Scene::setShadowRays(int num)
{
    std::vector<ILight*>::iterator it = mLights.begin();
    for (; it != mLights.end(); ++it)
        (*it)->setShadowRays(num);
}
