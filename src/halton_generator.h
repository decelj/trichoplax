#ifndef halton_generator_h
#define halton_generator_h

#include "isamples_generator.h"
#include <glm/glm.hpp>

class HaltonGenerator
    : public ISamplesGenerator
{
public:
    HaltonGenerator(unsigned xBase, unsigned yBase);
    ~HaltonGenerator();

    glm::vec2 generateSample(unsigned sampleNumber) override;

private:
    unsigned mXBase;
    unsigned mYBase;
};

#endif /* halton_generator_h */
