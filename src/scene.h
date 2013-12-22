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
class Raytracer;

class Scene
{
public:
    void createBuffer(const int width, const int height);
    void render(const std::string& filename);
    void setCamera(Camera* cam) { mCam = cam; }
    inline void addPrimitive(IPrimitive* prim) { mKdTree->addPrimitive(prim); }
    inline void addLight(ILight* lgt) { mLights.push_back(lgt); }
    inline void setMaxDepth(unsigned int depth) { mMaxTraceDepth = depth; }
    void setShadowRays(int num);
    
    inline std::vector<ILight*>::const_iterator lightsBegin() const
    { return mLights.begin(); }
    inline std::vector<ILight*>::const_iterator lightsEnd() const
    { return mLights.end(); }
    
    static Scene* instance();
    static void destroy();

private:
    explicit Scene() {}
    void setup();
    void cleanupThreads(bool force=false);
    ~Scene();

    Camera* mCam;
    Sampler* mSampler;
    ImageBuffer* mImgBuffer;
    KdTree* mKdTree;
    std::vector<ILight*> mLights;
    std::vector<Raytracer*> mTracers;
    unsigned int mMaxTraceDepth;
    
    static Scene* mInstance;
};

#endif

