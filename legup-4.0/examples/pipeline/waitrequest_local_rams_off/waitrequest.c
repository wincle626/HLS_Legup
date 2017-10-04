#define SIZE 100
#define BINSIZE 100

#define SIZE2 1800
#define BINSIZE2 SIZE2

#include <stdio.h>
#include "waitrequest.h"


int hash1()
{
	int result1[BINSIZE2] = {0};
	//int result1[BINSIZE2];
	int i, a, collision=0;
loop1: for (i=0; i<SIZE2; i++) {
		a = input2[i];
		a = (a+0x7ed55d16) + (a<<12);
		a = (a^0xc761c23c) ^ (a>>19);
		a = (a+0x165667b1) + (a<<5);
		a = (a+0xd3a2646c) ^ (a<<9);
		a = (a+0xfd7046c5) + (a<<3);
		a = (a^0xb55a4f09) ^ (a>>16);
		a = (a<0) ? -1*a : a;	
		int hash = a%BINSIZE2;
		int temp = result1[hash]++;
        collision = (temp!=0) ? collision+1 : collision;
        //printf("i: %d collision: %d\n", i, collision);
	}
	return collision;
}

int hash3() 
{
    int result3[BINSIZE] = {0};
    int i, a, collision=0;
    loop2: for (i=0; i<SIZE; i++) {
       a = input[i];
       a = (a ^ 61) ^ (a >> 16);
       a = a + (a << 3);
       a = a ^ (a >> 4);
       a = a * 0x27d4eb2d;
       a = a ^ (a >> 15);
       a = (a<0) ? -1*a : a;
       int hash = a%BINSIZE;
       //printf("hash = %d\n", hash);
       // the array result3[] has a loop carried dependency
       int temp = result3[hash];
       collision = (temp!=0) ? collision+1 : collision;
       //printf("collision = %d\n", collision);
       result3[hash] = temp + 1;
   }
   return collision;
}

int main() {
    int i, j;
    int collision=0;
    int collision1=0;

    collision = hash1() + hash3();

    printf("Collision: %d\n", collision);
    if (collision == 1127) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }
    return collision;
}
