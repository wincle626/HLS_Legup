#define SIZE 100
#define BINSIZE 100
#include <stdio.h>
#include "hash.h"

int hash3() 
{
    int result3[BINSIZE] = {0};
    int i, a, collision=0;
    loop: for (i=0; i<SIZE; i++) {
       a = input[i];
       a = (a ^ 61) ^ (a >> 16);
       a = a + (a << 3);
       a = a ^ (a >> 4);
       a = a * 0x27d4eb2d;
       a = a ^ (a >> 15);
       a = (a<0) ? -1*a : a;
       int hash = a%BINSIZE;
       printf("hash = %d\n", hash);
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

    collision = hash3();
    printf("collision = %d\n", collision);

    if (collision == 91) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }
    return collision;
}
