/*
+--------------------------------------------------------------------------+
| CHStone : a suite of benchmark programs for C-based High-Level Synthesis |
| ======================================================================== |
|                                                                          |
| * Collected and Modified : Y. Hara, H. Tomiyama, S. Honda,               |
|                            H. Takada and K. Ishii                        |
|                            Nagoya University, Japan                      |
|                                                                          |
| * Remark :                                                               |
|    1. This source code is modified to unify the formats of the benchmark |
|       programs in CHStone.                                               |
|    2. Test vectors are added for CHStone.                                |
|    3. If "main_result" is 0 at the end of the program, the program is    |
|       correctly executed.                                                |
|    4. Please follow the copyright of each benchmark program.             |
+--------------------------------------------------------------------------+
*/
/*
 * Copyright (C) 2008
 * Y. Hara, H. Tomiyama, S. Honda, H. Takada and K. Ishii
 * Nagoya University, Japan
 * All rights reserved.
 *
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis. The authors disclaims any and all warranties, 
 * whether express, implied, or statuary, including any implied warranties or 
 * merchantability or of fitness for a particular purpose. In no event shall the
 * copyright-holder be liable for any incidental, punitive, or consequential damages
 * of any kind whatsoever arising from the use of these programs. This disclaimer
 * of warranty extends to the user of these programs and user's customers, employees,
 * agents, transferees, successors, and assigns.
 *
 */
#define NUM_ACCEL 4
#define OMP_ACCEL 3
#define sinN 144

#define OPS_PER_ACCEL sinN/NUM_ACCEL

#include <stdio.h>
#include <pthread.h>
#include "softfloat.c"
#include "df.h"

struct thread_data{
   int  startidx;
   int  maxidx;
};

float64
float64_abs (float64 x)
{
  return (x & 0x7fffffffffffffffULL);
}

float64
float64_sin (float64 rad)
{
  float64 app;
  float64 diff;
  float64 m_rad2;
  int inc;

  app = diff = rad;
  inc = 1;
  m_rad2 = float64_neg (float64_mul (rad, rad, 3));
  do
    {
      diff = float64_div (float64_mul (diff, m_rad2, 3),
			  int32_to_float64 ((2 * inc) * (2 * inc + 1)), 3);
      app = float64_add (app, diff, 3);
      inc++;
    }
  while (float64_ge (float64_abs (diff), 0x3ee4f8b588e368f1ULL, 3));	/* 0.00001 */
  return app;
}

void* dfsin(void* threadarg) {
    
    int i, tid, main_result=0;
    int temp[OMP_ACCEL] = {0};
	struct thread_data* arg = (struct thread_data*) threadarg;
	int startidx = arg->startidx;
	int maxidx = arg->maxidx;
	#pragma omp parallel for num_threads(OMP_ACCEL) private(i, tid)
	for (i=startidx; i<maxidx; i++)
	{
	  float64 result;
      tid = omp_get_thread_num();
	  result = float64_sin (sin_test_in[i]);
      temp[tid] += (result == sin_test_out[i]);

	}
    for (i=0; i<OMP_ACCEL; i++) {
        main_result += temp[i];
    }
    pthread_exit((void*)main_result);
}

int
main ()
{
    legup_start_counter(0);
    int main_result=0;
    int i;
	pthread_t threads[NUM_ACCEL];
	int result[NUM_ACCEL]={0};
	struct thread_data data[NUM_ACCEL];

	for (i=0; i<NUM_ACCEL; i++) {
		//initialize structs to pass into accels
		data[i].startidx = i*OPS_PER_ACCEL;
		data[i].maxidx = (i+1)*OPS_PER_ACCEL;
	}
	
    //launch threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_create(&threads[i], NULL, dfsin, (void *)&data[i]);
	}
    
	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], (void**)&result[i]);
	}

    for (i=0; i<NUM_ACCEL; i++) {
        main_result += result[i];
    }
    int perf_counter = legup_stop_counter(0);
    printf("perf_counter = %d\n", perf_counter);
    printf ("Sum: %d\n", main_result);
    if (main_result == sinN) {
      printf("PASS\n");
    } else {
      printf("FAIL\n");
    }
    return main_result;
    }
