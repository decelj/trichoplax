#ifndef __SAMPLER_H__
#define __SAMPLER_H__

#include <pthread.h>

class SamplePacket;

class Sampler
{
public:
    static const unsigned short sSamplesPerPixel;
    static const unsigned short sSamplesPerAxis;
    static const float sSubpixelStep;
    
    explicit Sampler(const unsigned short width, const unsigned short height);

    bool buildSamplePacket(SamplePacket& packet);

private:
    explicit Sampler() { } // Don't use default constructor

    unsigned short mWidth, mHeight;
    unsigned short mCurrentX, mCurrentY;
    
    pthread_mutex_t mLock;
};

#endif


