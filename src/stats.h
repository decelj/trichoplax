#ifndef __STATS_H__
#define __STATS_H__

#include "ray.h"

class Stats
{
public:
    explicit Stats();
    
    inline void increment(const Ray::TYPE v) { ++mValues[v]; }
    
    friend class StatsCollector;
protected:
    size_t mValues[Ray::TYPE_COUNT];
    
private:
    Stats(Stats& other) {} // No copy
};

#endif
