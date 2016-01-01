#include <iostream>
#include <algorithm>
#include <FreeImage.h>
#include <assert.h>

#include "image_buffer.h"
#include "sampler.h"
#include "sample.h"
#include "common.h"
#include "debug.h"
#include "scoped_lock.h"

ImageBuffer::ImageBuffer(const unsigned width, const unsigned height)
    : mPixels(NULL)
    , mWidth(width)
    , mHeight(height)
{
    mPixels = new float[mWidth*mHeight*4];
    bzero(mPixels, sizeof(mPixels));
}

ImageBuffer::~ImageBuffer()
{
    delete [] mPixels;
}

void ImageBuffer::commit(const Sample& sample, const glm::vec4& color)
{
    const unsigned int offset = 
        (static_cast<int>(sample.y) * mWidth + static_cast<int>(sample.x)) * 4;
    
    // Note: No thread locking needed here since each thread executes and commits
    // a unique pixel as controlled via sampler in Sampler::buildSamplePacket
    assert(color.a <= 1.f);
    assert(mPixels[offset] == 0.f &&
           mPixels[offset+2] == 0.f &&
           mPixels[offset+3] == 0.f);
    mPixels[offset+3] = color.a;
    mPixels[offset+2] = color.r;
    mPixels[offset+1] = color.g;
    mPixels[offset] = color.b;
}

void ImageBuffer::write(const std::string& filename) const
{
    FIBITMAP *img = FreeImage_Allocate(mWidth, mHeight, 32, FI_RGBA_RED_MASK,
                                       FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
    
    const int bytespp = FreeImage_GetLine(img) / mWidth;
    std::cout << "Saving image: " << filename << std::endl;
    
    for (unsigned int y = 0; y < mHeight; ++y) {
        BYTE *bits = FreeImage_GetScanLine(img, y);
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
