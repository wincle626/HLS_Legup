#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#define RUN 32
#define NUM_ACCEL 4
#define OMP_ACCEL 4
#define OPS_PER_ACCEL RUN/NUM_ACCEL

#include "pricing.h"
#include "fixedptc.h"
#include "fixedptc.c"
#include "pricing.c"
#include "seed.h"

fixedpt asset_path_test ( int seed );

void *black_scholes(void* threadarg) {

    int i, tid;
    fixedpt temp[OMP_ACCEL]={0};
    fixedpt sum=0;
	int* arg = (int*) threadarg;
	int offset = *arg;
    fixedpt u;
    
	#pragma omp parallel for num_threads(OMP_ACCEL) private(i, u, tid)
    for (i =0; i<RUN/NUM_ACCEL; i++) {
      tid = omp_get_thread_num();
      //fixedpt seed = get_uniform_fixed(&dummySeed); 
      //printf("%d\n", seed);
      fixedpt seed = seeds[i+offset];
      u = asset_path_test (seed);
      //printf("seed = %d, u = %d\n", seed, u >> (FIXEDPT_BITS - FIXEDPT_WBITS));
      result[i+offset] = u;           
      temp[tid] += (u >> FIXEDPT_BITS - FIXEDPT_WBITS);
    }
    for (i=0; i<OMP_ACCEL; i++) {
        sum += temp[i];
    }
    
	pthread_exit((void*)sum);
}

int main ( void ){
    legup_start_counter(0);
    int i, ret;
    fixedpt sum = 0;
	//create the thread variables
	fixedpt result[NUM_ACCEL] = {0};
	pthread_t threads[NUM_ACCEL];
	int data[NUM_ACCEL];

	for (i=0; i<NUM_ACCEL; i++) {
		data[i] = i*OPS_PER_ACCEL;
	}

	//launch threads
	//for (i=0; i<1; i++) {
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_create(&threads[i], NULL, black_scholes, (void *)&data[i]);
	}
	 
	//join the threads
	//for (i=0; i<1; i++) {
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], (void**)&result[i]);
	}

	for (i=0; i<NUM_ACCEL; i++) {
		sum += result[i];
	}
    int perf_counter = legup_stop_counter(0);
    printf("perf_counter = %d\n", perf_counter);
    printf("sum = %d\n", sum); // the golden result will be the SUM of the prices
    if (sum == 11159) {
      printf("RESULT: PASS\n");
    }
    else {
      printf("RESULT: FAIL\n");
    }
    ret = sum;
    return ret;
}

fixedpt asset_path_test (int seed ){
    
    int n = 100;
    fixedpt mu, s0, sigma, t1;
    fixedpt s;
    //int holdSeed = seed;

    s0 = 13107200; // fixedpt_rconst(200.0);
//        printf("%d\n", s0>> (FIXEDPT_BITS - FIXEDPT_WBITS));
    
    mu = 16384; // fixedpt_rconst(0.25);
//        printf("%d\n", mu>> (FIXEDPT_BITS - FIXEDPT_WBITS));

    sigma = 4391; //fixedpt_rconst(0.067);
//        printf("%d\n", sigma>> (FIXEDPT_BITS - FIXEDPT_WBITS));

    t1 = 131072; // fixedpt_rconst(2.0);
//        printf("%d\n", t1>> (FIXEDPT_BITS - FIXEDPT_WBITS));

    s = asset_path_fixed_simplified ( s0, mu, sigma, t1, n, seed);
    //s = asset_path_fixed_simplified ( s0, mu, sigma, t1, n, &seed);
    
    return s;
}
/******************************************************************************/


