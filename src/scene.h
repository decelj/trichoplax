#ifndef __SCENE_H__
#define __SCENE_H__

#include <string>
#include <vector>
#include <assert.h>

#include "kdtree.h"

class Camera;
class Sampler;
class ImageBuffer;
class ILight;
class Ray;
class IPrimitive;
class EnvSphere;

class Scene
{
private:
    typedef std::vector<ILight*> LightVector;

public:
    typedef LightVector::const_iterator ConstLightIter;

    void render(const std::string& filename);
    void setCamera(Camera* cam) { mCam = cam; }
    inline void addPrimitive(IPrimitive* prim) { mKdTree->addPrimitive(prim); }
    inline void addLight(ILight* lgt) { mLights.push_back(lgt); }
    inline void setMaxDepth(unsigned int depth) { mMaxTraceDepth = depth; }
    void setEnvSphereImage(const std::string& file);
    void setShadowRays(int num);
    
    ConstLightIter lightsBegin() const { return mLights.begin(); }
    ConstLightIter lightsEnd() const { return mLights.end(); }
    
    static Scene& instance();
    static void create();
    static void destroy();

private:
    explicit Scene() {}
    void setup();
    void createBuffer();
    ~Scene();

    Camera* mCam;
    Sampler* mSampler;
    ImageBuffer* mImgBuffer;
    KdTree* mKdTree;
    EnvSphere* mEnvSphere;
    LightVector mLights;
    unsigned int mMaxTraceDepth;
    
    static Scene* mInstance;
};

#endif

