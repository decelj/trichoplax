#ifndef isamples_sequence_h
#define isamples_sequence_h

#include <glm/glm.hpp>

class ISamplesGenerator
{
public:
    ISamplesGenerator();
    virtual ~ISamplesGenerator();

    virtual glm::vec2 generateSample(unsigned sampleNumber) = 0;
};

#endif /* isamples_sequence_h */
