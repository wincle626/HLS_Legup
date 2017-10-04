#include <stdio.h>

#define SIZE 500
#include "hash.h"

//#define PRINT_RESULT
//#define CHECK_COLLISION
//#define CHECK_RESULT

int result1[SIZE];
int result2[SIZE];
int result3[SIZE];
int result4[SIZE];

#define BINSIZE 500

int hash1()
{
	int result[BINSIZE] = {0};
	int i, a, collision=0;
	for (i=0; i<SIZE; i++) {
		a = input[i];
		a = (a+0x7ed55d16) + (a<<12);
		a = (a^0xc761c23c) ^ (a>>19);
		a = (a+0x165667b1) + (a<<5);
		a = (a+0xd3a2646c) ^ (a<<9);
		a = (a+0xfd7046c5) + (a<<3);
		a = (a^0xb55a4f09) ^ (a>>16);
		a = (a<0) ? -1*a : a;		
		int hash = a%BINSIZE;
		int temp = result[hash]++;
        collision = (temp!=0) ? collision+1 : collision;
	}
    return collision;
}

int hash2()
{
	int result[BINSIZE] = {0};
	int i, a, collision=0;
	for (i=0; i<SIZE; i++) {
		a = input[i];
		a -= (a<<6);
		a ^= (a>>17);
		a -= (a<<9);
		a ^= (a<<4);
		a -= (a<<3);
		a ^= (a<<10);
		a ^= (a>>15);
		a = (a<0) ? -1*a : a;		
		int hash = a%BINSIZE;
		int temp = result[hash]++;
        collision = (temp!=0) ? collision+1 : collision;
	}
    return collision;
}

int hash3() {
	int result[BINSIZE] = {0};
	int i, a, collision=0;
	for (i=0; i<SIZE; i++) {
		a = input[i];
	    a = (a ^ 61) ^ (a >> 16);
		a = a + (a << 3);
	    a = a ^ (a >> 4);
	    a = a * 0x27d4eb2d;
	    a = a ^ (a >> 15);
		a = (a<0) ? -1*a : a;		
		int hash = a%BINSIZE;
		int temp = result[hash]++;
        collision = (temp!=0) ? collision+1 : collision;
	}
    return collision;
}

int hash4()
{
	int result[BINSIZE] = {0};
	int i, a, collision=0;
	for (i=0; i<SIZE; i++) {
		a = input[i];
	    a = (a+0x479ab41d) + (a<<8);
	    a = (a^0xe4aa10ce) ^ (a>>5);
	    a = (a+0x9942f0a6) - (a<<14);
	    a = (a^0x5aedd67d) ^ (a>>3);
	    a = (a+0x17bea992) + (a<<7);
		a = (a<0) ? -1*a : a;		
		int hash = a%BINSIZE;
		int temp = result[hash]++;
        collision = (temp!=0) ? collision+1 : collision;
	}
    return collision;
}

#define CHECK_RESULT
int main() {
	int i, j;
	int collision1=0, collision2=0, collision3=0, collision4=0;
	collision1 = hash1();
	collision2 = hash2();
	collision3 = hash3();
	collision4 = hash4();
	
#ifdef PRINT_RESULT
	printf("result1\n");
	for (i=0; i<SIZE; i++) {
		printf("%d,\n", result1[i]);
	}
	printf("result2\n");
	for (i=0; i<SIZE; i++) {
		printf("%d,\n", result2[i]);
	}
	printf("result3\n");
	for (i=0; i<SIZE; i++) {
		printf("%d,\n", result3[i]);
	}
	printf("result4\n");
	for (i=0; i<SIZE; i++) {
		printf("%d,\n", result4[i]);
	}
#endif
#ifdef CHECK_COLLISION
	for (i=0; i<SIZE; i++) {
		for (j=i+1; j<SIZE; j++) {
			if (result1[i] == result1[j]) {
				collision1++;
			}
			if (result2[i] == result2[j]) collision2++;
			if (result3[i] == result3[j]) collision3++;
			if (result4[i] == result4[j]) collision4++;
		}
	}

	printf("collision1 = %d, collision2 = %d, collision3 = %d, collision4 = %d\n", collision1, collision2, collision3, collision4);
#endif	
#ifdef CHECK_RESULT
	int count=0;
    count += (collision1 == 178);
    count += (collision2 == 182);
    count += (collision3 == 196);
    count += (collision4 == 179);
	printf("result = %d\n", count);
	if (count == 4) {
		printf("PASS\n");
	}
	else {
		printf("FAIL\n");
	}
#endif
	return 0;
}
