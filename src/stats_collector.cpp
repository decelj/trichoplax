#include <iostream>

#include "stats_collector.h"
#include "ray.h"
#include "stats.h"

StatsCollector::StatsCollector()
{
    mStats.clear();
}

void StatsCollector::addStats(Stats* s) {
    mStats.emplace_back(s);
}

void StatsCollector::print() const {
    for (int i = 0; i < Ray::TYPE_COUNT; ++i) {
        unsigned int sum = 0;
        for (auto it = mStats.begin(); it != mStats.end(); ++it)
            sum += (*it)->mValues[i];
        
        switch (i) {
            case Ray::PRIMARY:
                std::cout << "Primary rays: " << sum << std::endl;
                break;
            case Ray::REFLECTED:
                std::cout << "Reflected rays: " << sum << std::endl;
                break;
            case Ray::REFRACTED:
                std::cout << "Refracted rays: " << sum << std::endl;
                break;
            case Ray::SHADOW:
                std::cout << "Shadow rays: " << sum << std::endl;
                break;
            default:
                break;
        }
    }
}
