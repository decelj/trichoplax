#include "hit.h"
#include "ray.h"

#include "iprimitive.h"


Hit::Hit(const Ray& r)
    : P(r.point(r.mMaxT))
    , Ng(r.mHitPrim->normal(P, r.mHitBarycentrics))
    , N(r.didHitBackFace() ? -Ng : Ng)
    , V(-r.mDir)
    , I(r.mDir)
    , hitBackFace(r.didHitBackFace())
{
}
