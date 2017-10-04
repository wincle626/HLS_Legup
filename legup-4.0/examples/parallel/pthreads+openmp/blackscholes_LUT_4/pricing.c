# include "pricing.h"
# include "fixedptc.h"

fixedpt gauss[]=
    {
        65491,  89346,  83182,  20223,  22561,  99038,  -22329, -110084,        84740,  1581,
        -111835,        36218,  23393,  62585,  27236,  5964,   8282,   8259,   15276,  -48891,
        -33840, -46799, -108009,        897,    13784,  -27635, 71581,  -11815, 36092,  -40768,
        39878,  26148,  75646,  -18380, -2596,  21789,  107708, 19317,  -37662, -13205,
        -29976, -41331, 21166,  83351,  169124, 53424,  89178,  30209,  46099,  66819,
        61548,  -103641,        29733,  87278,  33890,  -107341,        89330,  -38737, 98572,  -33020,
        -82833, 39655,  24114,  65664,  -72907, 22864,  105109, 105081, 28411,  -2148,
        -52060, -8852,  15555,  22608,  82986,  -63801, 60344,  -41525, -44346, 21411,
        103126, 27711,  -59951, -44394, -46044, 43166,  35303,  42726,  22657,  -44729,
        -24145, 112622, -13066, 63949,  -54951, 34804,  67172,  -1354,  -34293, 112109,
        -32318, 73849,  -12666, -113970,        -31972, -53898, -14703, -58318, -79491, 2641,
        -63713, -61847, -12430, -74801, -56338, 402,    62157,  122417, 23259,  -115926,
        19988,  -169139,        -23176, -24884, -26217, 17623,  3442,   -37369, -38714, 14170,
        -75847, -71795, -107157,        -29159, 24374,  -6658,  34094,  126746, 6602,   92904,
        -21598, -149237,        -2310,  20464,  -17258, 19044,  4454,   34591,  -52753, 66556,
        66829,  -34947, -103733,        10690,  63589,  34366,  40831,  -75573, 2858,   -19609,
        -55477, 34710,  -63400, 40086,  -10831, -38671, 56497,  6311,   18599,  -28058,
        -15821, 29690,  49490,  -126859,        72042,  -67179, 22573,  42507,  76515,  -48001,
        -48543, -1383,  26673,  -77791, 129492, -74030, -27664, -25854, -27459, -93813,
        -116971,        -93481, 74236,  114673, -57131, -123663,        -4024,  -36024, -17738, -85856,
        38957,  41646,  12327,  -91003, -23459, 7391,   -43082, 32548,  100817, 52515,
        14132,  49405,  122888, 38075,  62222,  -3335,  138669, 85488,  70903,  -92323,
        -27406, 13110,  -14431, 14750,  -68374, -49634, -726,   -105063,        11694,  -24827,
        2731,   -13821, 30139,  -5423,  77546,  -42791, -29774, 29109,  50366,  -51235,
        -9850,  4040,   -102047,        49429,  -2937,  -9161,  31629,  -14355, 44817,  57398,
        119664, 63631,  -12084, -58272, 5724,   -3325,
    };

fixedpt asset_path_fixed_simplified ( fixedpt s0, fixedpt mu, fixedpt sigma, fixedpt t1, int n, int seed ){
    int i, tid;
    fixedpt dt, stepnum, p;
    //fixedpt gaussR1 = 0, gaussR2 = 0;
    fixedpt gaussR1[OMP_ACCEL], gaussR2[OMP_ACCEL];

    //    stepnum = fixedpt_rconst(n);
    stepnum = n << FIXEDPT_FBITS; // ??? janders
    dt = fixedpt_div(t1, stepnum);

    fixedpt constA = fixedpt_mul(fixedpt_sub(mu, fixedpt_mul(sigma, sigma)), dt);
    fixedpt constB = fixedpt_mul(sigma, fixedpt_sqrt ( dt ));

    p = s0;
    for ( i = 1; i <= n; i++ )
    {  
      tid = omp_get_thread_num();
      if (i & 1) {// iteration is odd, generate two random Gaussian numbers (the Box-Muller transform gens 2 numbers)
    	seed = get_two_normal_fixed_LUT(seed, gaussR1, gaussR2);
      }
      
      p = fixedpt_mul(p, fixedpt_exp (fixedpt_add(constA,
						  fixedpt_mul(constB, i & 1 ? gaussR1[tid] : gaussR2[tid]))));
      

      //      fixedpt_print(p);
    }
    return p;
}

//LUT for gaussian random number generator
//void get_two_normal_fixed_LUT(int *seed, fixedpt *n1, fixedpt *n2) 
int get_two_normal_fixed_LUT(int seed, fixedpt *n1, fixedpt *n2) 
{
  fixedpt r1, r2;

  r1 = get_uniform_fixed (&seed);
  r2 = get_uniform_fixed (&seed);
   
  int tid = omp_get_thread_num();
  n1 += tid;
  n2 += tid;
  *n1 = gauss[r1>>8];
  *n2 = gauss[r2>>8];

  return seed;
}


void get_two_normal_fixed(int *seed, fixedpt *n1, fixedpt *n2) 
{
  fixedpt r1, r2;

  fixedpt twoPI = 411775; // ??? janders -- hard-code 2PI in fixed point to avoid conversion from double
  // from 2 uniform random numbers r1 and r2, we will generate two Gaussian random numbers deposited into n1 and n2
  r1 = get_uniform_fixed (seed);
  r2 = get_uniform_fixed (seed);
  
  *n1 = fixedpt_mul(fixedpt_sqrt ( fixedpt_mul(-1*FIXEDPT_TWO , fixedpt_ln( r1 )) ), fixedpt_cos(fixedpt_mul( twoPI , r2)));
  *n2 = fixedpt_mul(fixedpt_sqrt ( fixedpt_mul(-1*FIXEDPT_TWO , fixedpt_ln( r1)) ), fixedpt_sin (fixedpt_mul( twoPI , r2)));
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

