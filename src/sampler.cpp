#include <math.h>
#include <iostream>

#include "sampler.h"
#include "sample.h"

const unsigned Sampler::sSamplesPerPixel = 16;
const unsigned Sampler::sSamplesPerAxis = sqrtf(Sampler::sSamplesPerPixel);
const float Sampler::sSubpixelStep = 1.0f / (sSamplesPerAxis + 1);

Sampler::Sampler (const unsigned short width, const unsigned short height)
:   mWidth(width),
    mHeight(height),
    mPixelIdx(0)
{
}

bool Sampler::buildSamplePacket(SamplePacket& packet)
{
    unsigned pixelId = mPixelIdx.fetch_add(1, std::memory_order_relaxed);
    
    float pixelX = static_cast<unsigned>(pixelId % mWidth);
    float pixelY = static_cast<unsigned>(pixelId / mWidth);
    if (pixelX == 0.f && static_cast<unsigned>(pixelY) % 10 == 0)
        std::cout << "scanline " << static_cast<unsigned>(pixelY) << std::endl;
    
    packet.clear();
    for (unsigned i = 0; i < sSamplesPerPixel; ++i) {
        float x = pixelX + (((i % sSamplesPerAxis) + 1.0f) * sSubpixelStep);
        float y = pixelY + ((floorf(i / sSamplesPerAxis) + 1.0f) * sSubpixelStep);
        packet.addSample(x, y);
    }
    
    return pixelY < mHeight;
}

