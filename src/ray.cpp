#include "ray.h"
#include <stdio.h>

void Ray::transform(const glm::mat4& m)
{
    
    // Apply translation to origin
    mOrigin = glm::vec3(m * glm::vec4(mOrigin, 1.0));
    
    // Apply upper 3x3 rotation to direction
    // dot(row, dir)
    /*mDir[0] = mDir[0] * m[0][0] + mOrigin[1] * m[0][1] + mOrigin[2] * m[0][2];
    mDir[1] = mDir[1] * m[1][0] + mOrigin[1] * m[1][1] + mOrigin[2] * m[1][2];
    mDir[2] = mDir[2] * m[2][0] + mOrigin[1] * m[2][1] + mOrigin[2] * m[2][2];
    mDir = glm::normalize(mDir); */
    
    mDir = glm::normalize(glm::vec3(m * glm::vec4(mDir, 0.0)));
}

