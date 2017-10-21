#include <string.h>
#include <iostream>

#include "stats.h"

Stats::Stats()
    : boxTests(0)
    , primitiveTests(0)
{
    for (unsigned i = 0; i < Ray::TYPE_COUNT; ++i)
    {
        rayCounts[i] = 0;
    }
}

void Stats::accumulate(const Stats& other)
{
    for (unsigned i = 0; i < Ray::TYPE_COUNT; ++i)
    {
        rayCounts[i] += other.rayCounts[i];
    }

    boxTests += other.boxTests;
    primitiveTests += other.primitiveTests;
}
