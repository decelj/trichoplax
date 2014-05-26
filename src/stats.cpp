#include <string.h>
#include <iostream>

#include "stats.h"

Stats::Stats()
{
    bzero(mValues, sizeof(unsigned int) * Ray::TYPE_COUNT);
}
