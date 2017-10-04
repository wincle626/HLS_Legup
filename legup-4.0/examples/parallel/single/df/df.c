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
#define OMP_ACCEL 4
#define sinN 36
#define addN 760
#define mulN 820
#define divN 432
#include <stdio.h>
#include "softfloat.c"
#include "df.h"


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
  m_rad2 = float64_neg (float64_mul (rad, rad));
  do
    {
      diff = float64_div (float64_mul (diff, m_rad2),
			  int32_to_float64 ((2 * inc) * (2 * inc + 1)));
      app = float64_add (app, diff);
      inc++;
    }
  while (float64_ge (float64_abs (diff), 0x3ee4f8b588e368f1ULL));	/* 0.00001 */
  return app;
}




int dfsin() {
    
    int i, main_result=0;
	 // #pragma omp parallel for num_threads(OMP_ACCEL) private(i)
      for (i = 0; i < sinN; i++)
	{
	  float64 result;
	  result = float64_sin (sin_test_in[i]);
	  main_result += (result == sin_test_out[i]);

	}

    return main_result;
}


int add() {
    
    int i, main_result=0;
	 // #pragma omp parallel for num_threads(OMP_ACCEL) private(i)
      for (i = 0; i < addN; i++)
	{
	  float64 result;
	  float64 x1 = add_a_input[i];
	  float64 x2 = add_b_input[i];
	  result = float64_add (x1, x2);
	  main_result += (result == add_z_output[i]);

	}
    return main_result;
}

int mult() {
    
    int i, main_result=0;
    //#pragma omp parallel for num_threads(OMP_ACCEL) private(i)
    for (i = 0; i < mulN; i++)
    {
    float64 result;
    //tid = omp_get_thread_num();
    float64 x1 = mul_a_input[i];
    float64 x2 = mul_b_input[i];
    result = float64_mul (x1, x2);
	main_result += (result == mul_z_output[i]);
    //temp[tid] += (result == mul_z_output[i]);
    }
    return main_result;
}

int div() {
   
    int i, main_result = 0;
    //#pragma omp parallel for num_threads(OMP_ACCEL) private(i)
    for (i = 0; i < divN; i++)
    {
    float64 result;
    float64 x1 = div_a_input[i];
    float64 x2 = div_b_input[i];
    result = float64_div (x1, x2);
    main_result += (result == div_z_output[i]);
    }
    return main_result;
}

int
main ()
{
  int main_result=0;
  int i;
      main_result+= add();
      main_result+= mult();
      main_result+= div();
      main_result+= dfsin();
      printf ("Sum: %d\n", main_result);
      if (main_result == sinN+divN+mulN+addN) {
          printf("PASS\n");
      } else {
          printf("FAIL\n");
      }
      return main_result;
    }
