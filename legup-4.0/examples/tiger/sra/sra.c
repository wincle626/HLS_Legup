#include <stdio.h>

// sqrt(52^2 + 84^2) = 98.79
// This should be approximated as 100
int inData[] = {52, 84};

int abs(int a) {
    return ( ((a) < 0) ? -(a) : (a) );
}

int min(int a, int b) {
    return ( ((a) < (b)) ? (a) : (b) );
}

int max(int a, int b) {
    return ( ((a) > (b)) ? (a) : (b) );
}

int sra(int a, int b) {
    int x = max(abs(a), abs(b));
    int y = min(abs(a), abs(b));
    int sqrt = max(x, x-(x>>3)+(y>>1));

    printf("%d\n", sqrt);
    return sqrt;
}

// square-root approximation:
int main() {
    int a = inData[0];
    int b = inData[1];

    return sra(a, b);
}

