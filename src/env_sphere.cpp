#include <iostream>

#include "env_sphere.h"
#include "ray.h"
#include "common.h"

void sphericalCoordinates(const glm::vec3& v, float& r, float& theta, float& phi)
{
    r = glm::length(v);
    theta = acosf(v.z / r);
    phi = atan2f(v.y, v.x);
}

EnvSphere::EnvSphere(const std::string& file)
: mImg(NULL),
  mWidth(0),
  mHeight(0)
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
    
    float u = (phi + M_PI) / (M_PI * 2.f);
    float v = theta / M_PI;
    assert(u <= 1.f && u >= 0.f);
    assert(v <= 1.f && v >= 0.f);
    
    unsigned int x = (mWidth - 1) * u;
    unsigned int y = (mHeight - 1) * v;
    RGBQUAD value;
    
    if (!FreeImage_GetPixelColor(mImg, x, y, &value)) {
        std::cerr << "Error getting pixel " << x << ", " << y << std::endl;
        result.r = 0.f;
        result.g = 0.f;
        result.b = 0.f;
    } else {
        result.r = value.rgbRed / 255.f;
        result.g = value.rgbGreen / 255.f;
        result.b = value.rgbBlue / 255.f;
    }
}
