#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <memory>

#include "assimp_parser.h"
#include "scene.h"
#include "material.h"
#include "transform_stack.h"
#include "triangle.h"
#include "point_light.h"
#include "direct_light.h"
#include "camera.h"

#undef AI_CONFIG_PP_SBP_REMOVE
#define AI_CONFIG_PP_SBP_REMOVE aiPrimitiveType_POINTS | aiPrimitiveType_LINES

namespace {
    
template<typename T>
inline glm::vec3 toVec3(const T& vec)
{
    return glm::vec3(vec[0], vec[1], vec[2]);
}

inline glm::mat4 toMat4(const aiMatrix4x4& mat)
{
    return glm::mat4(mat.a1, mat.b1, mat.c1, mat.d1,
                     mat.a2, mat.b2, mat.c2, mat.d2,
                     mat.a3, mat.b3, mat.c3, mat.d3,
                     mat.a4, mat.b4, mat.c4, mat.d4);
}

PointLight* parsePointLight(const aiLight* aiLight, const TransformStack* tStack)
{
    return new PointLight(tStack->transformPoint(toVec3(aiLight->mPosition)),
                          toVec3(aiLight->mColorDiffuse),
                          0.f /*radius*/, .1f /*bias*/,
                          aiLight->mAttenuationConstant,
                          aiLight->mAttenuationLinear,
                          aiLight->mAttenuationQuadratic);
}

DirectLight* parseDirectLight(const aiLight* aiLight, const TransformStack* tStack)
{
    return new DirectLight(tStack->transformNormal(toVec3(aiLight->mDirection)),
                           toVec3(aiLight->mColorDiffuse),
                           .1f /*bias*/);
}
} // anonymous namespace


AssimpParser::~AssimpParser()
{
    // trichoplax's primatives now own the material pointers, release them
    // from the unique_ptr.
    for (MaterialVector::iterator it = mMaterials.begin(); it != mMaterials.end(); ++it)
    {
        it->release();
    }
}

std::string AssimpParser::parse(const std::string& file, Scene& scene)
{
    mScene = &scene;
    
    Assimp::Importer importer;
    mAiScene = importer.ReadFile(file,
                                 aiProcess_Triangulate |
                                 aiProcess_JoinIdenticalVertices |
                                 aiProcess_SortByPType |
                                 aiProcess_FindDegenerates |
                                 aiProcess_RemoveRedundantMaterials |
                                 aiProcess_FindInvalidData);
    if (!mAiScene)
        throw std::runtime_error(importer.GetErrorString());
    
    if (!mAiScene->HasCameras())
        throw std::runtime_error("scene has no cameras!");
    
    if (!mAiScene->HasMeshes())
        throw std::runtime_error("scene has no geometry!");
    
    if (!mAiScene->HasMaterials())
        throw std::runtime_error("scene has no materials!");
    
    loadMaterials();
    
    TransformStack tStack;
    loadScene(mAiScene->mRootNode, &tStack);

    // TODO: Fix this
    return std::string("assimpImage.png");
}

void AssimpParser::loadLight(const aiLight* light, const TransformStack* tStack)
{
    switch (light->mType)
    {
        case aiLightSource_DIRECTIONAL:
            mScene->addLight(parseDirectLight(light, tStack));
            break;
            
        case aiLightSource_POINT:
            mScene->addLight(parsePointLight(light, tStack));
            break;
            
        default:
            std::cerr << "Invalid light type, ignoring!" << std::endl;
            break;
    }
}

void AssimpParser::loadCamera(const aiCamera* cam, const TransformStack* tStack)
{
    // Convert hfov in radians to vfov / 2 in radians
    float vfov = atanf(tanf(2.f * cam->mHorizontalFOV) / cam->mAspect);
    // convert from [pi/2, -pi/2] -> [pi, 0] then convert to degrees
    vfov = static_cast<float>((vfov + M_PI_2) / M_PI) * 180.f;
    vfov *= 2.f;
    
    mScene->setCamera(new Camera(vfov,
        tStack->transformPoint(toVec3(cam->mPosition)),
        tStack->transformNormal(toVec3(cam->mLookAt)),
        tStack->transformNormal(toVec3(cam->mUp)),
        800 /*width TODO: FIX ME!*/,
        cam->mAspect == 0. ? 600 : static_cast<unsigned>(800.f / cam->mAspect)));
}

void AssimpParser::loadScene(const aiNode* node, TransformStack* tStack)
{
    tStack->push();
    tStack->transform(toMat4(node->mTransformation));
    
    for (unsigned meshIdx = 0; meshIdx < node->mNumMeshes; ++meshIdx)
    {
        const aiMesh* mesh = mAiScene->mMeshes[node->mMeshes[meshIdx]];
        for (unsigned faceIdx = 0; faceIdx < mesh->mNumFaces; ++faceIdx)
        {
            const aiFace& face = mesh->mFaces[faceIdx];
            
            if (face.mNumIndices != 3)
            {
                std::cerr << "Invalid number of verticies (" << face.mNumIndices
                    << "), skipping face!" << std::endl;
                continue;
            }
            
            mScene->addPrimitive(
                new Triangle(tStack->transformPoint(toVec3(mesh->mVertices[face.mIndices[0]])),
                             tStack->transformPoint(toVec3(mesh->mVertices[face.mIndices[1]])),
                             tStack->transformPoint(toVec3(mesh->mVertices[face.mIndices[2]])),
                             mMaterials[mesh->mMaterialIndex].get()));
        }
    }
    
    for (unsigned i = 0; i < mAiScene->mNumLights; ++i)
    {
        if (mAiScene->mLights[i]->mName == node->mName)
        {
            loadLight(mAiScene->mLights[i], tStack);
        }
    }
    
    if (mAiScene->mCameras[mCameraIdx]->mName == node->mName)
        loadCamera(mAiScene->mCameras[mCameraIdx], tStack);
    
    for (unsigned cIdx = 0; cIdx < node->mNumChildren; ++cIdx)
        loadScene(node->mChildren[cIdx], tStack);
    
    tStack->pop();
}

void AssimpParser::loadMaterials()
{
    mMaterials.reserve(mAiScene->mNumMaterials);
    for (unsigned i = 0; i < mAiScene->mNumMaterials; ++i)
    {
        const aiMaterial* aiMaterial = mAiScene->mMaterials[i];
        std::unique_ptr<Material> material(new Material);
        aiColor3D value;
        float valueF;
        
        if (aiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, value) != AI_SUCCESS)
        {
            throw std::runtime_error("error reading material!");
        }
        material->setAmbient(gammaToLinear(toVec3(value)));
        
        if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, value) != AI_SUCCESS)
        {
            throw std::runtime_error("error reading material!");
        }
        material->setDiffuse(gammaToLinear(toVec3(value)));
        
        if (aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, value) != AI_SUCCESS)
        {
            throw std::runtime_error("error reading material!");
        }
        if (aiMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, valueF) != AI_SUCCESS)
        {
            valueF = 1.f; // TODO: this seems to error sometimes, not sure why...
        }
        material->setSpecular(gammaToLinear(toVec3(value)*valueF));
        
        if (aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, value) != AI_SUCCESS)
        {
            throw std::runtime_error("error reading material!");
        }
        material->setEmissive(gammaToLinear(toVec3(value)));
        
        if (aiMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, value) != AI_SUCCESS)
        {
            throw std::runtime_error("error reading material!");
        }
        material->setTransparency(toVec3(value));
        
        if (aiMaterial->Get(AI_MATKEY_REFRACTI, valueF) != AI_SUCCESS)
        {
            throw std::runtime_error("error reading material!");
        }
        material->setIor(valueF);
        
        if (aiMaterial->Get(AI_MATKEY_SHININESS, valueF) != AI_SUCCESS)
        {
            throw std::runtime_error("error reading material!");
        }
        material->setShininess(valueF);
        
        mMaterials.push_back(std::move(material));
    }
}
