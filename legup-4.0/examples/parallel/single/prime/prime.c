#define SIZE 5000
#define NUM_ACCEL 4
#define OPS_PER_ACCEL SIZE/NUM_ACCEL
#include <stdio.h>
#include "hash.h"

int prime() {
	for (i=2; i<=num; i++)
	{
 
		if (num % i == 0)
		{
		    printf("%d is a factor of %d\n", i, num);
			numFactors++;
		}
	}
	if (numFactors == 1)
        printf("The number %d is prime\n", num);
	else
        printf("The number %d is not a prime\n", num);
	return 0;
    
}

int main() {
     	
	int num, i, numFactors = 0;
	printf("Enter a number: ");
	scanf("%d", &num);
	printf("The number is %d\n", num);
	for (i=2; i<=num; i++)
	{
 
		if (num % i == 0)
		{
		    printf("%d is a factor of %d\n", i, num);
			numFactors++;
		}
	}
	if (numFactors == 1)
        printf("The number %d is prime\n", num);
	else
        printf("The number %d is not a prime\n", num);
	return 0;
}
