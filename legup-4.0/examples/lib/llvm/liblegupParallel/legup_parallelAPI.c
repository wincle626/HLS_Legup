//#define OMP_NUM_THREADS	(volatile int*)  0xC5000004
//#define OMP_THREAD_ID	(volatile int*)  0xC5000000

#define LOCK	(volatile int*)  0xC5000000
#define UNLOCK	(volatile int*)  0xC5000010
#define LOCK_OFFSET	8

#define BARRIER_INIT	(volatile int*)  0xC5001000
//#define BARRIER_WAIT	(volatile int*)  0xC5000044
#define BARRIER_WAIT	(volatile int*)  0xC5001010

//lock function which writes the threadID to the mutex core
//then reads from the mutex core to check if the stored threadID matches its own threadID
//threadID needs to be added by 1, to prevent the accel with threadID of 0 
//from being confused that it is the owner of the mutex 
//since the stored threadID of mutex core is set to 0 when it is in reset, or just released from another accel

/*
void legup_lock(int threadID, int mutexNum) {
	do {
		*(LOCK+LOCK_OFFSET*mutexNum) = threadID+1;
    } while (*(LOCK+LOCK_OFFSET*mutexNum) != threadID+1);
}
*/
// poll until the status turns 1, which means the mutex is free
// when the mutex is free, reading from the mutex sets the status of the mutex to 0
// making it locked
void legup_lock(int mutexNum) {
    while (*(LOCK+LOCK_OFFSET*mutexNum) == 0);
}
/*
int legup_lock2(int mutexNum) {
    while (*(LOCK+LOCK_OFFSET*mutexNum) == 0);

    return 1;
}
*/
//unlock function which writes the threadID to the mutex core
//if the stored threadID matches its own threadID, this frees the mutex core 
//threadID needs to be added by 1, for the same reason as the lock function
/*
void legup_unlock(int threadID, int mutexNum) {
	*(UNLOCK+LOCK_OFFSET*mutexNum) = threadID+1;
}
*/
void legup_unlock(int mutexNum) {
	*(UNLOCK+LOCK_OFFSET*mutexNum) = 1;
}

void legup_barrier_init(int n) {
	*BARRIER_INIT = n;
}

void legup_barrier_wait() {
	*BARRIER_WAIT = 1;
	while (*BARRIER_WAIT != 0) {
	}
}
