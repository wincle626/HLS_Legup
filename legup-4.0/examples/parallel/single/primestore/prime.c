#define SIZE 100
#include <stdio.h>
#include "prime.h"

int result[SIZE];

int prime(int n) {
	int num, i, j, k;
	int isFactor=0, numFactors=0, isPrimeFactor=0, numPrimeFactors=0;
	int primeNumbers=0;
	//loop through each input
	for (i=1; i<=n; i++) {
		num = i;
        //if (num==0) continue;
	    numFactors = 0;
        //test each factor
		for (j=2; j<=num; j++) {
			isFactor = (num % j == 0);
            //printf("isFactor = %d\n", isFactor);
			numFactors = (isFactor==1)? numFactors+1 : numFactors;
			//to calculate if the factor is a prime number
			//if (isFactor)
			//	printf("%d is a factor of %d\n", j, num);
			//	for (k=2; k<=j; k++) {
			//		isPrimeFactor = (j % k == 0);
			//		numPrimeFactors = (isPrimeFactor)? numPrimeFactors+1 : numPrimeFactors;
			//	}
			//}
		}
        //accumulate the number of prime numbers
		primeNumbers = (numFactors==1)? primeNumbers+1 : primeNumbers; 
		result[num] = numFactors;
		if (numFactors == 1)
		    printf("The number %d is a prime number\n", num);
		else
		   printf("The number %d is not a prime number\n", num);
	}
	return primeNumbers;
}

int main() {
    legup_start_counter(0);
    int limit = SIZE;
    int sum = prime(limit);
    int perf_counter = legup_stop_counter(0);
    printf("perf_counter = %d\n", perf_counter);
	
	printf("Sum = %d\n", sum);
	if (sum == 25)
		printf("PASS\n");
	else 
		printf("FAIL\n");
	
	
	return 0;
}
