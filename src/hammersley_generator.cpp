#include "hammersley_generator.h"
#include "LDsequence_utils.h"

HammersleyGeneratorBase2::HammersleyGeneratorBase2(unsigned totalNumberOfSamples, unsigned scramble)
    : ISamplesGenerator()
    , mTotalNumberOfSamples(totalNumberOfSamples)
    , mScramble(scramble)
{
}

HammersleyGeneratorBase2::~HammersleyGeneratorBase2()
{
}

glm::vec2 HammersleyGeneratorBase2::generateSample(unsigned sampleNumber)
{
    return glm::vec2((float)sampleNumber / (float)mTotalNumberOfSamples,
                     VanDerCorputBase2(sampleNumber, mScramble));
}
