//
//  stats.h
//  trichoplax
//
//  Created by Justin DeCell on 12/21/13.
//  Copyright (c) 2013 com.jdecell. All rights reserved.
//

#ifndef __STATS_H__
#define __STATS_H__

#include "ray.h"

class Stats
{
public:
    explicit Stats();
    
    inline void increment(const Ray::TYPE v) { ++mValues[v]; }
    
    friend class StatsCollector;
protected:
    unsigned int mValues[Ray::TYPE_COUNT];
    
private:
    Stats(Stats& other) {} // No copy
};

#endif
