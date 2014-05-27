#include <iostream>
#include <FreeImage.h>

#include "image_buffer.h"
#include "sampler.h"
#include "sample.h"
#include "common.h"
#include "debug.h"
#include "scoped_lock.h"

ImageBuffer::ImageBuffer(const unsigned short width, const unsigned short height)
    : mWidth(width),
      mHeight(height)
{
    pthread_mutex_init(&mBufferLock, NULL);
    mPixels = new float[mWidth*mHeight*4];
    bzero(mPixels, sizeof(mPixels));
}

ImageBuffer::~ImageBuffer()
{
    delete [] mPixels;
    pthread_mutex_destroy(&mBufferLock);
}

void ImageBuffer::commit(const Sample& sample, const glm::vec4& color)
{
    ScopedLock lock(&mBufferLock);
    
    const unsigned int offset = 
        (static_cast<int>(sample.y) * mWidth + static_cast<int>(sample.x)) * 4;
    
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
            bits[FI_RGBA_ALPHA] = MIN(mPixels[offset+3], 1.0f) * 255;
            bits[FI_RGBA_RED] = MIN(mPixels[offset+2], 1.0f) * 255;
            bits[FI_RGBA_GREEN] = MIN(mPixels[offset+1], 1.0f) * 255;
            bits[FI_RGBA_BLUE] = MIN(mPixels[offset], 1.0f) * 255;
            
            bits += bytespp;
        }
    }

    FreeImage_Save(FIF_PNG, img, filename.c_str(), 0);
    FreeImage_Unload(img);
}
