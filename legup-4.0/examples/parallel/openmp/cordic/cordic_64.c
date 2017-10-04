//Cordic in 32 bit signed fixed point math
//Function is valid for arguments in range -pi/2 -- pi/2
//for values pi/2--pi: value = half_pi-(theta-half_pi) and similarly for values -pi---pi/2
//
// 1.0 = 1073741824
// 1/k = 0.6072529350088812561694
// pi = 3.1415926536897932384626
//Constants
#include <stdio.h>
#include <math.h> // for testing only!
#include "cordic.h"


//Cordic in 64 bit signed fixed point math
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
#define CORDIC_NTAB 64
int cordic_ctab [] = {0x3243F6A8, 0x1DAC6705, 0x0FADBAFC, 0x07F56EA6, 0x03FEAB76, 0x01FFD55B, 0x00FFFAAA, 0x007FFF55, 0x003FFFEA, 0x001FFFFD, 0x000FFFFF, 0x0007FFFF, 0x0003FFFF, 0x0001FFFF, 0x0000FFFF, 0x00007FFF, 0x00003FFF, 0x00001FFF, 0x00000FFF, 0x000007FF, 0x000003FF, 0x000001FF, 0x000000FF, 0x0000007F, 0x0000003F, 0x0000001F, 0x0000000F, 0x00000008, 0x00000004, 0x00000002, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, };

int s[100], c[100];
int cordic(long long theta, int i)
//int cordic(int theta, int *s, int *c)
{
  int k, d, tx, ty, tz;
//	printf("%d, \n", theta);
  long long x=cordic_1K,y=0,z=theta;
 // n = (n>CORDIC_NTAB) ? CORDIC_NTAB : n;
  for (k=0; k<CORDIC_NTAB; ++k)
  {
    d = z>>63;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k) ^ d) - d);
    ty = y + (((x>>k) ^ d) - d);
    tz = z - ((cordic_ctab[k] ^ d) - d);
    x = tx; y = ty; z = tz;
  }  
 //*c = x; *s = y;
 c[i] = x; s[i] = y;
  return x+y;
}


/*
#define cordic_1K 0x26DD3B6A
#define half_pi 0x6487ED51
#define M_PI 3.1415926536897932384626
#define MUL 1073741824.000000
#define CORDIC_NTAB 32
const int cordic_ctab [] = {0x3243F6A8, 0x1DAC6705, 0x0FADBAFC, 0x07F56EA6, 0x03FEAB76, 0x01FFD55B, 
0x00FFFAAA, 0x007FFF55, 0x003FFFEA, 0x001FFFFD, 0x000FFFFF, 0x0007FFFF, 0x0003FFFF, 
0x0001FFFF, 0x0000FFFF, 0x00007FFF, 0x00003FFF, 0x00001FFF, 0x00000FFF, 0x000007FF, 
0x000003FF, 0x000001FF, 0x000000FF, 0x0000007F, 0x0000003F, 0x0000001F, 0x0000000F, 
0x00000008, 0x00000004, 0x00000002, 0x00000001, 0x00000000, };

int s[100], c[100];
//void cordic(int theta, int *s, int *c, int n)
int cordic(int theta, int i)
{
	//printf("theta = %d, \n", theta);
  int k, d, tx, ty, tz;
  int x=cordic_1K,y=0,z=theta;
  //n = (n>CORDIC_NTAB) ? CORDIC_NTAB : n;
  //for (k=0; k<n; ++k)
  for (k=0; k<CORDIC_NTAB; ++k)
  {
    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k) ^ d) - d);
    ty = y + (((x>>k) ^ d) - d);
    tz = z - ((cordic_ctab[k] ^ d) - d);
    x = tx; y = ty; z = tz;
  }  
 c[i] = x; s[i] = y;
	return x+y;
}
*/
/*
#define FRACT_BITS 16
#define FRACT_BITS_D2 8
#define FIXED_ONE (1 << FRACT_BITS)
#define INT2FIXED(x) ((x) << FRACT_BITS)
#define FLOAT2FIXED(x) ((int)((x) * (1 << FRACT_BITS))) 
#define FIXED2INT(x) ((x) >> FRACT_BITS)
#define FIXED2DOUBLE(x) (((double)(x)) / (1 << FRACT_BITS))
#define MULT(x, y) ( ((x) >> FRACT_BITS_D2) * ((y)>> FRACT_BITS_D2) )
*/

//Print out sin(x) vs fp CORDIC sin(x)
int main()
{
    double p;
    //long long p;
    int s,c,t;
    int i, sum=0;    
    //for(i=-50;i<=50;i++)
    for(i=0;i<=100;i++)
    {
        p = ((i-50)/50.0)*M_PI/2;        
		//int temp = MULT(FLOAT2FIXED((i/50.0)), FLOAT2FIXED((M_PI/2)));
		//printf("p = %lf, fixed p = %d, temp = %d, ", p, temp);
        //use 32 iterations
        //sum += cordic((p*MUL), &s, &c);
        sum += cordic(theta[i], i);
        //sum += cordic((p*MUL), i);
        //these values should be nearly equal (for testing)
        //printf("%f : %f, %f : %f\n", s/MUL, sin(p), c/MUL, cos(p));
    }     

	printf("sum = %d\n", sum);
	if (sum == -368571120) {
		printf("PASS\n");
	} else {
		printf("FAIL\n");
	}  
}
