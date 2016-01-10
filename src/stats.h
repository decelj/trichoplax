#ifndef __STATS_H__
#define __STATS_H__

#include "ray.h"

class Stats
{
public:
    friend class StatsCollector;

    explicit Stats();
    
    void increment(const Ray::TYPE v) { ++mValues[v]; }
    
private:
    Stats(Stats&) = delete;
    Stats& operator=(Stats&) = delete;

    unsigned long long mValues[Ray::TYPE_COUNT];
};

#endif
