#include <stdio.h>

int a(int);
int b(int);
int c(int);

int a(int g) {
	volatile int x = 2;
	return b(g + x);
}

int b(int g) {
	volatile int x = 7;
	return c(g + x);
}

int c(int g) {
	volatile int x = 12;
	return g + x;
}

int main() {
	int i;
	int x = 0;

	for (i = 0; i<1; i++) {
		x += a(i);
		x += b(i);
	}
	printf("Result: %d\n", x);
    if (x == 40) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
	return x;
}
