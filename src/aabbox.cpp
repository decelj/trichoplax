#include <algorithm>

#include "aabbox.h"

short AABBox::longestAxis() const
{
    glm::vec3 result = mBounds[1] - mBounds[0];
    result[0] = fabs(result[0]);
    result[1] = fabs(result[1]);
    result[2] = fabs(result[2]);
    
    if (result[0] > result[1] && result[0] > result[2])
        return 0;
    else if (result[1] > result[0] && result[1] > result[2])
        return 1;
    
    return 2;
}
