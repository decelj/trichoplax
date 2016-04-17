#ifndef __STATS_COLLECTOR_H__
#define __STATS_COLLECTOR_H__

#include <vector>

class Stats;

class StatsCollector
{
public:
    explicit StatsCollector();
    
    inline void addStats(const Stats* s)
    { mStats.emplace_back(s); }
    void print() const;
    unsigned long long totalRaysCast() const;
    
private:
    StatsCollector(const StatsCollector&) = delete;
    StatsCollector& operator=(const StatsCollector&) = delete;
    
    std::vector<const Stats*> mStats;
};

#endif
