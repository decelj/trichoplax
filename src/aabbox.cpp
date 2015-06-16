#include <algorithm>

#include "aabbox.h"
#include "ray.h"

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

bool AABBox::intersect(const Ray& ray) const
{
    const glm::vec3 llToOrigin = mBounds[0] - ray.origin();
    const glm::vec3 urToOrigin = mBounds[1] - ray.origin();
    const glm::vec3 nearTs = llToOrigin * ray.inverseDir();
    const glm::vec3 farTs = urToOrigin * ray.inverseDir();
    float tMin = ray.minT();
    float tMax = ray.maxT();
    
    for (int i = 0; i < 3; ++i) {
        float nearT = nearTs[i];
        float farT = farTs[i];
        if (nearT > farT) {
            float tmp = nearT;
            nearT = farT;
            farT = tmp;
        }
        
        tMin = std::max(nearT, tMin);
        tMax = std::min(farT, tMax);
        if (tMin > tMax)
            return false;
    }
    
    return true;
}
