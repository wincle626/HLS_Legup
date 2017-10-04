#include <stdio.h>

#define N 100
#define INITIALIZE {1,2,3,4,5,6,7,8,9,10, 11,12,13,14,15,16,17,18,19,20, 21,22,23,24,25,26,27,28,29,30, 31,32,33,34,35,36,37,38,39,40, 41,42,43,44,45,46,47,48,49,50, 51,52,53,54,55,56,57,58,59,60, 61,62,63,64,65,66,67,68,69,70, 71,72,73,74,75,76,77,78,79,80, 81,82,83,84,85,86,87,88,89,90, 91,92,93,94,95,96,97,98,99,-100}

volatile int a[N] = INITIALIZE;
volatile int b[N] = INITIALIZE;
volatile int c[N] = INITIALIZE;
volatile int d[N] = INITIALIZE;
volatile int e[N] = INITIALIZE;
volatile int f[N] = INITIALIZE;
volatile int g[N] = INITIALIZE;
volatile int h[N] = INITIALIZE;
volatile int ii[N] =INITIALIZE;;
volatile int j[N] = INITIALIZE;
volatile int k[N] = INITIALIZE;
volatile int l[N] = INITIALIZE;
volatile int m[N] = INITIALIZE;
volatile int n[N] = INITIALIZE;
volatile int o[N] = INITIALIZE;
volatile int p[N] = INITIALIZE;
volatile int q[N] = INITIALIZE;
volatile int r[N] = INITIALIZE;
volatile int s[N] = INITIALIZE;
volatile int t[N] = INITIALIZE;
volatile int u[N] = INITIALIZE;
volatile int v[N] = INITIALIZE;
volatile int w[N] = INITIALIZE;
volatile int x[N] = INITIALIZE;
volatile int y[N] = INITIALIZE;
volatile int z[N] = INITIALIZE;
volatile int aa[N] = INITIALIZE;
volatile int bb[N] = INITIALIZE;
volatile int cc[N] = INITIALIZE;
volatile int dd[N] = INITIALIZE;

// Simple accumulation
int main() {
    int sum = 0;
    int sum2 = 0;
    int i;
loop: for (i = 0; i < N-10; i++) {

    int tmp = i;
    int sum11, sum12, sum13, sum14, sum15, sum16, sum17, sum18, sum19, sum110, sum111, sum112, sum113, sum114;
    int sum21, sum22, sum23, sum24, sum25, sum26, sum27;
    int sum31, sum32, sum33, sum34;
    int sum41, sum42;
    int sum51;

    // level 1
    //sum11 = dd[i-1] + dd[i-2];
    sum11 = dd[i+9] + dd[i+8];
    sum12 = b[i+2] + b[i+3];

    sum13 = c[i+4] + c[i+5];
    sum14 = d[i+6] + d[i+7];

    sum15 = e[i] + e[i+1];
    sum16 = f[i+2] + f[i+3];

    sum17 = g[i] + g[i+1];
    sum18 = h[i+2] + h[i+3];

    sum19 = ii[i];// + ii[i+1];
    sum110 = j[i+2] + j[i+3];

    sum111 = 0; //u[i];// + v[i];
    sum112 = 0;//w[i];// + x[i];

    sum113 = 0;//n[i+4];// + z[i];
    sum114 = 0;//n[i+5];// + bb[i];

    // level 2
    sum21 = sum11 + sum12;
    sum22 = sum13 + sum14;
    sum23 = sum15 + sum16;
    sum24 = sum17 + sum18;
    sum25 = sum19 + sum110;
    sum26 = sum111 + sum112;
    sum27 = sum113 + sum114;

    // level 3
    sum31 = sum21 + sum22;
    sum32 = sum23 + sum24;
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
    dd[i+1] = sum51 + dd[i] + tmp;

}
    sum = dd[i];
    printf("sum = %d\n", sum);

    if (sum == 87301) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }

    return sum;
}
