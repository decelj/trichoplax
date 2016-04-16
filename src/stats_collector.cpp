#include <iostream>
#include <iomanip>

#include "stats_collector.h"
#include "ray.h"
#include "stats.h"

StatsCollector::StatsCollector()
{
    mStats.clear();
}

void StatsCollector::print() const {
    std::cout << "Ray Type Stats:" << std::endl;

    for (int i = 0; i < Ray::TYPE_COUNT; ++i) {
        size_t sum = 0;
        for (auto it = mStats.begin(); it != mStats.end(); ++it)
            sum += (*it)->mValues[i];
        
        switch (i) {
            case Ray::PRIMARY:
                std::cout << std::left << std::setw(20) <<
                    "  Primary rays: " << sum << std::endl;
                break;
            case Ray::REFLECTED:
                std::cout << std::left << std::setw(20) <<
                    "  Reflected rays: " << sum << std::endl;
                break;
            case Ray::REFRACTED:
                std::cout << std::left << std::setw(20) <<
                    "  Refracted rays: " << sum << std::endl;
                break;
            case Ray::SHADOW:
                std::cout << std::left << std::setw(20) <<
                    "  Shadow rays: " << sum << std::endl;
                break;
            case Ray::GI:
                std::cout << std::left << std::setw(20) <<
                    "  GI rays: " << sum << std::endl;
            default:
                break;
        }
    }
}

unsigned long long StatsCollector::totalRaysCast() const
{
    unsigned long long count = 0;
    for (auto it = mStats.begin(); it != mStats.end(); ++it)
    {
        for (unsigned i = 0; i < Ray::TYPE_COUNT; ++i)
        {
            count += (*it)->mValues[i];
        }
    }

    return count;
}
