#include <assert.h>

#include "hit.h"
#include "ray.h"

Hit::Hit(const Ray& r) :
    P(r.point(r.mMaxT)),
    N(r.mHitPrim->normal(P)),
    I(-r.mDir)
{ }
