#ifndef __SAMPLER_H__
#define __SMAPLER_H__

#include <stdio.h>
#include <pthread.h>

struct Sample;

class Sampler
{
public:
    static const int sSamplesPerPixel;
    static const int sSamplesPerAxis;
    
    explicit Sampler(const int width, const int height);

    bool getSample(Sample* s);

private:
    explicit Sampler() { } // Don't use default constructor

    int mWidth, mHeight;
    int mCurrentX, mCurrentY;
    int mCurrentSample;
    
    pthread_mutex_t mLock;
};

#endif


