#ifndef isampler_h
#define isampler_h

class MultiSampleRay;
class Noise;

class ISampler
{
public:
    ISampler();
    virtual ~ISampler();
    
    virtual void generateSample(Noise& noise, MultiSampleRay& outRay) const = 0;
};

#endif /* isampler_h */
