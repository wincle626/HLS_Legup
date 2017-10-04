# include "black_scholes.h"
# include "fixedptc.h"


fixedpt asset_path_fixed_simplified ( fixedpt s0, fixedpt mu, fixedpt sigma, fixedpt t1, int n, int seed ){
    int i;
    fixedpt dt, stepnum, p;
    fixedpt gaussR1[OMP_ACCEL], gaussR2[OMP_ACCEL];

    //    stepnum = fixedpt_rconst(n);
    stepnum = n << FIXEDPT_FBITS; // ??? janders
    dt = fixedpt_div(t1, stepnum);

    fixedpt constA = fixedpt_mul(fixedpt_sub(mu, fixedpt_mul(sigma, sigma)), dt);
    fixedpt constB = fixedpt_mul(sigma, fixedpt_sqrt ( dt ));

    int s = seed;
    p = s0;
    int tid;
    for ( i = 1; i <= n; i++ )
    {  
         tid = omp_get_thread_num();
      if (i & 1) { // iteration is odd, generate two random Gaussian numbers (the Box-Muller transform gens 2 numbers)
    	s = get_two_normal_fixed(s, gaussR1, gaussR2);
      }
      
      p = fixedpt_mul(p, fixedpt_exp (fixedpt_add(constA,
						  fixedpt_mul(constB, i & 1 ? gaussR1[tid] : gaussR2[tid]))));
      
//         if (tid == 0) {
//            printf("i = %d, seed = %d, gaussR1 = %d, gaussR1 = %d, p = %d\n", i, s, gaussR1[tid], gaussR2[tid], p);
//         }

      //      fixedpt_print(p);
    }
    return p;
}

int get_two_normal_fixed(int seed, fixedpt *n1, fixedpt *n2) 
{
  fixedpt r1, r2;

  fixedpt twoPI = 411775; // ??? janders -- hard-code 2PI in fixed point to avoid conversion from double
  // from 2 uniform random numbers r1 and r2, we will generate two Gaussian random numbers deposited into n1 and n2
  int s = seed;
  r1 = get_uniform_fixed (&s);
  r2 = get_uniform_fixed (&s);

  int tid = omp_get_thread_num();
  n1 += tid;
  n2 += tid;
  /*
  fixedpt ln1 = fixedpt_ln( r1 );
  fixedpt mul1 =  fixedpt_mul(-1*FIXEDPT_TWO , ln1);
  fixedpt mul2 = fixedpt_mul( twoPI , r2);
  fixedpt cos1 = fixedpt_cos(mul2);
  fixedpt sqrt1 = fixedpt_sqrt ( mul1);
  fixedpt gaussR1 = fixedpt_mul(sqrt1, cos1);
  n1 += tid;
  *n1 = gaussR1;
  fixedpt sin1 = fixedpt_sin(mul2);
  fixedpt gaussR2 = fixedpt_mul(sqrt1, sin1);
  n2 += tid;
  *n2 = gaussR2;*/
  *n1 = fixedpt_mul(fixedpt_sqrt ( fixedpt_mul(-1*FIXEDPT_TWO , fixedpt_ln( r1 )) ), fixedpt_cos(fixedpt_mul( twoPI , r2)));
  *n2 = fixedpt_mul(fixedpt_sqrt ( fixedpt_mul(-1*FIXEDPT_TWO , fixedpt_ln( r1)) ), fixedpt_sin (fixedpt_mul( twoPI , r2)));

//  if (tid==0){
//    printf("seed = %d, ln1 = %d, mul1 = %d, mul2 = %d, cos1 = %d, sin1 = %d, sqrt1 = %d, gaussR1 = %d, gaussR2 = %d\n", s, ln1, mul1, mul2, cos1, sin1, sqrt1, gaussR1, gaussR2);
  //}
  return s;
}

fixedpt get_uniform_fixed ( int *seed )

{
    int i4_huge = 2147483647;
    int k;
    fixedpt r;

    k = *seed / 127773;
    // printf("k = %i \n", k);
    
    *seed = 16807 * ( *seed - k * 127773 ) - k * 2836;
    //printf("*seed = %i \n", *seed);
    
    if ( *seed < 0 )
    {
      *seed = *seed + i4_huge;
    }

    r = *seed & 0x0000FFFF;

    return r;
}
/******************************************************************************/
