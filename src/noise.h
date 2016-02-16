#ifndef noise_cpp
#define noise_cpp

#include <random>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>

class Noise
{
public:
    Noise();
    ~Noise();

    void initGISamples(unsigned numberOfSamples);

    unsigned generateGISequenceNumber();
    const glm::vec3& getGISample(unsigned sequenceNumber, unsigned sampleNumber) const;
    float generateNormalizedFloat();
private:
    typedef std::vector<glm::vec3> SamplesArray;

    Noise(const Noise&) = delete;
    Noise& operator=(const Noise&) = delete;
    
    std::random_device      mDevice;
    std::mt19937            mGenerator;
    SamplesArray            mGISamples[128];

    static unsigned         sGISequeces;
};

inline float Noise::generateNormalizedFloat()
{
    return std::generate_canonical<float, 32>(mGenerator);
}

inline unsigned Noise::generateGISequenceNumber()
{
    return std::min(sGISequeces - 1, (unsigned)(sGISequeces * generateNormalizedFloat()));
}

inline const glm::vec3& Noise::getGISample(unsigned sequenceNumber, unsigned sampleNumer) const
{
    return mGISamples[sequenceNumber][sampleNumer];
}

#endif /* noise_cpp */
