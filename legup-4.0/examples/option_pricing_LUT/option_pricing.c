#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "pricing.h"
#include "fixedptc.h"
#include "fixedptc.c"
#include "pricing.c"

fixedpt asset_path_test ( int seed );

int main ( void ){

    int run = 32;
    int dummySeed = 123;
    int i, ret;
    fixedpt sum = 0;
    for (i =0; i<run; i++) {
      fixedpt u;
      u = asset_path_test (get_uniform_fixed(&dummySeed));
      printf(" u = %d\n", u >> (FIXEDPT_BITS - FIXEDPT_WBITS));
      sum += (u >> (FIXEDPT_BITS - FIXEDPT_WBITS));
    }

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

    s = asset_path_fixed_simplified ( s0, mu, sigma, t1, n, &seed);
    
    return s;
}
/******************************************************************************/


