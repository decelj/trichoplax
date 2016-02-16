#ifndef LDsequence_utils_h
#define LDsequence_utils_h

inline float RadicalInverse(unsigned n, unsigned base)
{
    float value = 0.f;
    float inverseBase = 1.f / (float)base;
    float inverseBi = inverseBase;
    while (n > 0)
    {
        unsigned Di = (n % base);
        value += (float)Di * inverseBi;
        n /= base;
        inverseBi *= inverseBase;
    }

    return value;
}

inline float VanDerCorputBase2(unsigned n, unsigned scramble)
{
    n = (n << 16) | (n >> 16);
    n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
    n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
    n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
    n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
    n ^= scramble;
    return (float)n / (float)0x100000000LL;
}

#endif /* LDsequence_utils_h */
