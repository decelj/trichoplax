#ifndef noise_cpp
#define noise_cpp

#include <random>

class Noise
{
public:
    Noise();
    ~Noise();
    
    float generateNormalizedFloat();
    
private:
    Noise(const Noise&) = delete;
    Noise& operator=(const Noise&) = delete;
    
    std::random_device      mDevice;
    std::mt19937            mGenerator;
};

inline float Noise::generateNormalizedFloat()
{
    return std::generate_canonical<float, 24>(mGenerator);
}

#endif /* noise_cpp */
