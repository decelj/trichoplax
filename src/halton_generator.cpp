#include "halton_generator.h"
#include "LDsequence_utils.h"

#include <glm/glm.hpp>


HaltonGenerator::HaltonGenerator(unsigned xBase, unsigned yBase)
    : ISamplesGenerator()
    , mXBase(xBase)
    , mYBase(yBase)
{
}

HaltonGenerator::~HaltonGenerator()
{
}

glm::vec2 HaltonGenerator::generateSample(unsigned sampleNumber)
{
    return glm::vec2(RadicalInverse(sampleNumber, mXBase),
                     RadicalInverse(sampleNumber, mYBase));
}
