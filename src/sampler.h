#ifndef __SAMPLER_H__
#define __SMAPLER_H__

#include <pthread.h>

class SamplePacket;

class Sampler
{
public:
    static const int sSamplesPerPixel;
    static const int sSamplesPerAxis;
    static const float sSubpixelStep;
    
    explicit Sampler(const int width, const int height);

    bool buildSamplePacket(SamplePacket* packet);

private:
    explicit Sampler() { } // Don't use default constructor

    int mWidth, mHeight;
    int mCurrentX, mCurrentY;
    
    pthread_mutex_t mLock;
};

#endif


