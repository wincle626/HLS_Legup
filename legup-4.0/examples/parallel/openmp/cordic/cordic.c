#include <stdio.h>
//#include <math.h> // for testing only!
#include "cordic.h"

#define OMP_ACCEL 4
//#define PRINT_VALUES 1

//Cordic in 32, 64 bit signed fixed point math
//Function is valid for arguments in range -pi/2 -- pi/2
//for values pi/2--pi: value = half_pi-(theta-half_pi) and similarly for values -pi---pi/2
//
// 1.0 = 1073741824
// 1/k = 0.6072529350088812561694
// pi = 3.1415926536897932384626
//Constants
#define cordic_1K 0x26DD3B6A
#define half_pi 0x6487ED51
#define M_PI 3.1415926536897932384626
#define MUL 1073741824.000000

#define CORDIC_NTAB32 32
const int cordic_ctab32 [] = {0x3243F6A8, 0x1DAC6705, 0x0FADBAFC, 0x07F56EA6, 0x03FEAB76, 0x01FFD55B, 
0x00FFFAAA, 0x007FFF55, 0x003FFFEA, 0x001FFFFD, 0x000FFFFF, 0x0007FFFF, 0x0003FFFF, 
0x0001FFFF, 0x0000FFFF, 0x00007FFF, 0x00003FFF, 0x00001FFF, 0x00000FFF, 0x000007FF, 
0x000003FF, 0x000001FF, 0x000000FF, 0x0000007F, 0x0000003F, 0x0000001F, 0x0000000F, 
0x00000008, 0x00000004, 0x00000002, 0x00000001, 0x00000000, };

volatile int s32[100], c32[100];
//void cordic(int theta, int *s, int *c, int n)
int cordic32()
{
	int i, tid, sum = 0;
	int temp[OMP_ACCEL] = {0, 0, 0, 0};
	#pragma omp parallel for num_threads(OMP_ACCEL) private(i, tid)
	for(i=0;i<=100;i++)
	{
		int k, d, tx, ty, tz;
		int x=cordic_1K,y=0,z=theta[i];
		double p = ((i-50)/50.0)*M_PI/2; //for testing
		//n = (n>CORDIC_NTAB) ? CORDIC_NTAB : n;
		for (k=0; k<CORDIC_NTAB32; ++k)
		{
			d = z>>31;
			//get sign. for other architectures, you might want to use the more portable version
			//d = z>=0 ? 0 : -1;
			tx = x - (((y>>k) ^ d) - d);
			ty = y + (((x>>k) ^ d) - d);
			tz = z - ((cordic_ctab32[k] ^ d) - d);
			x = tx; y = ty; z = tz;
		} 
		s32[i] = y;
		//s32[i] = (y/MUL)*1000000;
		tid = omp_get_thread_num();
		temp[tid] += y;
		//these values should be nearly equal (for testing)
		//printf("%f : %f, %f : %f\n", y/MUL, sin(p), x/MUL, cos(p));
		//printf("%f : %f, %f : %f\n", (y/MUL)*10000000, sin(p), (x/MUL)*10000000, cos(p));
	} 
	//printf("cordic32 sin sum = %d\n", sum); 
	for (i=0; i<OMP_ACCEL; i++) {
		sum += temp[i];	
	}
	return sum;
}

#define CORDIC_NTAB64 64
const int cordic_ctab64 [] = {0x3243F6A8, 0x1DAC6705, 0x0FADBAFC, 0x07F56EA6, 0x03FEAB76, 0x01FFD55B, 0x00FFFAAA, 0x007FFF55, 0x003FFFEA, 0x001FFFFD, 0x000FFFFF, 0x0007FFFF, 0x0003FFFF, 0x0001FFFF, 0x0000FFFF, 0x00007FFF, 0x00003FFF, 0x00001FFF, 0x00000FFF, 0x000007FF, 0x000003FF, 0x000001FF, 0x000000FF, 0x0000007F, 0x0000003F, 0x0000001F, 0x0000000F, 0x00000008, 0x00000004, 0x00000002, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, };

volatile int s64[100], c64[100];
//this function calculates the sin and cos of -pi/2 to pi/2
int cordic64()
{
	int i, tid, sum = 0;
	int temp[OMP_ACCEL] = {0, 0, 0, 0};
	#pragma omp parallel for num_threads(OMP_ACCEL) private(i, tid)
	for(i=0;i<=100;i++)
	{
		//p = ((i-50)/50.0)*M_PI/2;        
		int k, d, tx, ty, tz;
		long long x=cordic_1K,y=0,z=theta[i];
		// n = (n>CORDIC_NTAB) ? CORDIC_NTAB : n;
		for (k=0; k<CORDIC_NTAB64; ++k)
		{
			d = z>>63;
			//get sign. for other architectures, you might want to use the more portable version
			//d = z>=0 ? 0 : -1;
			tx = x - (((y>>k) ^ d) - d);
			ty = y + (((x>>k) ^ d) - d);
			tz = z - ((cordic_ctab64[k] ^ d) - d);
			x = tx; y = ty; z = tz;
		}  
		//*c = x; *s = y;
		//c[i] = x; s[i] = y;
		s64[i] = y;
		tid = omp_get_thread_num();
		temp[tid] += y;
		//these values should be nearly equal (for testing)
		//printf("%f : %f, %f : %f\n", s/MUL, sin(p), c/MUL, cos(p));
	}     

	for (i=0; i<OMP_ACCEL; i++) {
		sum += temp[i];	
	}
	return sum;
}

//Print out sin(x) vs fp CORDIC sin(x)
int main()
{
    //double p;
    int i, sum=0;   

	sum += cordic64();
	sum += cordic32();

#ifdef PRINT_VALUES
	int i;
	for (i=0; i<= 100; i++) {
		printf("sin32 = %f, sin64 = %f\n", s32[i], s64[i]);
	}
#endif
	printf("sum = %d\n", sum);
	if (sum == 114) {
		printf("PASS\n");
	} else {
		printf("FAIL\n");
	}  
}
