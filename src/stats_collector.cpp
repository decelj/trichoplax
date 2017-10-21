#include <iostream>
#include <iomanip>
#include <stdint.h>

#include "stats_collector.h"
#include "ray.h"
#include "stats.h"


namespace
{
    const char* RayTypeToName(Ray::TYPE type)
    {
#define CASE(_T, _S) case _T: return _S; break
        switch (type)
        {
            CASE(Ray::PRIMARY, "Primary rays");
            CASE(Ray::REFLECTED, "Reflection rays");
            CASE(Ray::REFRACTED, "Refraction rays");
            CASE(Ray::GI, "GI rays");
            CASE(Ray::SHADOW, "Shadow rays");
            default: break;
        }
#undef CASE

        return "Undefined";
    }
}


StatsCollector::StatsCollector()
{
    mStats.clear();
}

void StatsCollector::print() const
{
    Stats allThreadStats;
    for (const Stats* stats : mStats)
    {
        allThreadStats.accumulate(*stats);
    }

    uint64_t totalRaysCast = 0;
    std::cout << "Ray Type Stats:" << std::endl;
    for (unsigned i = 0; i < Ray::TYPE_COUNT; ++i)
    {
        std::cout << "  " << std::left << std::setw(20)
            << RayTypeToName(static_cast<Ray::TYPE>(i))
            << allThreadStats.rayCounts[i] << std::endl;

        totalRaysCast += allThreadStats.rayCounts[i];
    }

    std::cout << "\nTraversal Stats:" << std::endl;
    std::cout << std::left << std::setw(22) << "  Box Tests:" << allThreadStats.boxTests
        << " (" << (double)allThreadStats.boxTests / (double)totalRaysCast << " per ray)" << std::endl;
    std::cout << std::left << std::setw(22) << "  Primitive Tests: " << allThreadStats.primitiveTests
        << " (" << (double)allThreadStats.primitiveTests / (double)totalRaysCast << " per ray)" << std::endl;
}

uint64_t StatsCollector::totalRaysCast() const
{
    uint64_t count = 0;
    for (const Stats* stats : mStats)
    {
        for (unsigned i = 0; i < Ray::TYPE_COUNT; ++i)
        {
            count += stats->rayCounts[i];
        }
    }

    return count;
}
