#include <stdio.h>

#define abs(a) ( ((a) < 0) ? -(a) : (a) )
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )

int inData[] = {52, 84};

// square-root approximation:
int main() {

    // sqrt(52^2 + 84^2) = 98.79
    // This should be approximated as 100
    int a = inData[0];
    int b = inData[1];

    int x = max(abs(a), abs(b));
    int y = min(abs(a), abs(b));
    int sqrt = max(x, x-(x>>3)+(y>>1));

    printf("Result: %d\n", sqrt);
    if (sqrt == 100) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return sqrt;
}

