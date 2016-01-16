#ifndef __trichoplax__timer__
#define __trichoplax__timer__

#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>

template<typename ClockT>
class _Timer {
public:
    typedef typename ClockT::time_point time_point;
    typedef typename ClockT::duration duration;

    _Timer();
    
    void start();
    duration elapsed() const;
    std::string elapsedToString(duration elapsed);
    
private:
    time_point mStart;
};


template<typename ClockT>
_Timer<ClockT>::_Timer()
    : mStart()
{
}

template<typename ClockT>
inline void _Timer<ClockT>::start()
{
    mStart = ClockT::now();
}

template<typename ClockT>
inline typename _Timer<ClockT>::duration _Timer<ClockT>::elapsed() const
{
    return ClockT::now() - mStart;
}

template<typename ClockT>
inline std::string _Timer<ClockT>::elapsedToString(typename _Timer<ClockT>::duration elapsed)
{
    using namespace std::chrono;
    using namespace std;

    ostringstream ss;
    ss << setw(2) << setfill('0') << duration_cast<hours>(elapsed).count() << ":"
       << setw(2) << setfill('0') << duration_cast<minutes>(elapsed % hours(1)).count() << ":"
       << setw(2) << setfill('0') << duration_cast<seconds>(elapsed % minutes(1)).count() << "."
       << setw(3) << setfill('0') << duration_cast<milliseconds>(elapsed % seconds(1)).count();

    return ss.str();
}

template<>
inline std::string _Timer<std::chrono::high_resolution_clock>::elapsedToString(typename _Timer<std::chrono::high_resolution_clock>::duration elapsed)
{
    using namespace std::chrono;
    using namespace std;

    ostringstream ss;
    ss << setw(2) << setfill('0') << duration_cast<seconds>(elapsed).count() << "."
       << setw(3) << setfill('0') << duration_cast<milliseconds>(elapsed % seconds(1)).count();

    return ss.str();
}

typedef _Timer<std::chrono::system_clock> Timer;
typedef _Timer<std::chrono::high_resolution_clock> HighResTimer;

#endif
