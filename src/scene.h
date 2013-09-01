#ifndef __SCENE_H__
#define __SCENE_H__

#include <string>
#include <vector>
#include <assert.h>

#include "raytracer.h"

class Camera;
class Sampler;
class ImageBuffer;
class ILight;
class Ray;
class IPrimitive;

class Scene
{
public:
    void createBuffer(const int width, const int height);
    void render(const std::string& filename);
    void setCamera(const Camera* cam) { mCam = cam; }
    inline void addPrimitive(IPrimitive* prim) { mRaytracer->addPrimitive(prim); }
    inline void addLight(ILight* lgt) { mLights.push_back(lgt); }
    inline void setMaxDepth(int depth) const { mRaytracer->setMaxDepth(depth); }
    void setShadowRays(int num);
    
    inline const Camera* camera() const { return mCam; }
    inline Raytracer* raytracer() { return mRaytracer; }
    inline Sampler* sampler() { return mSampler; }
    inline ImageBuffer* imageBuffer() { return mImgBuffer; }
    
    inline bool traceShadow(Ray& r, float maxDistance) const { return mRaytracer->traceShadow(r, maxDistance); }
    inline bool traceAndShade(Ray& r, glm::vec3* result) const { return mRaytracer->traceAndShade(r, result); }
    
    inline std::vector<ILight*>::const_iterator lightsBegin() const { return mLights.begin(); }
    inline std::vector<ILight*>::const_iterator lightsEnd() const { return mLights.end(); }
    
    static Scene* instance();
    static void destroy();

private:
    explicit Scene() {}
    void setup();
    ~Scene();

    const Camera* mCam;
    Sampler* mSampler;
    ImageBuffer* mImgBuffer;
	Raytracer* mRaytracer;
    std::vector<ILight*> mLights;
    
    static Scene* mInstance;
};

#endif

