#ifndef __trichoplax__assimp_parser__
#define __trichoplax__assimp_parser__

#include <string>
#include <vector>
#include <memory>

#include "iparser.h"
#include "material.h"

class Scene;
class TransformStack;
struct aiScene;
struct aiNode;
struct aiLight;
struct aiCamera;


class AssimpParser : public IParser
{
public:
    AssimpParser() : mScene(NULL), mAiScene(NULL), mMaterials(0),
        mCameraIdx(0) // TODO allow user to select camera
    { }
    virtual ~AssimpParser();
    
    virtual std::string parse(const std::string& file, Scene* const scene);
    
private:
    void loadMaterials();
    void loadLight(const aiLight* light, const TransformStack* tStack);
    void loadCamera(const aiCamera* cam, const TransformStack* tStack);
    void loadScene(const aiNode* node, TransformStack* tStack);
    
    Scene* mScene;
    const aiScene* mAiScene;
    std::vector<std::unique_ptr<Material> > mMaterials;
    unsigned mCameraIdx;
};

#endif /* defined(__trichoplax__assimp_parser__) */
