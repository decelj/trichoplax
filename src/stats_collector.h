#ifndef __STATS_COLLECTOR_H__
#define __STATS_COLLECTOR_H__

#include <vector>
#include <cstdint>

class Stats;

class StatsCollector
{
public:
    explicit StatsCollector();
    
    void addStats(const Stats* s) { mStats.emplace_back(s); }

    void print() const;
    uint64_t totalRaysCast() const;
    
private:
    StatsCollector(const StatsCollector&) = delete;
    StatsCollector& operator=(const StatsCollector&) = delete;
    
    std::vector<const Stats*> mStats;
};

#endif
