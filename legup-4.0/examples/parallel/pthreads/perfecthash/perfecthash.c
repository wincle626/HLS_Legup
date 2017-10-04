#define ARRAY_SIZE 24000
#define NUM_ACCEL 6
#define OPS_PER_ACCEL ARRAY_SIZE/NUM_ACCEL

typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned short int  ub2;
typedef  unsigned       char ub1;

#include <stdio.h>
#include <pthread.h>
#include "perfecthash.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int final_result = 0;

struct thread_data{
   int  startidx;
   int  maxidx;
};

ub4 phash(ub4 val, ub2 * scramble, ub1 * tab)
{
	ub4 a, b, rsl;

	b = (val >> 5) & 0x1fff;
	a = ((val << 22) >> 19);
	rsl = (a^scramble[tab[b]]);
	return rsl;
}

void *perfect_hash (void *threadarg) {
	int i, hash;
	int result=0;
	struct thread_data* arg = (struct thread_data*) threadarg;
	int startidx = arg->startidx;
	int maxidx = arg->maxidx;

	for (i=startidx; i<maxidx; i++) {
		hash=phash(key[i], scramble, tab);
		result +=(hash_table[hash] == key[i]);
		//printf("hash_table[hash] = %d, key[i] = %d\n", hash_table[hash], key[i]);
	}

	pthread_mutex_lock (&mutex);
	final_result += result;
	pthread_mutex_unlock (&mutex);	
	pthread_exit(NULL);
}

int main() {
	
	int i;
	//create the thread variables
	pthread_t threads[NUM_ACCEL];
	struct thread_data data[NUM_ACCEL];

	for (i=0; i<NUM_ACCEL; i++) {
		data[i].startidx = i*OPS_PER_ACCEL;
		data[i].maxidx = (i+1)*OPS_PER_ACCEL;
	}

	for (i=0; i<NUM_ACCEL; i++) {
		//initialize structs to pass into accels
//		data[i].startidx = i*OPS_PER_ACCEL;
//		data[i].maxidx = (i+1)*OPS_PER_ACCEL;
		pthread_create(&threads[i], NULL, perfect_hash, (void *)&data[i]);
	}

	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], NULL);
	}

	//check final result
	printf ("Result: %d\n", final_result);
	if (final_result == 24000) {
		printf("RESULT: PASS\n");
	} else {
		printf("RESULT: FAIL\n");
	}
}
