#ifndef __SAMPLER_H__
#define __SAMPLER_H__

#include <atomic>

class SamplePacket;

class Sampler
{
public:
    static const unsigned sSamplesPerPixel;
    static const unsigned sSamplesPerAxis;
    static const float sSubpixelStep;
    
    explicit Sampler(unsigned width, unsigned height);
    Sampler(const Sampler&) = delete;
    Sampler operator=(const Sampler&) = delete;

    bool buildSamplePacket(SamplePacket& packet);

private:
    const unsigned mWidth, mHeight;
    std::atomic_uint mPixelIdx;
};

#endif


