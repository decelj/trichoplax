#ifndef __STATS_COLLECTOR_H__
#define __STATS_COLLECTOR_H__

#include <vector>

class Stats;

class StatsCollector
{
public:
    explicit StatsCollector();
    
    void addStats(Stats *s);
    void print() const;
    
private:
    StatsCollector(StatsCollector& other) {}
    
    std::vector<Stats*> mStats;
};

#endif
