#include <sstream>
#include <iostream>
#include <iomanip>

#include "timer.h"

Timer::Timer() :
    mStart(0)
{ }

void Timer::start() {
    mStart = clock();
}

const std::string Timer::elapsed() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << (static_cast<float>(clock() - mStart) / CLOCKS_PER_SEC);
    
    return ss.str();
}
