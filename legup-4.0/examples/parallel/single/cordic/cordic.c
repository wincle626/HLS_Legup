#include <stdio.h>
//#include <math.h> // for testing only!
#include "cordic.h"

//Function is valid for arguments in range -pi/2 -- pi/2
//for values pi/2--pi: value = half_pi-(theta-half_pi) and similarly for values -pi---pi/2
//
// 1.0 = 1073741824
// 1/k = 0.6072529350088812561694
// pi = 3.1415926536897932384626
#define M_PI 3.1415926536897932384626

//8 bit fixed point//
//Constants
#define cordic_1K8 0x00000026
#define half_pi8 0x00000064
#define MUL8 64.000000
#define CORDIC_NTAB8 8
int cordic_ctab8 [] = {0x00000032, 0x0000001D, 0x0000000F, 0x00000007, 0x00000003, 0x00000001, 0x00000000, 0x00000000};

volatile int s8[100], c8[100];
int cordic8()
{
	int i, sum = 0;
	for(i=0;i<=100;i++)
	{
		int k, d, tx, ty, tz;
		int x=cordic_1K8,y=0, z=theta8[i];
		double p = ((i-50)/50.0)*M_PI/2; //for testing
		//z = (int)(p*MUL8); //for testing
		//printf("%d,\n", z);
		//for (k=0; k<n; ++k)
		for (k=0; k<CORDIC_NTAB8; ++k)
		{
			d = z>>7;
			//get sign. for other architectures, you might want to use the more portable version
			//d = z>=0 ? 0 : -1;
			tx = x - (((y>>k) ^ d) - d);
			ty = y + (((x>>k) ^ d) - d);
			tz = z - ((cordic_ctab8[k] ^ d) - d);
			x = tx; y = ty; z = tz;
		}
		s8[i] = (y);
		//s32[i] = (y/mul)*1000000;
		sum += (y);
		//these values should be nearly equal (for testing)
		//printf("%f : %f, %f : %f\n", y/MUL8, sin(p), x/MUL8, cos(p));
		//printf("%f : %f\n", y/MUL16, sin(p));
		//printf("%f : %f, %f : %f\n", (y/MUL)*10000000, sin(p), (x/MUL)*10000000, cos(p));
	} 
	return sum; 
}


//16 bit fixed point//
//Constants
#define cordic_1K16 0x000026DD
#define half_pi16 0x00006487
#define MUL16 16384.000000
#define CORDIC_NTAB16 16
int cordic_ctab16 [] = {0x00003243, 0x00001DAC, 0x00000FAD, 0x000007F5, 0x000003FE, 0x000001FF, 0x000000FF, 0x0000007F, 0x0000003F, 0x0000001F, 0x0000000F, 0x00000007, 0x00000003, 0x00000001, 0x00000000, 0x00000000, };

volatile int s16[100], c16[100];
int cordic16() 
{
	int i, sum = 0;
	for(i=0;i<=100;i++)
	{
		int k, d, tx, ty, tz, sine;
		int x=cordic_1K16,y=0,z=theta16[i];
		//n = (n>CORDIC_NTAB) ? CORDIC_NTAB : n;
		double p = ((i-50)/50.0)*M_PI/2; //for testing
		//z = (int)(p*MUL16); //for testing
		//printf("%d,\n", z);
		for (k=0; k<CORDIC_NTAB16; ++k)
		{
			d = z>>15;
			//get sign. for other architectures, you might want to use the more portable version
			//d = z>=0 ? 0 : -1;
			tx = x - (((y>>k) ^ d) - d);
			ty = y + (((x>>k) ^ d) - d);
			tz = z - ((cordic_ctab16[k] ^ d) - d);
			x = tx; y = ty; z = tz;
		}  
		//*c = x; *s = y;
		s16[i] = (y); //to make integer
		//s32[i] = (y/mul)*1000000;
		sum += (y);
		//these values should be nearly equal (for testing)
		//printf("%f : %f, %f : %f\n", y/MUL16, sin(p), x/MUL16, cos(p));
		//printf("%f : %f\n", y/MUL16, sin(p));
		//printf("%f : %f, %f : %f\n", (y/MUL)*10000000, sin(p), (x/MUL)*10000000, cos(p));
	}
	return sum;
}

//48 bit fixed point//
//Constants
#define cordic_1K48 0x000026DD
#define half_pi48 0x00006487
#define MUL48 16384.000000
#define CORDIC_NTAB48 48
int cordic_ctab48 [] = {0x00003243, 0x00001DAC, 0x00000FAD, 0x000007F5, 0x000003FE, 0x000001FF, 0x000000FF, 0x0000007F, 0x0000003F, 0x0000001F, 0x0000000F, 0x00000007, 0x00000003, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, };

volatile int s48[100], c48[100];

int cordic48()
{
	int i, sum = 0;
	for(i=0;i<=100;i++)
	{
		int k, d, tx, ty, tz, sine;
		long long x=cordic_1K48,y=0,z=theta48[i];
		double p = ((i-50)/50.0)*M_PI/2; //for testing
		//n = (n>CORDIC_NTAB) ? CORDIC_NTAB : n;
		//z = (int)(p*MUL16); //for testing
		//printf("%lld,\n", z);
		//for (k=0; k<n; ++k)
		for (k=0; k<CORDIC_NTAB48; ++k)
		{
			d = z>>47;
			//get sign. for other architectures, you might want to use the more portable version
			//d = z>=0 ? 0 : -1;
			tx = x - (((y>>k) ^ d) - d);
			ty = y + (((x>>k) ^ d) - d);
			tz = z - ((cordic_ctab48[k] ^ d) - d);
			x = tx; y = ty; z = tz;
		}  

		s48[i] = (y); //to make integer
		//s32[i] = (y/MUL)*1000000;
		sum += (y);
		//these values should be nearly equal (for testing)
		//printf("%f : %f, %f : %f\n", y/MUL48, sin(p), x/MUL48, cos(p));
		//printf("%f : %f, %f : %f\n", (y/MUL)*10000000, sin(p), (x/MUL)*10000000, cos(p));
	} 
	//printf("cordic32 sin sum = %d\n", sum); 
	return sum;
}


//32 bit fixed point//
#define cordic_1K 0x26DD3B6A
#define half_pi 0x6487ED51

#define MUL32 1073741824.000000
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
	int i, sum = 0;
	for(i=0;i<=100;i++)
	{
		int k, d, tx, ty, tz, sine;
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
		s32[i] = (y);//to make integer
		//s32[i] = (y/MUL)*1000000;
		sum += (y);
		//these values should be nearly equal (for testing)
		//printf("%f : %f, %f : %f\n", y/MUL32, sin(p), x/MUL32, cos(p));
		//printf("%f : %f, %f : %f\n", (y/MUL)*10000000, sin(p), (x/MUL)*10000000, cos(p));
	} 
	//printf("cordic32 sin sum = %d\n", sum); 
	return sum;
}

//64 bit fixed point//
#define MUL64 1073741824.000000
#define CORDIC_NTAB64 64
const int cordic_ctab64 [] = {0x3243F6A8, 0x1DAC6705, 0x0FADBAFC, 0x07F56EA6, 0x03FEAB76, 0x01FFD55B, 0x00FFFAAA, 0x007FFF55, 0x003FFFEA, 0x001FFFFD, 0x000FFFFF, 0x0007FFFF, 0x0003FFFF, 0x0001FFFF, 0x0000FFFF, 0x00007FFF, 0x00003FFF, 0x00001FFF, 0x00000FFF, 0x000007FF, 0x000003FF, 0x000001FF, 0x000000FF, 0x0000007F, 0x0000003F, 0x0000001F, 0x0000000F, 0x00000008, 0x00000004, 0x00000002, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, };

volatile int s64[100], c64[100];
//this function calculates the sin and cos of -pi/2 to pi/2
int cordic64()
{
	int i, sum = 0;
	for(i=0;i<=100;i++)
	{
		//p = ((i-50)/50.0)*M_PI/2;        
		int k, d, tx, ty, tz, sine;
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
		s64[i] = (y); //to make integer
		//s64[i] = (y/MUL)*1000000;
		sum += (y);
		//these values should be nearly equal (for testing)
		//printf("%f : %f, %f : %f\n", y/MUL64, sin(p), c/MUL64, cos(p));
	}    
	//printf("cordic64 sin sum = %d\n", sum); 
	return sum;
}

int cordic816() {
	int sum=0;
	sum += cordic8();
	sum += cordic16();
	return sum;
}

//#define PRINT_VALUES 1

//Print out sin(x) vs fp CORDIC sin(x)
int main()
{
    //double p;
    int sum=0;   

	sum += cordic64();
	sum += cordic48();
	sum += cordic32();
	sum += cordic16();
//	sum += cordic8();
//	sum += cordic816();
#ifdef PRINT_VALUES
	int i;
	for (i=0; i<= 100; i++) {
		//printf("sin16 =%f, sin32 = %f, sin48 = %f, sin64 = %f\n", s16[i]/MUL16, s32[i]/MUL32, s48[i]/MUL48, s64[i]/MUL64);
		//printf("sin16 =%f, sin32 = %f, sin8 = %f, sin64 = %f\n", s16[i]/MUL16, s32[i]/MUL32, s8[i]/MUL8, s64[i]/MUL64);
		printf("sin8= %f, sin16 =%f, sin32 = %f, sin48 = %f, sin64 = %f\n", s8[i]/MUL8, s16[i]/MUL16, s32[i]/MUL32, s48[i]/MUL48, s64[i]/MUL64);
	}
#endif
	printf("sum = %d\n", sum);
	if (sum == 214) {
	//if (sum == 186) {
//	if (sum == 252) {
		printf("PASS\n");
	} else {
		printf("FAIL\n");
	}  
}
