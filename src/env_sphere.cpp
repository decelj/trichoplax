#include <iostream>
#include "FreeImage.h"

#include "env_sphere.h"
#include "ray.h"
#include "common.h"


EnvSphere::EnvSphere(const std::string& file)
    : mImg(nullptr)
    , mWidth(0)
    , mHeight(0)
{
    FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
    format = FreeImage_GetFileType(file.c_str(), 0);
    if (format == FIF_UNKNOWN)
    {
        format = FreeImage_GetFIFFromFilename(file.c_str());
    }

    FIBITMAP* fiImage = nullptr;
    if (format != FIF_UNKNOWN && FreeImage_FIFSupportsReading(format))
    {
        fiImage = FreeImage_Load(format, file.c_str());
    }
    
    if (fiImage == nullptr)
    {
        std::cerr << "Error loading env sphere: " << file << std::endl;
    }
    else
    {
        FreeImage_AdjustGamma(fiImage, 2.2f);

        mHeight = FreeImage_GetHeight(fiImage);
        mWidth = FreeImage_GetWidth(fiImage);

        mImg = new glm::vec3[mWidth * mHeight];
        for (unsigned y = 0; y < mHeight; ++y)
        {
            for (unsigned x = 0; x < mWidth; ++x)
            {
                RGBQUAD color;
                FreeImage_GetPixelColor(fiImage, x, y, &color);

                mImg[y * mWidth + x].r = color.rgbRed / 255.f;
                mImg[y * mWidth + x].g = color.rgbGreen / 255.f;
                mImg[y * mWidth + x].b = color.rgbBlue / 255.f;
            }
        }

        FreeImage_Unload(fiImage);
    }
}

EnvSphere::~EnvSphere()
{
    delete [] mImg;
    mImg = nullptr;
}

void EnvSphere::sample(const Ray& ray, glm::vec4& result) const
{
    float u = atan2f(ray.dir().z, ray.dir().x) / TWO_PI + 0.5f;
    float v = acosf(-ray.dir().y) * INV_PI;
    TP_ASSERT(u <= 1.f && u >= 0.f);
    TP_ASSERT(v <= 1.f && v >= 0.f);

    glm::vec2 coords((mWidth - 1) * u, (mHeight - 1) * v);
    coords = glm::max(glm::vec2(0.f), coords - 0.5f);

    glm::vec2 st = glm::fract(coords);
    glm::uvec2 centerCoord = glm::uvec2(coords);

    /* | b | c |
     * | a | d | */
    glm::vec3 a, b, c, d;
    a = getPixelClamped(centerCoord.x, centerCoord.y);
    b = getPixelClamped(centerCoord.x, centerCoord.y + 1);
    c = getPixelClamped(centerCoord.x + 1, centerCoord.y + 1);
    d = getPixelClamped(centerCoord.x + 1, centerCoord.y);

    glm::vec3 color;
    color = a * (1.f - st.x) * (1.f - st.y) +
        b * (1.f - st.x) * st.y +
        c * st.x * st.y +
        d * st.x * (1.f - st.y);

    result += glm::vec4(color, 1.f);
}
