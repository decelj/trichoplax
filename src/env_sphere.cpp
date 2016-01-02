#include <iostream>

#include "env_sphere.h"
#include "ray.h"
#include "common.h"


EnvSphere::EnvSphere(const std::string& file)
    : mImg(NULL)
    , mWidth(0)
    , mHeight(0)
{
    FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
    format = FreeImage_GetFileType(file.c_str(), 0);
    if (format == FIF_UNKNOWN)
    {
        format = FreeImage_GetFIFFromFilename(file.c_str());
    }
    
    if (format != FIF_UNKNOWN && FreeImage_FIFSupportsReading(format))
    {
        mImg = FreeImage_Load(FIF_TIFF, file.c_str(), TIFF_DEFAULT);
    }
    
    if (mImg == NULL)
    {
        std::cerr << "Error loading env sphere: " << file << std::endl;
    }
    else
    {
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
    float u = atan2f(ray.dir().z, ray.dir().x) / (2.f * static_cast<float>(M_PI)) + 0.5f;
    float v = acosf(-ray.dir().y) / static_cast<float>(M_PI);
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

    result.a = 1.f;
}
