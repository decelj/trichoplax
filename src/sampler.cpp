#include <math.h>
#include <iostream>

#include "sampler.h"
#include "sample.h"
#include "scoped_lock.h"

const unsigned short Sampler::sSamplesPerPixel = 16;
const unsigned short Sampler::sSamplesPerAxis = sqrtf(Sampler::sSamplesPerPixel);
const float Sampler::sSubpixelStep = 1.0f / (sSamplesPerAxis + 1);

Sampler::Sampler (const unsigned short width, const unsigned short height)
:   mWidth(width),
    mHeight(height),
    mCurrentX(0),
    mCurrentY(0)
{
    pthread_mutex_init(&mLock, NULL);
}

bool Sampler::buildSamplePacket(SamplePacket *packet)
{
    unsigned short localX, localY;
    {
        ScopedLock lock(&mLock);
        
        ++mCurrentX;
        if (mCurrentX >= mWidth) {
            mCurrentX = 0;
            ++mCurrentY;
            if (mCurrentY % 10 == 0)
                std::cout << "scanline " << mCurrentY << std::endl;
        }
        
        // Copy the values to local variables so I can release the mutex
        localX = mCurrentX;
        localY = mCurrentY;
    } // Unlock mutex
    
    packet->clear();
    for (unsigned short i = 0; i < sSamplesPerPixel; ++i) {
        float x = localX + (((i % sSamplesPerAxis) + 1.0f) * sSubpixelStep);
        float y = localY + ((floorf(i / sSamplesPerAxis) + 1.0f) * sSubpixelStep);
        packet->addSample(x, y);
    }
    
    return localY < mHeight;
}

