/*
 *  abbox.cpp
 *  raytracer
 *
 *  Created by Justin DeCell on 5/6/13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

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

float AABBox::split(const short axis) const
{
    return ((mBounds[1][axis] - mBounds[0][axis]) / 2.0) + mBounds[0][axis];
}

bool AABBox::intersect(const Ray& ray) const
{
    const glm::vec3* invDir = ray.inverseDir();
    unsigned short signX = invDir->x < 0.0f ? 1 : 0;
    unsigned short signY = invDir->y < 0.0f ? 1 : 0;
    
    float tMin = (mBounds[signX].x - ray.origin()->x) * invDir->x;
    float tMax = (mBounds[1-signX].x - ray.origin()->x) * invDir->x;
    float tyMin = (mBounds[signY].y - ray.origin()->y) * invDir->y;
    float tyMax = (mBounds[1-signY].y - ray.origin()->y) * invDir->y;
    
    if ((tMin > tyMax) || (tyMin > tMax))
        return false;
    
    if (tyMin > tMin)
        tMin = tyMin;
    
    if (tyMax > tMax)
        tMax = tyMax;
    
    unsigned short signZ = invDir->z < 0.0f ? 1 : 0;
    float tzMin = (mBounds[signZ].z - ray.origin()->z) * invDir->z;
    float tzMax = (mBounds[1-signZ].z - ray.origin()->z) * invDir->z;
    
    if ((tMin > tzMax) || (tzMin > tMax))
        return false;
    
    return true;
}
