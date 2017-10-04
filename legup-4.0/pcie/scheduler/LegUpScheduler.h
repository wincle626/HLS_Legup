
#ifndef LEGUPSCHEDULER_H
#define LEGUPSCHEDULER_H

#include <pthread.h>

typedef long long BitVector;
typedef int       AccelHandle;
#define INVAILD_HANDLE (-1)

typedef struct {
    BitVector availableAccels;
    int       totalOfAccels;
    int       numOfUsedAccels;

    pthread_cond_t  waitForAvailableAccel;
    pthread_mutex_t lock;
} Scheduler;

#define SCHEDULER_INIT(total) {\
    /* availableAccels = */ (BitVector)0, \
    /* totalOfAccels = */   total, \
    /* numOfUsedAccels = */ 0, \
    PTHREAD_COND_INITIALIZER, \
    PTHREAD_MUTEX_INITIALIZER \
}

AccelHandle getAccel(Scheduler *s);
int getAccelIfAvailable(Scheduler *s);
void freeAccel(Scheduler *s, AccelHandle handle);

#endif
