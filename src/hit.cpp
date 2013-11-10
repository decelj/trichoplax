#include <assert.h>

#include "hit.h"
#include "ray.h"

Hit::Hit(const Ray& r) :
    mPrim(r.mHitPrim),
    P(r.point(r.mHitT)),
    N(r.mHitPrim->normal(P)),
    I(-r.mDir)
{ }
