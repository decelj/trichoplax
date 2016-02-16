#include "noise.h"
#include "halton_generator.h"
#include "sampler_utils.h"

#include <random>

unsigned Noise::sGISequeces = 128;

Noise::Noise()
    : mDevice()
    , mGenerator(mDevice())
    , mGISamples()
{
}

Noise::~Noise()
{
}

void Noise::initGISamples(unsigned numberOfSamples)
{
    HaltonGenerator generator(2, 3);
    for (unsigned i = 0; i < sGISequeces; ++i)
    {
        mGISamples[i].clear();
        mGISamples[i].reserve(numberOfSamples);

        for (unsigned j = 0; j < numberOfSamples; ++j)
        {
            glm::vec2 sample = generator.generateSample(i * sGISequeces + j);
            mGISamples[i].emplace_back(cosineSampleHemisphere(sample));
        }
    }
}

