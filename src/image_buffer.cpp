#include <iostream>
#include <FreeImage.h>

#include "image_buffer.h"
#include "sampler.h"
#include "sample.h"
#include "common.h"
#include "debug.h"

ImageBuffer::ImageBuffer(const int width, const int height)
    : mWidth(width),
      mHeight(height)
{
    pthread_mutex_init(&mBufferLock, NULL);
    //mPixels = (float*) malloc(sizeof(float) * mWidth * mHeight * 4);
    mPixels = new float[mWidth*mHeight*4];
    bzero(mPixels, sizeof(float) * width * height * 4);
}

ImageBuffer::~ImageBuffer()
{
    delete [] mPixels;
    pthread_mutex_destroy(&mBufferLock);
}

void ImageBuffer::commit(const Sample& sample, const glm::vec3& color)
{
    pthread_mutex_lock(&mBufferLock);
    
    const unsigned int offset = (static_cast<int>(sample.y) * mWidth + static_cast<int>(sample.x)) * 4;
    
    mPixels[offset+3] += 1.0f;
    mPixels[offset+2] += color.r;
    mPixels[offset+1] += color.g;
    mPixels[offset] += color.b;
    
    pthread_mutex_unlock(&mBufferLock);
}

void ImageBuffer::write(const std::string& filename)
{
    FreeImage_Initialise();
    FIBITMAP *img = FreeImage_Allocate(mWidth, mHeight, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
    
    const int bytespp = FreeImage_GetLine(img) / mWidth;
    std::cout << "Saving image: " << filename << std::endl;
    
    for (unsigned int y = 0; y < mHeight; ++y) {
        BYTE *bits = FreeImage_GetScanLine(img, y);
        for (unsigned int x = 0; x < mWidth; ++x) {
            int offset = (y * mWidth + x) * 4;
            //printf("%f, %f, %f, %f\n", mPixels[offset+2], mPixels[offset+1], mPixels[offset], mPixels[offset+3]);
            bits[FI_RGBA_ALPHA] = MIN((mPixels[offset+3] / Sampler::sSamplesPerPixel), 1.0f) * 255;
            bits[FI_RGBA_RED] = MIN((mPixels[offset] / Sampler::sSamplesPerPixel), 1.0f) * 255;
            bits[FI_RGBA_GREEN] = MIN((mPixels[offset+1] / Sampler::sSamplesPerPixel), 1.0f) * 255;
            bits[FI_RGBA_BLUE] = MIN((mPixels[offset+2] / Sampler::sSamplesPerPixel), 1.0f) * 255;
            
            bits += bytespp;
        }
    }

    FreeImage_Save(FIF_PNG, img, filename.c_str(), 0);
    FreeImage_Unload(img);
    FreeImage_DeInitialise();
}
