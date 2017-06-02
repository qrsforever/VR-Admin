#ifndef __UTILS_THREAD__H_
#define __UTILS_THREAD__H_

#include <pthread.h>

namespace VR {

class Thread {
public:
    Thread();
    virtual ~Thread();

    void start();
    virtual void run() = 0;

private:
    pthread_t mID;

    static void *threadEntry(void *r);
};

}

#endif
