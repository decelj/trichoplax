#pragma once

#include <string>
#include "iparser.h"

class Scene;

namespace fbxsdk
{
    class FbxScene;
    class FbxManager;
    class FbxNode;
}

class FBXImporter : public IParser
{
public:
    FBXImporter();
    ~FBXImporter();

    std::string parse(const std::string& file, Scene& scene) override;

private:
    fbxsdk::FbxManager* mFBXManager;
    fbxsdk::FbxScene*   mScene;
};
