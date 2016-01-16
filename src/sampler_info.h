#ifndef sampler_info_h
#define sampler_info_h

class StratifiedSamplerInfo2D
{
public:
    StratifiedSamplerInfo2D();
    StratifiedSamplerInfo2D(unsigned numberOfSamples);
    StratifiedSamplerInfo2D(const StratifiedSamplerInfo2D& other) = default;

    StratifiedSamplerInfo2D& operator=(const StratifiedSamplerInfo2D& rhs) = default;

    void setNumberOfSamples(unsigned numberOfSamples);
    unsigned selectXStrata(unsigned sampleNumber) const { return sampleNumber % mXStratas; }
    unsigned selectYStrata(unsigned sampleNumber) const { return sampleNumber % mYStratas; }

    unsigned xStratas() const           { return mXStratas; }
    unsigned yStratas() const           { return mYStratas; }
    unsigned numberOfSamples() const    { return mTotalNumberOfSamples; }

protected:
    unsigned mTotalNumberOfSamples;
    unsigned mXStratas;
    unsigned mYStratas;
};


class FixedDomainSamplerInfo2D
    : public StratifiedSamplerInfo2D
{
public:
    FixedDomainSamplerInfo2D(float xDomainMax, float yDomainMax);
    FixedDomainSamplerInfo2D(unsigned numberOfSamples, float xDomainMax, float yDomainMax);
    FixedDomainSamplerInfo2D(const FixedDomainSamplerInfo2D& other) = default;

    FixedDomainSamplerInfo2D& operator=(const FixedDomainSamplerInfo2D& rhs) = default;

    void setNumberOfSamples(unsigned numberOfSamples);

    float computeXValue(unsigned sampleNumber, float normalizedSample) const;
    float computeYValue(unsigned sampleNumber, float normalizedSample) const;

private:
    float mXDomainMax;
    float mYDomainMax;
    float mXStrataSize;
    float mYStrataSize;
};


inline float FixedDomainSamplerInfo2D::computeXValue(unsigned sampleNumber, float normalizedSample) const
{
    return normalizedSample * mXStrataSize + (float)selectXStrata(sampleNumber) * mXStrataSize;
}

inline float FixedDomainSamplerInfo2D::computeYValue(unsigned sampleNumber, float normalizedSample) const
{
    return normalizedSample * mYStrataSize + (float)selectYStrata(sampleNumber) * mYStrataSize;
}

#endif /* sampler_info_h */
