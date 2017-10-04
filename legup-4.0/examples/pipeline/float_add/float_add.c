#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)
#define fabs(a) (((a) >= 0.0f) ? (a) : -(a))

#include <assert.h>
#include <stdio.h>
#define N 8

float f_float(float a, float b){
        return (a+2*b) / (a*b) - a*1.5f;
}

int main(void)
{

        int k;
        volatile float a[N] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
        volatile float b[N] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
        float x = 0;

        loop: for(k=0; k < N; k++) {
            x += f_float(a[k], b[k]);
            printf("Value is %x\n", FLT2HEX(x));
        }

        float exp = 76.135719f;

        printf("Final value is %x\n", FLT2HEX(x));
        printf("Expected is %x\n", FLT2HEX(exp));

        float sub = x-exp;
        float diff = fabs(sub);
        printf("sub is %x\n", FLT2HEX(sub));
        printf("Diff is %x\n", FLT2HEX(diff));
        float eps = 1e-6f;
        printf("eps is %x\n", FLT2HEX(eps));

        int equal = diff <= 1e-6f;

        printf("equal is %x\n", equal);

        if (equal) {
            printf("PASSED\n");
            return 76;
        } else {
            printf("Value is %f\n", x);
            printf("FAILED\n");
            return 0;
        }
}
