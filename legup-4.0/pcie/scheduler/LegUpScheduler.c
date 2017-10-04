
#include "LegUpScheduler.h"
#include <assert.h>
#include <stdio.h>

#define EXTRACT_BIT(v, i) ((v) & (1L << (i)))
#define SET_BIT(v, i) ((v) | (1L << (i)))
#define UNSET_BIT(v, i) ((v) & (~(1L << (i))))

// Find the next zero bit, set it to 1 and return the bit location.
// If can't find a zero bit within total, returns -1;
static int getAndSetNextAvailableBit(BitVector * pv, int total)
{
    assert(total < (sizeof(BitVector) * 8));
    int i;

    for (i = 0; i < total; i++) {
        if (EXTRACT_BIT(*pv, i) == 0) {
            *pv = SET_BIT(*pv, i);
            return i;
        }
    }
    return -1;
}

// Find the next available accel,
// return the accelNum if one is available,
// wait until a accel is availableotherwise
AccelHandle getAccel(Scheduler *s)
{
    assert(s != NULL);
    assert(s->totalOfAccels >= s->numOfUsedAccels);

    int ret;
    AccelHandle handle;

    if ((ret = pthread_mutex_lock(&s->lock)) != 0) {
        assert(0 && "ERROR: Fail to acquire lock.\n");
    }

    while (s->numOfUsedAccels >= s->totalOfAccels) {
        pthread_cond_wait(&s->waitForAvailableAccel,
                &s->lock);
    }

    handle = (AccelHandle) getAndSetNextAvailableBit(
            &s->availableAccels, s->totalOfAccels);
    // fprintf(stderr, "getAccel acquired accel %d %d\n", handle, s->totalOfAccels);
    assert(handle != INVAILD_HANDLE && "ERROR: Unable to find available bits");

    s->numOfUsedAccels ++;
    assert(s->totalOfAccels >= s->numOfUsedAccels);

    if ((ret = pthread_mutex_unlock(&s->lock)) != 0) {
        assert(0 && "ERROR: Fail to release lock.\n");
    }

    return handle;
}

int getAccelIfAvailable(Scheduler *s)
{
    assert(s != NULL);
    assert(s->totalOfAccels >= s->numOfUsedAccels);

    int ret;
    AccelHandle handle = INVAILD_HANDLE;

    if ((ret = pthread_mutex_lock(&s->lock)) != 0) {
        assert(0 && "ERROR: Fail to acquire lock.\n");
    }

    if (s->numOfUsedAccels < s->totalOfAccels) {
        handle = (AccelHandle) getAndSetNextAvailableBit(
                &s->availableAccels, s->totalOfAccels);
        // fprintf(stderr, "getAccelIfAvailable acquired accel %d %d\n", handle, s->totalOfAccels);
    }

    // fprintf(stderr, "acquired accel %d\n", handle);

    if (INVAILD_HANDLE != handle) s->numOfUsedAccels ++;
    assert(s->totalOfAccels >= s->numOfUsedAccels);

    if ((ret = pthread_mutex_unlock(&s->lock)) != 0) {
        assert(0 && "ERROR: Fail to release lock.\n");
    }

    return handle;
}

// Free the given accel with accelNum
void freeAccel(Scheduler *s, AccelHandle handle)
{
    assert(s != NULL);
    assert(handle != INVAILD_HANDLE);
    assert(handle >= 0 && handle < s->totalOfAccels);

    int ret;

    if ((ret = pthread_mutex_lock(&s->lock)) != 0) {
        assert(0 && "ERROR: Fail to acquire lock.\n");
    }

    s->availableAccels = UNSET_BIT(s->availableAccels, handle);
    s->numOfUsedAccels --;
    assert(s->numOfUsedAccels >= 0);
    // fprintf(stderr, "released accel %d\n", handle);

    pthread_cond_signal(&s->waitForAvailableAccel);

    if ((ret = pthread_mutex_unlock(&s->lock)) != 0) {
        assert(0 && "ERROR: Fail to release lock.\n");
    }

}

