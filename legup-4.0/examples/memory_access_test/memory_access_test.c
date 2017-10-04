//Performs read/write memory accesses from/to local and global memory
//Sum should always be 136, and final output should be 0
#include <stdio.h>

int global[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
int sum_global = 0;

int add ()
{
	volatile int local[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
	int i=0;
	int sum_local=0;
	int result=0;
	printf("\nAll results should equal 136\n");

	printf("Writing sum to local memory\n");
	for (i=0; i<16; i+=4)
	{
		sum_local += local[i] + local[i+1] + local[i+2] + local[i+3];
	}
	result += (sum_local == 136);
	printf("local + local + local + local = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += local[i] + local[i+1] + local[i+2] + global[i+3];
	}
	result += (sum_local == 136);
	printf("local + local + local + global = %d\n", sum_local);
	
	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += local[i] + local[i+1] + global[i+2] + local[i+3];
	}
	result += (sum_local == 136);
	printf("local + local + global + local = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += local[i] + local[i+1] + global[i+2] + global[i+3];
	}
	result += (sum_local == 136);
	printf("local + local + global + global = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += local[i] + global[i+1] + local[i+2] + local[i+3];
	}
	result += (sum_local == 136);
	printf("local + global + local + local = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += local[i] + global[i+1] + local[i+2] + global[i+3];
	}
	result += (sum_local == 136);
	printf("local + global + local + global = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += local[i] + global[i+1] + global[i+2] + local[i+3];
	}
	result += (sum_local == 136);
	printf("local + global + global + local = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += local[i] + global[i+1] + global[i+2] + global[i+3];
	}
	result += (sum_local == 136);
	printf("local + global + global + global = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += global[i] + local[i+1] + local[i+2] + local[i+3];
	}
	result += (sum_local == 136);
	printf("global + local + local + local = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += global[i] + local[i+1] + local[i+2] + global[i+3];
	}
	result += (sum_local == 136);
	printf("global + local + local + global = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += global[i] + local[i+1] + global[i+2] + local[i+3];
	}
	result += (sum_local == 136);
	printf("global + local + global + local = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += global[i] + local[i+1] + global[i+2] + global[i+3];
	}
	result += (sum_local == 136);
	printf("global + local + global + global = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += global[i] + global[i+1] + local[i+2] + local[i+3];
	}
	result += (sum_local == 136);
	printf("global + global + local + local = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += global[i] + global[i+1] + local[i+2] + global[i+3];
	}
	result += (sum_local == 136);
	printf("global + global + local + global = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += global[i] + global[i+1] + global[i+2] + local[i+3];
	}
	result += (sum_local == 136);
	printf("global + global + global + local = %d\n", sum_local);

	sum_local = 0;
	for (i=0; i<16; i+=4)
	{
		sum_local += global[i] + global[i+1] + global[i+2] + global[i+3];
	}
	result += (sum_local == 136);
	printf("global + global + global + global = %d\n", sum_local);



	printf("\nWriting sum to global memory\n");

	for (i=0; i<16; i+=4)
	{
		sum_global += local[i] + local[i+1] + local[i+2] + local[i+3];
	}
	result += (sum_global == 136);
	printf("local + local + local + local = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += local[i] + local[i+1] + local[i+2] + global[i+3];
	}
	result += (sum_global == 136);
	printf("local + local + local + global = %d\n", sum_global);
	
	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += local[i] + local[i+1] + global[i+2] + local[i+3];
	}
	result += (sum_global == 136);
	printf("local + local + global + local = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += local[i] + local[i+1] + global[i+2] + global[i+3];
	}
	result += (sum_global == 136);
	printf("local + local + global + global = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += local[i] + global[i+1] + local[i+2] + local[i+3];
	}
	result += (sum_global == 136);
	printf("local + global + local + local = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += local[i] + global[i+1] + local[i+2] + global[i+3];
	}
	result += (sum_global == 136);
	printf("local + global + local + global = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += local[i] + global[i+1] + global[i+2] + local[i+3];
	}
	result += (sum_global == 136);
	printf("local + global + global + local = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += local[i] + global[i+1] + global[i+2] + global[i+3];
	}
	result += (sum_global == 136);
	printf("local + global + global + global = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += global[i] + local[i+1] + local[i+2] + local[i+3];
	}
	result += (sum_global == 136);
	printf("global + local + local + local = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += global[i] + local[i+1] + local[i+2] + global[i+3];
	}
	result += (sum_global == 136);
	printf("global + local + local + global = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += global[i] + local[i+1] + global[i+2] + local[i+3];
	}
	result += (sum_global == 136);
	printf("global + local + global + local = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += global[i] + local[i+1] + global[i+2] + global[i+3];
	}
	result += (sum_global == 136);
	printf("global + local + global + global = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += global[i] + global[i+1] + local[i+2] + local[i+3];
	}
	result += (sum_global == 136);
	printf("global + global + local + local = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += global[i] + global[i+1] + local[i+2] + global[i+3];
	}
	result += (sum_global == 136);
	printf("global + global + local + global = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += global[i] + global[i+1] + global[i+2] + local[i+3];
	}
	result += (sum_global == 136);
	printf("global + global + global + local = %d\n", sum_global);

	sum_global = 0;
	for (i=0; i<16; i+=4)
	{
		sum_global += global[i] + global[i+1] + global[i+2] + global[i+3];
	}
	result += (sum_global == 136);
	printf("global + global + global + global = %d\n", sum_global);


	return result;
}

int main()
{
	int main_result = 0;
	main_result = add ();	

    printf("Result: %d\n", main_result);
    if (main_result == 32) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return main_result;
}
