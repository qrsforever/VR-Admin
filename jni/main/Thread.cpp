#include "Thread.h"

#include <map>

namespace VR {

static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;
static std::map<pthread_t, Thread *> gThreads;

static pthread_mutex_t gStartupMutex = PTHREAD_MUTEX_INITIALIZER;

Thread::Thread()
{
    pthread_mutex_lock(&gStartupMutex);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&mID, &attr, threadEntry, this);
    pthread_mutex_lock(&gMutex);
    gThreads.insert(std::make_pair(mID, this));
    pthread_mutex_unlock(&gMutex);
}

Thread::~Thread()
{
    pthread_mutex_lock(&gMutex);
    std::map<pthread_t, Thread *>::iterator it;
    it = gThreads.find(mID);
    if (it != gThreads.end())
        gThreads.erase(it);
    pthread_mutex_unlock(&gMutex);
}

void
Thread::start()
{
    pthread_mutex_unlock(&gStartupMutex);
}

void *
Thread::threadEntry(void *r)
{
    pthread_mutex_lock(&gStartupMutex);
    pthread_mutex_unlock(&gStartupMutex);

    Thread *self = (Thread *)r;
    if (self)
        self->run();
    return (void*)0;
}

}
