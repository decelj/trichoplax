#include <string.h>
#include <iostream>

#include "stats.h"

Stats::Stats()
{
    for (unsigned i = 0; i < Ray::TYPE_COUNT; ++i)
    {
        mValues[i] = 0;
    }
}
