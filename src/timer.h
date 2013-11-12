#ifndef __trichoplax__timer__
#define __trichoplax__timer__

#include <string>
#include <time.h>

class Timer {
public:
    Timer();
    
    void start();
    const std::string elapsed() const;
    
private:
    clock_t mStart;
};

#endif
