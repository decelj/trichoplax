#include <math.h>
#include <iostream>

#include "sampler.h"
#include "sample.h"

const int Sampler::sSamplesPerPixel = 16;
const int Sampler::sSamplesPerAxis = sqrtf(Sampler::sSamplesPerPixel);
const float Sampler::sSubpixelStep = 1.0f / (sSamplesPerAxis + 1);

Sampler::Sampler (const int width, const int height) 
:   mWidth(width), 
    mHeight(height), 
    mCurrentX(0), 
    mCurrentY(0) 
{
    pthread_mutex_init(&mLock, NULL);
}

bool Sampler::buildSamplePacket(SamplePacket *packet)
{
    pthread_mutex_lock(&mLock);
    ++mCurrentX;
    if (mCurrentX >= mWidth) {
        mCurrentX = 0;
        ++mCurrentY;
        if (mCurrentY % 10 == 0)
            std::cout << "scanline " << mCurrentY << std::endl;
    }
    
    // Copy the values to local variables so I can release the mutex
    int localX = mCurrentX;
    int localY = mCurrentY;
    pthread_mutex_unlock(&mLock);
    
    for (int i = 0; i < sSamplesPerPixel; ++i) {
        float x = localX + (((i % sSamplesPerAxis) + 1.0f) * sSubpixelStep);
        float y = localY + ((floorf(i / sSamplesPerAxis) + 1.0f) * sSubpixelStep);
        packet->addSample(x, y);
    }
    
    return localY < mHeight;
}

