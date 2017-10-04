#include <stdio.h>
#include <pthread.h> 

#define SIZE 500

#include "hash.h"

#define NUM_ACCEL 4
#define OMP_ACCEL 2
//#define PRINT_RESULT
//#define CHECK_COLLISION
//#define CHECK_RESULT
#define BINSIZE 500

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex4 = PTHREAD_MUTEX_INITIALIZER;

void* hash1()
{
	int result[BINSIZE] = {0};
	int i, a, tid, collision=0;
	int collisions[OMP_ACCEL] = {0, 0};
	#pragma omp parallel for num_threads(OMP_ACCEL) private(i, a, tid)
	for (i=0; i<SIZE; i++) {
		a = input[i];
		a = (a+0x7ed55d16) + (a<<12);
		a = (a^0xc761c23c) ^ (a>>19);
		a = (a+0x165667b1) + (a<<5);
		a = (a+0xd3a2646c) ^ (a<<9);
		a = (a+0xfd7046c5) + (a<<3);
		a = (a^0xb55a4f09) ^ (a>>16);
		a = (a<0) ? -1*a : a;	
		int hash = a%BINSIZE;
		pthread_mutex_lock (&mutex1);
		int temp = result[hash]++;
        collision = (temp!=0) ? collision+1 : collision;
		pthread_mutex_unlock (&mutex1);
	}
	pthread_exit((void*)collision);
}

void* hash2()
{
	int result[BINSIZE] = {0};
	int i, a, tid, collision=0;
	int collisions[OMP_ACCEL] = {0, 0};
	#pragma omp parallel for num_threads(OMP_ACCEL) private(i, a, tid)
	for (i=0; i<SIZE; i++) {
		a = input[i];
		a -= (a<<6);
		a ^= (a>>17);
		a -= (a<<9);
		a ^= (a<<4);
		a -= (a<<3);
		a ^= (a<<10);
		a ^= (a>>15);
		a = (a<0) ? -1*a : a;	
		int hash = a%BINSIZE;
		pthread_mutex_lock (&mutex2);
		int temp = result[hash]++;
        collision = (temp!=0) ? collision+1 : collision;
		pthread_mutex_unlock (&mutex2);
	}
	pthread_exit((void*)collision);
}

void* hash3()
{
	int result[BINSIZE] = {0};
	int i, a, tid, collision=0;
	int collisions[OMP_ACCEL] = {0, 0};
	#pragma omp parallel for num_threads(OMP_ACCEL) private(i, a, tid)
	for (i=0; i<SIZE; i++) {
		a = input[i];
	    a = (a ^ 61) ^ (a >> 16);
		a = a + (a << 3);
	    a = a ^ (a >> 4);
	    a = a * 0x27d4eb2d;
	    a = a ^ (a >> 15);
		a = (a<0) ? -1*a : a;	
		int hash = a%BINSIZE;
		pthread_mutex_lock (&mutex3);
		int temp = result[hash]++;
        collision = (temp!=0) ? collision+1 : collision;
		pthread_mutex_unlock (&mutex3);
	}
	pthread_exit((void*)collision);
}

void* hash4()
{
	int result[BINSIZE] = {0};
	int i, a, tid, collision=0;
	int collisions[OMP_ACCEL] = {0, 0};
	#pragma omp parallel for num_threads(OMP_ACCEL) private(i, a, tid)
	for (i=0; i<SIZE; i++) {
		a = input[i];
	    a = (a+0x479ab41d) + (a<<8);
	    a = (a^0xe4aa10ce) ^ (a>>5);
	    a = (a+0x9942f0a6) - (a<<14);
	    a = (a^0x5aedd67d) ^ (a>>3);
	    a = (a+0x17bea992) + (a<<7);
		a = (a<0) ? -1*a : a;	
		int hash = a%BINSIZE;
		pthread_mutex_lock (&mutex4);
		int temp = result[hash]++;
        collision = (temp!=0) ? collision+1 : collision;
		pthread_mutex_unlock (&mutex4);
	}
	pthread_exit((void*)collision);
}

#define CHECK_RESULT
int main() {
	int i, j;
	int collision[NUM_ACCEL] = {0};
	pthread_t threads[NUM_ACCEL];

	//launch threads
	pthread_create(&threads[0], NULL, hash1, NULL);
	pthread_create(&threads[1], NULL, hash2, NULL);
	pthread_create(&threads[2], NULL, hash3, NULL);
	pthread_create(&threads[3], NULL, hash4, NULL);
	 
	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], (void**)&collision[i]);
	}
#ifdef PRINT_RESULT
	printf("result1\n");
	for (i=0; i<SIZE; i++) {
		printf("%d,\n", result1[i]);
	}
	printf("result2\n");
	for (i=0; i<SIZE; i++) {
		printf("%d,\n", result2[i]);
	}
	printf("result3\n");
	for (i=0; i<SIZE; i++) {
		printf("%d,\n", result3[i]);
	}
	printf("result4\n");
	for (i=0; i<SIZE; i++) {
		printf("%d,\n", result4[i]);
	}
#endif
#ifdef CHECK_COLLISION
	for (i=0; i<SIZE; i++) {
		for (j=i+1; j<SIZE; j++) {
			if (result1[i] == result1[j]) {
				collision1++;
			}
			if (result2[i] == result2[j]) collision2++;
			if (result3[i] == result3[j]) collision3++;
			if (result4[i] == result4[j]) collision4++;
		}
	}

	printf("collision1 = %d, collision2 = %d, collision3 = %d, collision4 = %d\n", collision[0], collision[1], collision[2], collision[3]);
#endif	
#ifdef CHECK_RESULT
	printf("collision1 = %d, collision2 = %d, collision3 = %d, collision4 = %d\n", collision[0], collision[1], collision[2], collision[3]);
	int count=0;
    count += (collision[0] == 178);
    count += (collision[1] == 182);
    count += (collision[2] == 196);
    count += (collision[3] == 179);
	printf("result = %d\n", count);
	if (count == 4) {
		printf("PASS\n");
	}
	else {
		printf("FAIL\n");
	}
#endif
	return 0;
}
