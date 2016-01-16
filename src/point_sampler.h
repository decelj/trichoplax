#ifndef point_sampler_hpp
#define point_sampler_hpp

#include <glm/glm.hpp>
#include <limits>

#include "isampler.h"
#include "multi_sample_ray.h"

class Noise;

class PointSampler : public ISampler
{
public:
    PointSampler(const glm::vec3& direction);

    void generateSample(Noise& noise, MultiSampleRay& outRay) const override;

private:
    glm::vec3       mDirection;
};

inline void PointSampler::generateSample(Noise& /*noise*/, MultiSampleRay& outRay) const
{
    outRay.setDir(mDirection);
    outRay.setMaxDistance(std::numeric_limits<float>::max());
}

#endif /* point_sampler_hpp */
