#include "noise.h"
#include <random>

Noise::Noise() :
    mDevice(),
    mGenerator(mDevice())
{}

Noise::~Noise()
{}