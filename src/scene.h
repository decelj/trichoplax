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

    struct RenderSettings
    {
        RenderSettings();

        unsigned maxDepth;
        unsigned GISamples;
    };

    void render(const std::string& filename);
    void setCamera(Camera* cam) { mCam = cam; }
    void addPrimitive(IPrimitive* prim) { mKdTree->addPrimitive(prim); }
    void addLight(ILight* lgt) { mLights.push_back(lgt); }
    void setMaxDepth(unsigned depth) { mSettings.maxDepth = depth; }
    void setNumGISamples(unsigned numSamples) { mSettings.GISamples = numSamples; }
    void setEnvSphereImage(const std::string& file);
    void setShadowRays(int num);

    const RenderSettings& renderSettings() const { return mSettings; }
    
    ConstLightIter lightsBegin() const { return mLights.begin(); }
    ConstLightIter lightsEnd() const { return mLights.end(); }
    
    static Scene& instance();
    static void create();
    static void destroy();

private:
    explicit Scene();
    ~Scene();

    void createBuffer();

    Camera* mCam;
    Sampler* mSampler;
    ImageBuffer* mImgBuffer;
    KdTree* mKdTree;
    EnvSphere* mEnvSphere;
    LightVector mLights;
    RenderSettings mSettings;
    
    static Scene* sInstance;
};

#endif

