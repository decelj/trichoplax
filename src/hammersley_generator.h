#ifndef hammersley_generator_h
#define hammersley_generator_h

#include "isamples_generator.h"
#include <glm/glm.hpp>

class HammersleyGeneratorBase2
    : public ISamplesGenerator
{
public:
    HammersleyGeneratorBase2(unsigned totalNumberOfSamples, unsigned scramble);
    ~HammersleyGeneratorBase2();

    glm::vec2 generateSample(unsigned sampleNumber) override;

private:
    unsigned mTotalNumberOfSamples;
    unsigned mScramble;
};

#endif /* hammersley_generator_h */
