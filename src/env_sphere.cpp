#include <iostream>

#include "env_sphere.h"
#include "ray.h"
#include "common.h"

namespace
{

inline void sphericalCoordinates(const glm::vec3& v, float& r, float& theta, float& phi)
{
    r = glm::length(v);
    theta = acosf(v.z / r);
    phi = atan2f(v.y, v.x);
}
} // anonymous namespace


EnvSphere::EnvSphere(const std::string& file)
    : mImg(NULL)
    , mWidth(0)
    , mHeight(0)
{
    FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
    format = FreeImage_GetFileType(file.c_str(), 0);
    if (format == FIF_UNKNOWN)
        format = FreeImage_GetFIFFromFilename(file.c_str());
    
    if (format != FIF_UNKNOWN && FreeImage_FIFSupportsReading(format))
        mImg = FreeImage_Load(FIF_TIFF, file.c_str(), TIFF_DEFAULT);
    
    if (mImg == NULL) {
        std::cerr << "Error loading: " << file << std::endl;
    } else {
        mHeight = FreeImage_GetHeight(mImg);
        mWidth = FreeImage_GetWidth(mImg);
    }
}

EnvSphere::~EnvSphere()
{
    FreeImage_Unload(mImg);
    mImg = NULL;
}

void EnvSphere::sample(const Ray& ray, glm::vec4& result) const
{
    float r, theta, phi;
    sphericalCoordinates(ray.dir(), r, theta, phi);
    
    float u = (phi + static_cast<float>(M_PI)) / (static_cast<float>(M_PI) * 2.f);
    float v = theta / static_cast<float>(M_PI);
    TP_ASSERT(u <= 1.f && u >= 0.f);
    TP_ASSERT(v <= 1.f && v >= 0.f);
    
    unsigned x = static_cast<unsigned>((mWidth - 1) * u);
    unsigned y = static_cast<unsigned>((mHeight - 1) * v);
    RGBQUAD value;
    
    if (!FreeImage_GetPixelColor(mImg, x, y, &value)) {
        std::cerr << "Error getting pixel " << x << ", " << y << std::endl;
        result.r = 0.f;
        result.g = 0.f;
        result.b = 0.f;
    } else {
        result.r = gammaToLinear(value.rgbRed / 255.f);
        result.g = gammaToLinear(value.rgbGreen / 255.f);
        result.b = gammaToLinear(value.rgbBlue / 255.f);
    }
}
