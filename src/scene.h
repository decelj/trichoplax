#ifndef __SCENE_H__
#define __SCENE_H__

#include <string>
#include <vector>
#include <forward_list>

#include "kdtree.h"

class Camera;
class Sampler;
class ImageBuffer;
class Mesh;
class ILight;
class Ray;
class EnvSphere;

class Scene
{
private:
    typedef std::vector<ILight*> LightVector;
    typedef std::forward_list<Mesh*> MeshList;

public:
    typedef LightVector::const_iterator ConstLightIter;
    typedef LightVector::iterator       LightIter;

    struct RenderSettings
    {
        RenderSettings();

        uint32_t maxDepth;
        uint32_t GISamples;
        float bias;
    };

    void prepareForRendering();
    void render(const std::string& filename);
    void setCamera(Camera* cam) { mCam = cam; }
    Mesh& allocateMesh(uint32_t numberOfVerticies);
    void addLight(ILight* lgt) { mLights.push_back(lgt); }
    void setMaxDepth(uint32_t depth) { mSettings.maxDepth = depth; }
    void setNumGISamples(uint32_t numSamples) { mSettings.GISamples = numSamples; }
    void setBias(float bias) { mSettings.bias = bias; }
    void setImageSize(uint32_t width, uint32_t height);
    void setEnvSphereImage(const std::string& file);
    void setShadowRays(uint32_t num);

    bool hasCamera() const { return mCam != nullptr; }

    const RenderSettings& renderSettings() const { return mSettings; }
    
    ConstLightIter lightsBegin() const { return mLights.begin(); }
    ConstLightIter lightsEnd() const { return mLights.end(); }

    LightIter lightsBegin() { return mLights.begin(); }
    LightIter lightsEnd() { return mLights.end(); }
    
    static Scene& instance();
    static void create();
    static void destroy();

private:
    explicit Scene();
    ~Scene();

    void createBuffer();

    Camera*                 mCam;
    Sampler*                mSampler;
    ImageBuffer*            mImgBuffer;
    KdTree*                 mKdTree;
    EnvSphere*              mEnvSphere;
    LightVector             mLights;
    RenderSettings          mSettings;
    MeshList                mMeshes;
    
    static Scene* sInstance;
};

#endif

