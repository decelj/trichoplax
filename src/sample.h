#ifndef __SAMPLE_H__
#define __SAMPLE_H__

#include <vector>

struct Sample {
    Sample(float _x, float _y) : x(_x), y(_y) { }
    Sample() : x(0), y(0) { }
    float x, y;
};

class SamplePacket {
public:
    explicit SamplePacket() : mCurrSample(0)
    { mSamples.clear(); }
    
    inline void addSample(float x, float y) {
        mSamples.push_back(Sample(x, y));
    }
    
    bool nextSample(Sample& s) {
        if (mCurrSample >= mSamples.size()) return false;
        s = mSamples[mCurrSample];
        ++mCurrSample;
        return true;
    }
    
    inline void clear() {
        mSamples.clear();
        mCurrSample = 0;
    }
    
private:
    std::vector<Sample> mSamples;
    unsigned short mCurrSample;
};

#endif

