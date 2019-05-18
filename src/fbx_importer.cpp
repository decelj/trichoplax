#include "fbx_importer.h"

#define FBXSDK_DEFINE_NAMESPACE 1
#include <fbxsdk.h>

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <glm/glm.hpp>
#include <cmath>

#include "mesh.h"
#include "material.h"
#include "scene.h"
#include "ilight.h"
#include "direct_light.h"
#include "point_light.h"
#include "camera.h"

namespace
{
typedef std::unordered_map<const fbxsdk::FbxSurfaceMaterial*, Material*> FBXTPMaterialMap;

template<class T>
inline glm::vec3 toVec3(const T& v)
{
    return glm::vec3(v[0], v[1], v[2]);
}

template<class T>
inline glm::vec2 toVec2(const T& v)
{
    return glm::vec2(v[0], v[1]);
}

// Assumes aspect is aspectY / aspectX
inline double HFOV2VFOV(double fov, double aspectRatio)
{
    return 2.0 * std::atan(aspectRatio * std::tan(fov * FBXSDK_PI_DIV_180 * 0.5)) * FBXSDK_180_DIV_PI;
}

// Assumes aspect is aspectX / aspectY
inline double VFOV2HFOV(double fov, double aspectRatio)
{
    return 2.0 * std::atan(aspectRatio * std::tan(fov * FBXSDK_PI_DIV_180 * 0.5)) * FBXSDK_180_DIV_PI;
}

inline fbxsdk::FbxAMatrix getNodeTransform(fbxsdk::FbxNode& node, const fbxsdk::FbxTime& time = fbxsdk::FbxTime(0))
{
    FbxAMatrix matrixGeo;
    matrixGeo.SetIdentity();
    if (node.GetNodeAttribute())
    {
        const FbxVector4& lT = node.GetGeometricTranslation(FbxNode::eSourcePivot);
        const FbxVector4& lR = node.GetGeometricRotation(FbxNode::eSourcePivot);
        const FbxVector4& lS = node.GetGeometricScaling(FbxNode::eSourcePivot);
        matrixGeo.SetT(lT);
        matrixGeo.SetR(lR);
        matrixGeo.SetS(lS);
    }

    FbxAMatrix globalMatrix = node.EvaluateLocalTransform();
    return globalMatrix * matrixGeo;
}

void loadMesh(fbxsdk::FbxNode& node, Scene& scene, FBXTPMaterialMap& materialsMap, AABBox& sceneExtents)
{
    fbxsdk::FbxAMatrix nodeTransform = getNodeTransform(node);
    fbxsdk::FbxMesh* mesh = node.GetMesh();

    bool allAttributesByControlPoint = true;
    
    if (!mesh->GetElementMaterial())
    {
        std::cerr << "Mesh \"" << mesh->GetName()
            << "\" does not have a material assigned to it, skipping" << std::endl;
        return;
    }

    if (!mesh->GetElementNormalCount())
    {
        std::cerr << "Mesh \"" << mesh->GetName()
            << "\" does not have normals, skipping" << std::endl;
        return;
    }
    else
    {
        if (mesh->GetElementNormal(0)->GetMappingMode() != fbxsdk::FbxGeometryElement::eByControlPoint)
        {
            allAttributesByControlPoint = false;
        }
    }

    if (!mesh->GetElementUVCount())
    {
        std::cerr << "Mesh \"" << mesh->GetName()
            << "\" does not have UVs, skipping" << std::endl;
        return;
    }
    else
    {
        if (mesh->GetElementUV(0)->GetMappingMode() != fbxsdk::FbxGeometryElement::eByControlPoint)
        {
            allAttributesByControlPoint = false;
        }
    }

    Mesh& tpMesh = scene.allocateMesh(allAttributesByControlPoint ? mesh->GetControlPointsCount() : mesh->GetPolygonVertexCount());

    const fbxsdk::FbxGeometryElement::EMappingMode fbxMaterialMapping =
        mesh->GetElementMaterial()->GetMappingMode();

    fbxsdk::FbxLayerElementArrayTemplate<int>* fbxMaterialIndices;
    mesh->GetMaterialIndices(&fbxMaterialIndices);
    const int* fbxVertexIndices = mesh->GetPolygonVertices();
    const fbxsdk::FbxVector4* controlPoints = mesh->GetControlPoints();

    if (!sceneExtents.isValid())
    {
        const glm::vec3 point = toVec3(nodeTransform.MultT(controlPoints[0]));
        sceneExtents = AABBox(point, point);
    }

    if (allAttributesByControlPoint)
    {
        const fbxsdk::FbxGeometryElementNormal* fbxNormalElement = mesh->GetElementNormal(0);
        const fbxsdk::FbxGeometryElementUV* fbxUVElement = mesh->GetElementUV(0);

        for (int i = 0; i < mesh->GetControlPointsCount(); ++i)
        {
            const glm::vec3 position = toVec3(nodeTransform.MultT(controlPoints[i]));
            sceneExtents.encompass(position);

            int normalIndex = i;
            if (fbxNormalElement->GetReferenceMode() == fbxsdk::FbxLayerElement::eIndexToDirect)
            {
                normalIndex = fbxNormalElement->GetIndexArray().GetAt(i);
            }

            fbxsdk::FbxVector4 fbxNormal = fbxNormalElement->GetDirectArray().GetAt(normalIndex);
            fbxNormal = nodeTransform.MultR(fbxNormal);
            fbxNormal.Normalize();
            const glm::vec3 normal = toVec3(fbxNormal);

            int uvIndex = i;
            if (fbxUVElement->GetReferenceMode() == fbxsdk::FbxLayerElement::eIndexToDirect)
            {
                uvIndex = fbxUVElement->GetIndexArray().GetAt(i);
            }

            const glm::vec2 uv = toVec2(fbxUVElement->GetDirectArray().GetAt(uvIndex));
            tpMesh.addVertex(position, normal, uv);
        }

        for (int polygonIndex = 0; polygonIndex < mesh->GetPolygonCount(); ++polygonIndex)
        {
            TP_ASSERT(mesh->GetPolygonSize(polygonIndex) == 3);

            int fbxMaterialIndex = 0;
            if (fbxMaterialMapping == fbxsdk::FbxGeometryElement::eByPolygon)
            {
                fbxMaterialIndex = fbxMaterialIndices->GetAt(polygonIndex);
            }

            Material* tpMaterial = materialsMap[node.GetMaterial(fbxMaterialIndex)];
            TP_ASSERT(tpMaterial);

            const int polygonVertexIndex = mesh->GetPolygonVertexIndex(polygonIndex);
            tpMesh.addPrimitive(fbxVertexIndices[polygonVertexIndex],
                                fbxVertexIndices[polygonVertexIndex + 1],
                                fbxVertexIndices[polygonVertexIndex + 2],
                                tpMaterial);
        }
    }
    else
    {
        fbxsdk::FbxStringList UVNames;
        mesh->GetUVSetNames(UVNames);
        TP_ASSERT(UVNames.GetCount());
        const char* UVSetName = UVNames[0];

        for (int polygonIndex = 0; polygonIndex < mesh->GetPolygonCount(); ++polygonIndex)
        {
            TP_ASSERT(mesh->GetPolygonSize(polygonIndex) == 3);

            const int polygonVertexStartIndex = mesh->GetPolygonVertexIndex(polygonIndex);
            TP_ASSERT(polygonVertexStartIndex >= 0);

            const int polygonVertexCount = mesh->GetPolygonSize(polygonIndex);
            TP_ASSERT(polygonVertexCount == 3);

            for (int vertexIndex = 0; vertexIndex < polygonVertexCount; ++vertexIndex)
            {
                int controlPointIndex = fbxVertexIndices[polygonVertexStartIndex + vertexIndex];
                const glm::vec3 position = toVec3(nodeTransform.MultT(controlPoints[controlPointIndex]));
                sceneExtents.encompass(position);

                fbxsdk::FbxVector4 fbxNormal;
                mesh->GetPolygonVertexNormal(polygonIndex, vertexIndex, fbxNormal);
                fbxNormal = nodeTransform.MultR(fbxNormal);
                fbxNormal.Normalize();
                const glm::vec3 normal = toVec3(fbxNormal);

                fbxsdk::FbxVector2 fbxUV;
                bool vertexIsUnmapped;
                mesh->GetPolygonVertexUV(polygonIndex, vertexIndex, UVSetName, fbxUV, vertexIsUnmapped);
                const glm::vec2 uv = toVec2(fbxUV);
                TP_ASSERT(!vertexIsUnmapped);

                tpMesh.addVertex(position, normal, uv);
            }

            int fbxMaterialIndex = 0;
            if (fbxMaterialMapping == fbxsdk::FbxGeometryElement::eByPolygon)
            {
                fbxMaterialIndex = fbxMaterialIndices->GetAt(polygonIndex);
            }

            Material* tpMaterial = materialsMap[node.GetMaterial(fbxMaterialIndex)];
            TP_ASSERT(tpMaterial);

            unsigned tpVertexIndexStart = polygonIndex * 3;
            tpMesh.addPrimitive(tpVertexIndexStart,
                                tpVertexIndexStart + 1,
                                tpVertexIndexStart + 2,
                                tpMaterial);
        }
    }
}

// Get specific property value and connected texture if any.
// Value = Property value * Factor property value (if no factor property, multiply by 1).
glm::vec3 getVec3MaterialProperty(const fbxsdk::FbxSurfaceMaterial& material,
                               const char* propertyName,
                               const char* factorPropertyName)
{
    fbxsdk::FbxDouble3 result(0.0, 0.0, 0.0);
    const fbxsdk::FbxProperty& property = material.FindProperty(propertyName);
    const fbxsdk::FbxProperty& factorProperty = material.FindProperty(factorPropertyName);
    if (property.IsValid() && factorProperty.IsValid())
    {
        double factor = factorProperty.Get<fbxsdk::FbxDouble>();
        result = property.Get<fbxsdk::FbxDouble3>();
        result[0] *= factor;
        result[1] *= factor;
        result[2] *= factor;
    }

    /* TODO: Support textrues
    if (property.IsValid())
    {
        const int lTextureCount = property.GetSrcObjectCount<FbxFileTexture>();
        if (lTextureCount)
        {
            const FbxFileTexture* lTexture = property.GetSrcObject<FbxFileTexture>();
        }
    }
    */
    
    return glm::vec3(result[0], result[1], result[2]);
}

float getScalarMaterialProperty(const fbxsdk::FbxSurfaceMaterial& material,
                                const char* propertyName)
{
    float result = 0.f;

    const fbxsdk::FbxProperty& property = material.FindProperty(propertyName);
    if (property.IsValid())
    {
        result = static_cast<float>(property.Get<fbxsdk::FbxDouble>());
    }

    return result;
}

bool loadMaterials(fbxsdk::FbxNode& node, FBXTPMaterialMap& materialsMap)
{
    for (unsigned i = 0; i < (unsigned)node.GetMaterialCount(); ++i)
    {
        fbxsdk::FbxSurfaceMaterial* material = node.GetMaterial(i);
        if (materialsMap[material])
        {
            // Alreadly loaded material
            continue;
        }

        Material* tpMaterial = new Material;
        materialsMap[material] = tpMaterial;

        tpMaterial->setAmbient(getVec3MaterialProperty(*material,
                                                       fbxsdk::FbxSurfaceMaterial::sAmbient,
                                                       fbxsdk::FbxSurfaceMaterial::sAmbientFactor));
        tpMaterial->setDiffuse(getVec3MaterialProperty(*material,
                                                       fbxsdk::FbxSurfaceMaterial::sDiffuse,
                                                       fbxsdk::FbxSurfaceMaterial::sDiffuseFactor));

        // TODO: Figure out actual scale
        float shininess = getScalarMaterialProperty(*material, fbxsdk::FbxSurfaceMaterial::sShininess);
        shininess = glm::clamp(shininess, 0.f, 100.f) / 100.f;
        tpMaterial->setRoughness(1.f - shininess);

        glm::vec3 reflectivity = getVec3MaterialProperty(*material,
                                                         fbxsdk::FbxSurfaceMaterial::sReflection,
                                                         fbxsdk::FbxSurfaceMaterial::sReflectionFactor);
        tpMaterial->setReflectivity(luminance(reflectivity));

        tpMaterial->setTransparency(getVec3MaterialProperty(*material,
                                                            fbxsdk::FbxSurfaceMaterial::sTransparentColor,
                                                            fbxsdk::FbxSurfaceMaterial::sTransparencyFactor));
        // TODO: IOR
    }

    return node.GetMaterialCount() > 0;
}

void loadLight(fbxsdk::FbxNode& node, Scene& scene)
{
    const fbxsdk::FbxLight& light = *node.GetLight();
    ILight* tpLight = nullptr;

    glm::vec3 lightColor = toVec3(light.Color.Get());
    lightColor *= light.Intensity.Get(); // Maya normalization factor

    fbxsdk::FbxAMatrix matrix = getNodeTransform(node);
    fbxsdk::FbxVector4 fbxLightDir = matrix.MultR(fbxsdk::FbxVector4(0, 0, -1));
    fbxLightDir.Normalize();
    const glm::vec3 lightDir = toVec3(fbxLightDir);
    const glm::vec3 lightPos = toVec3(matrix.GetT());

    switch (light.LightType.Get())
    {
        case fbxsdk::FbxLight::eDirectional:
            tpLight = new DirectLight(lightDir, lightColor, 0.01f);
            break;

        case fbxsdk::FbxLight::ePoint:
            tpLight = new PointLight(lightPos,
                                     lightColor / (4.f * PI), // normalize, 4PI steradians in sphere
                                     1.f, // radius
                                     0.1f, // bias
                                     0.0f, // constant decay
                                     light.DecayType.Get() == fbxsdk::FbxLight::eLinear ? 1.f : 0.f, // Linear
                                     light.DecayType.Get() == fbxsdk::FbxLight::eQuadratic ? 1.f : 0.f); // Quad
            break;

        default:
            break;
    }

    if (tpLight)
    {
        scene.addLight(tpLight);
    }
}

void loadCamera(fbxsdk::FbxScene& fbxScene, fbxsdk::FbxNode& cameraNode, Scene& scene)
{
    const fbxsdk::FbxGlobalSettings& globalSettings = fbxScene.GetGlobalSettings();
    const fbxsdk::FbxString& currentCameraName = globalSettings.GetDefaultCamera();
    const fbxsdk::FbxString& cameraName = cameraNode.GetName();
    if (cameraName != currentCameraName)
    {
        return;
    }

    std::cout << "Loading camera: \"" << cameraName << "\"" << std::endl;

    fbxsdk::FbxCamera& fbxCamera = *(fbxsdk::FbxCamera*)cameraNode.GetNodeAttribute();
    TP_ASSERT(fbxCamera.ProjectionType.Get() == fbxsdk::FbxCamera::ePerspective);

    const fbxsdk::FbxAMatrix cameraMatrix = cameraNode.EvaluateGlobalTransform();
    const glm::vec3 eye = toVec3(fbxCamera.Position.Get());
    glm::vec3 up = toVec3(fbxCamera.UpVector.Get());

    glm::vec3 center;
    if (cameraNode.GetTarget())
    {
        center = toVec3(cameraNode.GetTarget()->EvaluateGlobalTransform().GetT());
    }
    else
    {
        // Get the direction
        fbxsdk::FbxAMatrix globalRotation;
        globalRotation.SetR(cameraMatrix.GetR());

        // Get the length
        fbxsdk::FbxVector4 interestPosition(fbxCamera.InterestPosition.Get());
        fbxsdk::FbxVector4 cameraGlobalPosition(cameraMatrix.GetT());
        double length = (interestPosition - cameraGlobalPosition).Length();

        // Set the center.
        // A camera with rotation = {0,0,0} points to the X direction. So create a
        // vector in the X direction, rotate that vector by the global rotation amount
        // and then position the center by scaling and translating the resulting vector
        fbxsdk::FbxVector4 rotationVector = fbxsdk::FbxVector4(1.0,0,0);
        center = toVec3(globalRotation.MultT(rotationVector));
        center *= length;
        center += eye;

        // Update the default up vector with the camera rotation.
        rotationVector = fbxsdk::FbxVector4(0,1.0,0);
        up = toVec3(globalRotation.MultT(rotationVector));
    }

    glm::vec3 forward = glm::normalize(center - eye);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    up = glm::normalize(glm::cross(right, forward));

    // Rotate the up vector with the roll value.
    float radians = static_cast<float>(fbxCamera.Roll.Get() * FBXSDK_PI_DIV_180);
    up = up * std::cosf(radians) + right * std::sinf(radians);

    //get the aspect ratio
    fbxsdk::FbxCamera::EAspectRatioMode camAspectRatioMode = fbxCamera.GetAspectRatioMode();
    double aspectX = fbxCamera.AspectWidth.Get();
    double aspectY = fbxCamera.AspectHeight.Get();
    double aspectRatio = 1.333333;
    switch(camAspectRatioMode)
    {
        case fbxsdk::FbxCamera::eWindowSize:
            aspectRatio = aspectX / aspectY;
            break;
        case fbxsdk::FbxCamera::eFixedRatio:
            aspectRatio = aspectX;
            break;
        case fbxsdk::FbxCamera::eFixedResolution:
            aspectRatio = aspectX / aspectY * fbxCamera.GetPixelRatio();
            break;
        case fbxsdk::FbxCamera::eFixedWidth:
            aspectRatio = fbxCamera.GetPixelRatio() / aspectY;
            break;
        case fbxsdk::FbxCamera::eFixedHeight:
            aspectRatio = fbxCamera.GetPixelRatio() * aspectX;
            break;
        default:
            break;

    }

    double filmHeight = fbxCamera.GetApertureHeight();
    double filmWidth = fbxCamera.GetApertureWidth() * fbxCamera.GetSqueezeRatio();
    double apertureRatio = filmHeight / filmWidth;

    //change the aspect ratio to Height : Width
    aspectRatio = 1.0 / aspectRatio;

    //revise the aspect ratio and aperture ratio
    switch(fbxCamera.GateFit.Get())
    {
        case fbxsdk::FbxCamera::eFitFill:
            if( apertureRatio > aspectRatio)  // the same as eHORIZONTAL_FIT
            {
                filmHeight = filmWidth * aspectRatio;
                fbxCamera.SetApertureHeight( filmHeight);
                apertureRatio = filmHeight / filmWidth;
            }
            else if( apertureRatio < aspectRatio) //the same as eVERTICAL_FIT
            {
                filmWidth = filmHeight / aspectRatio;
                fbxCamera.SetApertureWidth( filmWidth);
                apertureRatio = filmHeight / filmWidth;
            }
            break;
        case fbxsdk::FbxCamera::eFitVertical:
            filmWidth = filmHeight / aspectRatio;
            fbxCamera.SetApertureWidth( filmWidth);
            apertureRatio = filmHeight / filmWidth;
            break;
        case fbxsdk::FbxCamera::eFitHorizontal:
            filmHeight = filmWidth * aspectRatio;
            fbxCamera.SetApertureHeight( filmHeight);
            apertureRatio = filmHeight / filmWidth;
            break;
        case fbxsdk::FbxCamera::eFitStretch:
            aspectRatio = apertureRatio;
            break;
        case fbxsdk::FbxCamera::eFitOverscan:
            if( filmWidth > filmHeight)
            {
                filmHeight = filmWidth * aspectRatio;
            }
            else
            {
                filmWidth = filmHeight / aspectRatio;
            }
            apertureRatio = filmHeight / filmWidth;
            break;
        case fbxsdk::FbxCamera::eFitNone:
        default:
            break;
    }

    //change the aspect ratio to Width : Height
    aspectRatio = 1.0 / aspectRatio;

    double fieldOfViewX = 0.0;
    double fieldOfViewY = 0.0;
    if ( fbxCamera.GetApertureMode() == fbxsdk::FbxCamera::eVertical)
    {
        fieldOfViewY = fbxCamera.FieldOfView.Get();
        fieldOfViewX = VFOV2HFOV( fieldOfViewY, 1.0 / apertureRatio);
    }
    else if (fbxCamera.GetApertureMode() == fbxsdk::FbxCamera::eHorizontal)
    {
        fieldOfViewX = fbxCamera.FieldOfView.Get();
        fieldOfViewY = HFOV2VFOV(fieldOfViewX, apertureRatio);
    }
    else if (fbxCamera.GetApertureMode() == fbxsdk::FbxCamera::eFocalLength)
    {
        fieldOfViewX = fbxCamera.ComputeFieldOfView(fbxCamera.FocalLength.Get());
        fieldOfViewY = HFOV2VFOV(fieldOfViewX, apertureRatio);
    }
    else if (fbxCamera.GetApertureMode() == fbxsdk::FbxCamera::eHorizAndVert) {
        fieldOfViewX = fbxCamera.FieldOfViewX.Get();
        fieldOfViewY = fbxCamera.FieldOfViewY.Get();
    }

    scene.setCamera(new Camera((float)fieldOfViewY, eye, forward, up, 640, 480));
}

void loadScene(fbxsdk::FbxScene& fbxScene, fbxsdk::FbxNode& node, Scene& scene, FBXTPMaterialMap& materialsMap, AABBox& sceneExtents)
{
    if (node.GetNodeAttribute())
    {
        switch (node.GetNodeAttribute()->GetAttributeType())
        {
            case fbxsdk::FbxNodeAttribute::eMesh:
                if (loadMaterials(node, materialsMap))
                {
                    loadMesh(node, scene, materialsMap, sceneExtents);
                }
                break;

            case fbxsdk::FbxNodeAttribute::eLight:
                loadLight(node, scene);
                break;

            case fbxsdk::FbxNodeAttribute::eCamera:
                loadCamera(fbxScene, node, scene);
                break;

            default:
                break;
        }
    }

    for (int i = 0; i < node.GetChildCount(); ++i)
    {
        loadScene(fbxScene, *node.GetChild(i), scene, materialsMap, sceneExtents);
    }
}
} // anonymous namespace

FBXImporter::FBXImporter()
    : mFBXManager(nullptr)
    , mScene(nullptr)
{
}

FBXImporter::~FBXImporter()
{
    if (mFBXManager)
    {
        mFBXManager->Destroy();
    }

    mFBXManager = nullptr;
    mScene = nullptr;
}

std::string FBXImporter::parse(const std::string& file, Scene& scene)
{
    mFBXManager = fbxsdk::FbxManager::Create();
    fbxsdk::FbxIOSettings* ioSettings = fbxsdk::FbxIOSettings::Create(mFBXManager, IOSROOT);
    mFBXManager->SetIOSettings(ioSettings);

    ioSettings->SetBoolProp(IMP_FBX_ANIMATION, false);
    ioSettings->SetBoolProp(IMP_FBX_CHARACTER, false);
    ioSettings->SetBoolProp(IMP_CONSTRAINT, false);
    ioSettings->SetBoolProp(IMP_BONE, false);
    ioSettings->SetBoolProp(IMP_GEOMETRY, true);
    ioSettings->SetBoolProp(IMP_LIGHT, true);
    ioSettings->SetBoolProp(IMP_CAMERA, true);

    fbxsdk::FbxImporter* importer = fbxsdk::FbxImporter::Create(mFBXManager, "FBXImporter");
    if (!importer->Initialize(file.c_str()))
    {
        std::cerr << "Failed to load FBX file " << file << ":" << std::endl;
        std::cerr << importer->GetStatus().GetErrorString() << std::endl;

        importer->Destroy();
        throw std::runtime_error("Error loading scene");
    }

    mScene = fbxsdk::FbxScene::Create(mFBXManager, "scene");
    importer->Import(mScene);

    // Convert mesh, NURBS and patch into triangle mesh
    fbxsdk::FbxGeometryConverter geomConverter(mFBXManager);
    geomConverter.Triangulate(mScene, /*replace*/true);

    FBXTPMaterialMap materialsMap;
    AABBox sceneExtents;
    loadScene(*mScene, *mScene->GetRootNode(), scene, materialsMap, sceneExtents);

    if (!scene.hasCamera())
    {
        fbxsdk::FbxGlobalCameraSettings& globalCameraSettings = mScene->GlobalCameraSettings();
        // Check if we need to create the Producer cameras!
        if (globalCameraSettings.GetCameraProducerPerspective() == NULL &&
            globalCameraSettings.GetCameraProducerBottom() == NULL &&
            globalCameraSettings.GetCameraProducerTop() == NULL &&
            globalCameraSettings.GetCameraProducerFront() == NULL &&
            globalCameraSettings.GetCameraProducerBack() == NULL &&
            globalCameraSettings.GetCameraProducerRight() == NULL &&
            globalCameraSettings.GetCameraProducerLeft() == NULL)
        {
            globalCameraSettings.CreateProducerCameras();
        }
        

        const fbxsdk::FbxString& currentCameraName = mScene->GetGlobalSettings().GetDefaultCamera();
        fbxsdk::FbxCamera* fbxCamera = nullptr;
        if (currentCameraName.Compare(FBXSDK_CAMERA_PERSPECTIVE) == 0)
        {
            fbxCamera = globalCameraSettings.GetCameraProducerPerspective();
        }
        else if (currentCameraName.Compare(FBXSDK_CAMERA_BOTTOM) == 0)
        {
            fbxCamera = globalCameraSettings.GetCameraProducerBottom();
        }
        else if (currentCameraName.Compare(FBXSDK_CAMERA_TOP) == 0)
        {
            fbxCamera = globalCameraSettings.GetCameraProducerTop();
        }
        else if (currentCameraName.Compare(FBXSDK_CAMERA_FRONT) == 0)
        {
            fbxCamera = globalCameraSettings.GetCameraProducerFront();
        }
        else if (currentCameraName.Compare(FBXSDK_CAMERA_BACK) == 0)
        {
            fbxCamera = globalCameraSettings.GetCameraProducerBack();
        }
        else if (currentCameraName.Compare(FBXSDK_CAMERA_RIGHT) == 0)
        {
            fbxCamera = globalCameraSettings.GetCameraProducerRight();
        }
        else if (currentCameraName.Compare(FBXSDK_CAMERA_LEFT) == 0)
        {
            fbxCamera = globalCameraSettings.GetCameraProducerLeft();
        }

        if (fbxCamera)
        {
            loadCamera(*mScene, *fbxCamera->GetNode(), scene);
        }
        else
        {
            throw std::runtime_error("No camera found in scene!");
        }
    }

    importer->Destroy();
    importer = nullptr;

    float bias = std::min({sceneExtents.width(), sceneExtents.height(), sceneExtents.depth()}) / 100000.0f;
    for (Scene::LightIter it = scene.lightsBegin(); it != scene.lightsEnd(); ++it)
    {
        ILight* light = *it;
        light->setBias(bias);
    }

    return std::string("FBXImage.png");
}
