#include "sampler_info.h"

#include <cmath>

StratifiedSamplerInfo2D::StratifiedSamplerInfo2D()
    : mTotalNumberOfSamples(0)
    , mXStratas(0)
    , mYStratas(0)
{
    setNumberOfSamples(mTotalNumberOfSamples);
}

StratifiedSamplerInfo2D::StratifiedSamplerInfo2D(unsigned numberOfSamples)
    : mTotalNumberOfSamples(numberOfSamples)
    , mXStratas(0)
    , mYStratas(0)
{
    setNumberOfSamples(mTotalNumberOfSamples);
}

void StratifiedSamplerInfo2D::setNumberOfSamples(unsigned numberOfSamples)
{
    mXStratas = (unsigned)std::ceilf(std::log2f((float)numberOfSamples));
    mYStratas = (unsigned)std::floorf((float)numberOfSamples / mXStratas);
    mTotalNumberOfSamples = numberOfSamples;
}


FixedDomainSamplerInfo2D::FixedDomainSamplerInfo2D(float xDomainMax, float yDomainMax)
    : StratifiedSamplerInfo2D()
    , mXDomainMax(xDomainMax)
    , mYDomainMax(yDomainMax)
    , mXStrataSize(0.f)
    , mYStrataSize(0.f)
{
}

FixedDomainSamplerInfo2D::FixedDomainSamplerInfo2D(
    unsigned numberOfSamples, float xDomainMax, float yDomainMax)
    : StratifiedSamplerInfo2D(numberOfSamples)
    , mXDomainMax(xDomainMax)
    , mYDomainMax(yDomainMax)
    , mXStrataSize(mXDomainMax / (float)mXStratas)
    , mYStrataSize(mYDomainMax / (float)mYStratas)
{
}

void FixedDomainSamplerInfo2D::setNumberOfSamples(unsigned numberOfSamples)
{
    StratifiedSamplerInfo2D::setNumberOfSamples(numberOfSamples);

    mXStrataSize = mXDomainMax / (float)mXStratas;
    mYStrataSize = mYDomainMax / (float)mYStratas;
}
