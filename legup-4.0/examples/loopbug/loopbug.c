#include <stdio.h>

typedef unsigned long long float64;
int float_exception_flags = 0;

int extractFloat64Frac (float64 a)
{
    return 0;
}

static float64 roundAndPackFloat64 (int zSign, int zExp, int zSig)
{
    if (zExp < 0 && (zSig & 0x3FF))
        float_exception_flags |= 1;
    return 0;
}

static float64 addFloat64Sigs (float64 a, float64 b, int zSign)
{
    return roundAndPackFloat64 (a, b, zSign);
}

int extractFloat64Sign (float64 a) { 
    return a >> 63;
}

void float64_add (float64 a)
{
    int aSign = extractFloat64Sign (a);
    addFloat64Sigs (a, a, aSign);
}

int main ()
{
    int i;
    volatile int index = 0;
    volatile float64 x1 = 0;

    for (i = 0; i < 1000; i++)
    {
        printf("i = %d\nindex = %d\n", i, index);
        float64_add (x1);

        if (index > 3) break;
        index++;
    }
    int result = index + i;
    printf("Result: %d\n", result);
    if (result == 8) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return result;
}
