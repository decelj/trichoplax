#ifndef __STATS_H__
#define __STATS_H__

#include "ray.h"
#include <cstdint>

class Stats
{
public:
    explicit Stats();
    
    void incrementRayCount(const Ray::TYPE v) { ++rayCounts[v]; }
    void accumulate(const Stats& other);

    uint64_t rayCounts[Ray::TYPE_COUNT];
    uint64_t boxTests;
    uint64_t primitiveTests;
private:
    Stats(const Stats&) = delete;
    Stats& operator=(const Stats&) = delete;
};

#endif
