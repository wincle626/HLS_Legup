// Calculate constants that will infer a DSP
// Author: Andrew Canis
// Date: June 14, 2012
//
// x * 10 is equivalent to: 
// = x * (2^3 + 2)
// = (x << 3) + (x << 1)
// quartus will not infer a DSP if a multiply by constant
// can be replaced by shifts (which are free) and at most one addition
// to see if this is possible:
// 1) calculate the closest power of two
// 2) can get there by adding/subtracting x or (x << n)?
//
// to run:
// gcc -lm dsp_infer.c; ./a.out

#include <stdio.h>
#include <math.h>

#define DEBUG 0

int main(void) {
    
    int i, s, dist, closestpower2, addpower2;
    int num = 0;
    for (i = 0; i < 2000; i++) {
        if (DEBUG) printf("i: %d\n", i);
        s = log2((double)i)+0.5;
        if (DEBUG) printf("log2(i): %d\n", s);
        closestpower2 = pow(2,s);
        if (DEBUG) printf("pow(2,s): %d\n", closestpower2);
        dist = abs(i-closestpower2);
        if (DEBUG) printf("dist=abs(i-pow2): %d\n", dist);
        // a power of two
        if (dist == 0) continue;
        // within one addition/subtraction of a power of two
        s = log2((double)dist)+0.5;
        if (DEBUG) printf("s=log2(dist): %d\n", s);
        addpower2 = pow(2,s);
        if (DEBUG) printf("pow(2,s): %d\n", addpower2);
        dist = abs(dist-addpower2);
        if (DEBUG) printf("dist=abs(dist-pow2): %d\n", dist);
        if (dist == 0) continue;
        //if (closestpower2add == 1) continue;
        // with an even addition
        //if (a % 2 == 0) continue;
        printf("constant input to infer DSP: %d\n", i);
        num++;
        //if (num == 16) break;
    }
}


