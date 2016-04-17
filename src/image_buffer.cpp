#include <iostream>
#include <algorithm>
#include <FreeImage.h>

#include "image_buffer.h"
#include "sample.h"
#include "common.h"

ImageBuffer::ImageBuffer(unsigned width, unsigned height)
    : mPixels(nullptr)
    , mWidth(width)
    , mHeight(height)
{
    mPixels = new float[mWidth*mHeight*4];
    memset(mPixels, 0, sizeof(float) * mWidth * mHeight * 4);
}

ImageBuffer::~ImageBuffer()
{
    delete [] mPixels;
    mPixels = nullptr;
}

void ImageBuffer::commit(const Sample& sample, const glm::vec4& color)
{
    const unsigned int offset = 
        (static_cast<int>(sample.y) * mWidth + static_cast<int>(sample.x)) * 4;
    
    // Note: No thread locking needed here since each thread executes and commits
    // a unique pixel as controlled via sampler in Sampler::buildSamplePacket
    TP_ASSERT(color.a <= 1.f);
    TP_ASSERT(mPixels[offset] == 0.f &&
           mPixels[offset+2] == 0.f &&
           mPixels[offset+3] == 0.f);
    mPixels[offset+3] = color.a;
    mPixels[offset+2] = linearToGamma(color.r);
    mPixels[offset+1] = linearToGamma(color.g);
    mPixels[offset] = linearToGamma(color.b);
}

void ImageBuffer::write(const std::string& filename) const
{
    FIBITMAP *img = FreeImage_Allocate(mWidth, mHeight, 32, FI_RGBA_RED_MASK,
                                       FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
    
    const int bytespp = FreeImage_GetLine(img) / mWidth;
    std::cout << "Saving image: " << filename << std::endl;
    
    for (unsigned int y = 0; y < mHeight; ++y) {
        BYTE* bits = FreeImage_GetScanLine(img, y);
        for (unsigned int x = 0; x < mWidth; ++x) {
            const unsigned int offset = (y * mWidth + x) * 4;
            bits[FI_RGBA_ALPHA] = static_cast<unsigned char>(std::min(mPixels[offset+3], 1.0f) * 255);
            bits[FI_RGBA_RED] = static_cast<unsigned char>(std::min(mPixels[offset+2], 1.0f) * 255);
            bits[FI_RGBA_GREEN] = static_cast<unsigned char>(std::min(mPixels[offset+1], 1.0f) * 255);
            bits[FI_RGBA_BLUE] = static_cast<unsigned char>(std::min(mPixels[offset], 1.0f) * 255);
            
            bits += bytespp;
        }
    }

    FreeImage_Save(FIF_PNG, img, filename.c_str(), 0);
    FreeImage_Unload(img);
}
