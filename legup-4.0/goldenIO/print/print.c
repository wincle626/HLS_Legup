#include<stdio.h>

int print (char ch, char* s, short a[], int size,  unsigned short b, int c, unsigned int d, long e, unsigned long f, long long g, unsigned long long h[], int size2, float flt_test, double dbl_test){
	printf ("%c, %s, %hd, %hd, %hd, %hu, %d, %u, %ld, %lu, %lld, %llu, %f, %f\n", ch, s, a[0], a[1], a[2], b, c, d, e, f, g, h[size2], flt_test, dbl_test);

	return c;
}
