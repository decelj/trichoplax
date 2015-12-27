#include <algorithm>
#include <limits>

#include "aabbox.h"

AABBox::AABBox()
{
    mBounds[0] = glm::vec3(std::numeric_limits<float>::max());
    mBounds[1] = glm::vec3(std::numeric_limits<float>::max());
}

AABBox::AABBox(const glm::vec3& ll, const glm::vec3& ur)
{
    mBounds[0] = ll;
    mBounds[1] = ur;
}

short AABBox::longestAxis() const
{
    glm::vec3 result = mBounds[1] - mBounds[0];
    result[0] = fabsf(result[0]);
    result[1] = fabsf(result[1]);
    result[2] = fabsf(result[2]);
    
    if (result[0] > result[1] && result[0] > result[2])
        return 0;
    else if (result[1] > result[0] && result[1] > result[2])
        return 1;
    
    return 2;
}

void AABBox::split(AABBox* outLeft, AABBox* outRight, float plane, unsigned axis) const
{
    glm::vec3 leftUR = mBounds[1];
    leftUR[axis] = plane;
    outLeft->update(mBounds[0], leftUR);
    
    glm::vec3 rightLL = mBounds[0];
    rightLL[axis] = plane;
    outRight->update(rightLL, mBounds[1]);
}
