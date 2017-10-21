#include "hit.h"
#include "ray.h"

#include "triangle.h"


Hit::Hit(const Ray& r)
    : P(r.point(r.mMaxT))
    , Ng(r.mHitPrim->normal())
    , N(r.mHitPrim->interpolateNormal(P, r.mHitBarycentrics))
    , V(-r.mDir)
    , dPdU(0.f)
    , dPdV(0.f)
    , uv(r.mHitPrim->uv(r.mHitBarycentrics))
    , hitBackFace(r.didHitBackFace())
{
    r.mHitPrim->positionPartials(N, dPdU, dPdV);
    if (glm::dot(dPdU, dPdU) <= EPSILON)
    {
        TP_ASSERT(glm::dot(dPdV, dPdV) > EPSILON);
        tangent = glm::normalize(dPdV);
    }
    else
    {
        tangent = glm::normalize(dPdU);
    }

    bitangent = glm::normalize(glm::cross(N, tangent));
    tangent = glm::normalize(glm::cross(N, bitangent));
}
