#ifndef __trichoplax__scoped_lock__
#define __trichoplax__scoped_lock__

#include <pthread.h>

class ScopedLock
{
public:
    inline ScopedLock(pthread_mutex_t* lock)
        : mMutex(lock) { pthread_mutex_lock(mMutex); }
    
    inline ~ScopedLock() { pthread_mutex_unlock(mMutex); }
    
private:
    pthread_mutex_t* mMutex;
};

#endif /* defined(__trichoplax__scoped_lock__) */
