#ifndef __SAMPLE_H__
#define __SAMPLE_H__

#include <vector>

struct Sample {
    Sample(float _x, float _y) { x = _x; y = _y; }
    Sample() {}
    float x, y;
};

class SamplePacket {
public:
    explicit SamplePacket() { mSamples.clear(); mCurrSample = 0; }
    void addSample(float x, float y) {
        mSamples.push_back(Sample(x, y));
    }
    
    bool nextSample(Sample& s) {
        if (mCurrSample >= mSamples.size()) return false;
        s = mSamples[mCurrSample];
        ++mCurrSample;
        return true;
    }
    
private:
    std::vector<Sample> mSamples;
    int mCurrSample;
};

#endif

