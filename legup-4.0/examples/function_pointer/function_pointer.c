#include <stdio.h>

int a(int i) { return 3; }
int b(int i) { volatile int j = 7; j += i; return j; }

int main() {
	volatile int i = 3;
	int (*fp)(int);
	
	if (i == 3) fp = &a;	// choose a
	else	   fp = &b;
	
	fp(4);
	
	if (i != 3) fp = &a;
	else	   fp = &b;		// choose b
	
	fp(5);
	
	return i;
}
