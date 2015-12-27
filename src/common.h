#ifndef __COMMON_H__
#define __COMMON_H__

#include <math.h>
#include <cassert>

#define EPSILON .000001f
#define VEC3_IS_REL_ZERO(v) (v[0] < EPSILON && v[1] < EPSILON && v[2] < EPSILON)
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#if DEBUG
#define TP_ASSERT(_exp) assert(_exp)
#else
#define TP_ASSERT(_exp)
#endif

#define TP_UNUSED(_val) ((void)(_val))

inline bool relEq(const float a, const float b, const float tol=EPSILON)
{
    return fabsf(a - b) < tol;
}

#endif

