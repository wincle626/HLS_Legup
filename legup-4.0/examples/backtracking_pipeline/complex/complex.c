#include <stdio.h>
#include <assert.h>

#define N 100
#define INITIALIZE {1,2,3,4,5,6,7,8,9,10, 11,12,13,14,15,16,17,18,19,20, 21,22,23,24,25,26,27,28,29,30, 31,32,33,34,35,36,37,38,39,40, 41,42,43,44,45,46,47,48,49,50, 51,52,53,54,55,56,57,58,59,60, 61,62,63,64,65,66,67,68,69,70, 71,72,73,74,75,76,77,78,79,80, 81,82,83,84,85,86,87,88,89,90, 91,92,93,94,95,96,97,98,99,-100}

// 32 bit long long type
//#define FIXEDPT_BITS 32

// 64 bit long long type
#define FIXEDPT_BITS 64

// 64 bits
#define FIXEDPT_WBITS  50
#define FIXEDPT_FBITS  14


#define TYPE unsigned long long

#define I2F(a) (((TYPE)a)<<FIXEDPT_FBITS)
#define F2I(a) (((TYPE)a)>>FIXEDPT_FBITS)


inline TYPE mul(TYPE A, TYPE B)
{
    A = I2F(A);
    B = I2F(B);
	TYPE res = (((TYPE)A * (TYPE)B) >> FIXEDPT_FBITS);

    //printf("mul = a*b = %lld*%lld = %lld\n", F2I(A), F2I(B), F2I(res));

    //printf("mul = a*b = %lld*%lld = %lld\n", F2I(A), B, res);
    //assert(res != 0);
    return res;
}

inline TYPE div(TYPE A, TYPE B)
{
    TYPE res = (((TYPE)A << FIXEDPT_FBITS) / (TYPE)B);
    //printf("div = a/b = %lld/%lld = %lld\n", A, B, F2I(res));
    //assert(B != 0);
    //assert(res != 0);

    return res;
}


volatile TYPE a[N] = INITIALIZE;
volatile TYPE b[N] = INITIALIZE;
volatile TYPE c[N] = INITIALIZE;
volatile TYPE d[N] = INITIALIZE;
volatile TYPE e[N] = INITIALIZE;
volatile TYPE f[N] = INITIALIZE;
volatile TYPE g[N] = INITIALIZE;
volatile TYPE h[N] = INITIALIZE;
volatile TYPE ii[N] =INITIALIZE;;
volatile TYPE j[N] = INITIALIZE;
volatile TYPE k[N] = INITIALIZE;
volatile TYPE l[N] = INITIALIZE;
volatile TYPE m[N] = INITIALIZE;
volatile TYPE n[N] = INITIALIZE;
volatile TYPE o[N] = INITIALIZE;
volatile TYPE p[N] = INITIALIZE;
volatile TYPE q[N] = INITIALIZE;
volatile TYPE r[N] = INITIALIZE;
volatile TYPE s[N] = INITIALIZE;
volatile TYPE t[N] = INITIALIZE;
volatile TYPE u[N] = INITIALIZE;
volatile TYPE v[N] = INITIALIZE;
volatile TYPE w[N] = INITIALIZE;
volatile TYPE x[N] = INITIALIZE;
volatile TYPE y[N] = INITIALIZE;
volatile TYPE z[N] = INITIALIZE;
volatile TYPE aa[N] = INITIALIZE;
volatile TYPE bb[N] = INITIALIZE;
volatile TYPE cc[N] = INITIALIZE;
volatile TYPE dd[N] = INITIALIZE;
volatile TYPE ee[N] = INITIALIZE;

// Simple accumulation
int main() {
    TYPE sum = 0;
    TYPE sum2 = 0;
    int i;
loop: for (i = 0; i < N-15; i++) {

    TYPE sum11, sum12, sum13, sum14, sum15, sum16, sum17, sum18, sum19, sum110, sum111, sum112, sum113, sum114;
    TYPE sum21, sum22, sum23, sum24, sum25, sum26, sum27;
    TYPE sum31, sum32, sum33, sum34;
    TYPE sum41, sum42;
    TYPE sum51;

//      TYPE num = 100;
//      TYPE den = 3;
//      printf("test:\n");
//      TYPE q = div(num, den);
//      q = F2I(q);
//      printf("test = 100/3 = %lld/%lld = %lld\n", num, den, q);
//      q = mul(num, den);
//      q = F2I(q);
//      printf("test = 100*3 = %lld*%lld = %lld\n", num, den, q);

    // level 1
    //sum11 = dd[i-1] + dd[i-2];
    sum11 = div(dd[i+14], dd[i+14]>>1);
    sum12 = mul(b[i+2], b[i+3]);

    sum13 = mul(c[i+4], c[i+5]);
    sum14 = mul(d[i+6], d[i+7]);

    sum15 = div(e[i], e[i+1]);
    sum16 = f[i+2] + f[i+3];

    sum17 = g[i] + g[i+1];
    sum18 = h[i+2] - h[i+3];

    sum19 = ii[i];// + ii[i+1];
    sum110 = j[i+2] - j[i+3];

    sum111 = 0; //u[i];// + v[i];
    sum112 = 0;//w[i];// + x[i];

    sum113 = 0;//n[i+4];// + z[i];
    sum114 = 0;//n[i+5];// + bb[i];

    // level 2
    sum21 = mul(sum11, sum12);
    sum22 = sum13 + sum14;
    sum23 = mul(sum15, sum16);
    sum24 = sum17 + sum18;
    sum25 = sum19 + sum110;
    sum26 = sum111 - sum112;
    sum27 = sum113 + sum114;

    // level 3
    sum31 = sum21 - sum22;
    sum32 = mul(sum23, sum24);
    sum33 = sum25 + sum26;
    sum34 = sum27;
    bb[i] = sum31;

    // level 4
    sum41 = sum31 + sum32;
    sum42 = sum33 + sum34;
    aa[i] = sum41;

    // level 5
    sum51 = sum41 + sum42;

    cc[i] = sum51;


    //sum = sum51 + sum + tmp;
    dd[i+9] = a[i] + F2I(mul(sum51, dd[i]));// + tmp;

}
    sum = dd[i+5];
    printf("sum = %lld\n", sum);

    if (sum == 40288495459LL) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }

    return sum;
}
