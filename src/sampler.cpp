#include <math.h>

#include "sampler.h"
#include "sample.h"

const int Sampler::sSamplesPerPixel = 16;
const int Sampler::sSamplesPerAxis = sqrtf(Sampler::sSamplesPerPixel);
const float Sampler::sSubpixelStep = 1.0f / (sSamplesPerAxis + 1);

Sampler::Sampler (const int width, const int height) 
:   mWidth(width), 
    mHeight(height), 
    mCurrentX(0), 
    mCurrentY(0), 
    mCurrentSample(0) 
{
    pthread_mutex_init(&mLock, NULL);
}

bool Sampler::getSample(Sample* s)
{
    pthread_mutex_lock(&mLock);
    if (mCurrentSample == sSamplesPerPixel) {
        ++mCurrentX;
        mCurrentSample = 0;
        if (mCurrentX >= mWidth) {
            mCurrentX = 0;
            ++mCurrentY;
            if (mCurrentY % 10 == 0)
                printf("scanline %i\n", mCurrentY);
        }
    }
    
    s->x = mCurrentX + (((mCurrentSample % sSamplesPerAxis) + 1.0f) * sSubpixelStep);
    s->y = mCurrentY + ((floorf(mCurrentSample / sSamplesPerAxis) + 1.0f) * sSubpixelStep);
    
    ++mCurrentSample;
    pthread_mutex_unlock(&mLock);
    
    return mCurrentY < mHeight;
}

