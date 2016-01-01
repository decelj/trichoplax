#include <string.h>
#include <iostream>

#include "stats.h"

Stats::Stats()
{
    bzero(mValues, sizeof(size_t) * Ray::TYPE_COUNT);
}
