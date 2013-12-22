//
//  stats.cpp
//  trichoplax
//
//  Created by Justin DeCell on 12/21/13.
//  Copyright (c) 2013 com.jdecell. All rights reserved.
//

#include <string.h>
#include <iostream>

#include "stats.h"

Stats::Stats()
{
    memset(mValues, 0, sizeof(unsigned int) * Ray::TYPE_COUNT);
}
