/* Fixed point 16-bit input/output in place FFT

This algorithm is capable of handling any FFT size that is a power of 2
as well as forward and inverse FFT. 

To avoid floating point numbers, sin values are stored as integers
with respect to 2^15 = 32768 rather than 1, and the length of the sin wave
(2pi) is 1024. E.g. sin(pi/2) = 1, so the 256'th value in the sin array
(256 corresponds to pi/2) is 32768, which corresponds to 1. 

This scaling also applies to the input. For example if input data is a 
sine wave with amplitude 327 (654 peak-to-peak), this is equivalent
to a sine input with amplitude 0.01. The output is also with respect to
0.01. If input values must exceed +/- 1 i.e. exceed 32768, they must be
first scaled down and then the output scaled up by that factor (given
an input vector x, fft(c*x) = c*fft(x), where c is a scaling constant).


Acknowledgements:
1. Fixed point method (multiply/sine-lookup/butterfly calculation):
   Tom Roberts, Malcolm Slaneyand, Dimitrios P. Bouras
2. Time decimation bit-reversal algorithm:
   Steven W. Smith, http://www.dspguide.com/ch12/2.htm               */


#include <stdio.h>
#include <stdlib.h>

#define Input_Size  64
#define inverse     0

#define Period      1024
#define log2_Period 10

// Lookup for butterfly calculations
short Sinewave[Period-Period/4] = {
      0,    201,    402,    603,    804,   1005,   1206,   1406,
   1607,   1808,   2009,   2209,   2410,   2610,   2811,   3011,
   3211,   3411,   3611,   3811,   4011,   4210,   4409,   4608,
   4807,   5006,   5205,   5403,   5601,   5799,   5997,   6195,
   6392,   6589,   6786,   6982,   7179,   7375,   7571,   7766,
   7961,   8156,   8351,   8545,   8739,   8932,   9126,   9319,
   9511,   9703,   9895,  10087,  10278,  10469,  10659,  10849,
  11038,  11227,  11416,  11604,  11792,  11980,  12166,  12353,
  12539,  12724,  12909,  13094,  13278,  13462,  13645,  13827,
  14009,  14191,  14372,  14552,  14732,  14911,  15090,  15268,
  15446,  15623,  15799,  15975,  16150,  16325,  16499,  16672,
  16845,  17017,  17189,  17360,  17530,  17699,  17868,  18036,
  18204,  18371,  18537,  18702,  18867,  19031,  19194,  19357,
  19519,  19680,  19840,  20000,  20159,  20317,  20474,  20631,
  20787,  20942,  21096,  21249,  21402,  21554,  21705,  21855,
  22004,  22153,  22301,  22448,  22594,  22739,  22883,  23027,
  23169,  23311,  23452,  23592,  23731,  23869,  24006,  24143,
  24278,  24413,  24546,  24679,  24811,  24942,  25072,  25201,
  25329,  25456,  25582,  25707,  25831,  25954,  26077,  26198,
  26318,  26437,  26556,  26673,  26789,  26905,  27019,  27132,
  27244,  27355,  27466,  27575,  27683,  27790,  27896,  28001,
  28105,  28208,  28309,  28410,  28510,  28608,  28706,  28802,
  28897,  28992,  29085,  29177,  29268,  29358,  29446,  29534,
  29621,  29706,  29790,  29873,  29955,  30036,  30116,  30195,
  30272,  30349,  30424,  30498,  30571,  30643,  30713,  30783,
  30851,  30918,  30984,  31049,  31113,  31175,  31236,  31297,
  31356,  31413,  31470,  31525,  31580,  31633,  31684,  31735,
  31785,  31833,  31880,  31926,  31970,  32014,  32056,  32097,
  32137,  32176,  32213,  32249,  32284,  32318,  32350,  32382,
  32412,  32441,  32468,  32495,  32520,  32544,  32567,  32588,
  32609,  32628,  32646,  32662,  32678,  32692,  32705,  32717,
  32727,  32736,  32744,  32751,  32757,  32761,  32764,  32766,
  32767,  32766,  32764,  32761,  32757,  32751,  32744,  32736,
  32727,  32717,  32705,  32692,  32678,  32662,  32646,  32628,
  32609,  32588,  32567,  32544,  32520,  32495,  32468,  32441,
  32412,  32382,  32350,  32318,  32284,  32249,  32213,  32176,
  32137,  32097,  32056,  32014,  31970,  31926,  31880,  31833,
  31785,  31735,  31684,  31633,  31580,  31525,  31470,  31413,
  31356,  31297,  31236,  31175,  31113,  31049,  30984,  30918,
  30851,  30783,  30713,  30643,  30571,  30498,  30424,  30349,
  30272,  30195,  30116,  30036,  29955,  29873,  29790,  29706,
  29621,  29534,  29446,  29358,  29268,  29177,  29085,  28992,
  28897,  28802,  28706,  28608,  28510,  28410,  28309,  28208,
  28105,  28001,  27896,  27790,  27683,  27575,  27466,  27355,
  27244,  27132,  27019,  26905,  26789,  26673,  26556,  26437,
  26318,  26198,  26077,  25954,  25831,  25707,  25582,  25456,
  25329,  25201,  25072,  24942,  24811,  24679,  24546,  24413,
  24278,  24143,  24006,  23869,  23731,  23592,  23452,  23311,
  23169,  23027,  22883,  22739,  22594,  22448,  22301,  22153,
  22004,  21855,  21705,  21554,  21402,  21249,  21096,  20942,
  20787,  20631,  20474,  20317,  20159,  20000,  19840,  19680,
  19519,  19357,  19194,  19031,  18867,  18702,  18537,  18371,
  18204,  18036,  17868,  17699,  17530,  17360,  17189,  17017,
  16845,  16672,  16499,  16325,  16150,  15975,  15799,  15623,
  15446,  15268,  15090,  14911,  14732,  14552,  14372,  14191,
  14009,  13827,  13645,  13462,  13278,  13094,  12909,  12724,
  12539,  12353,  12166,  11980,  11792,  11604,  11416,  11227,
  11038,  10849,  10659,  10469,  10278,  10087,   9895,   9703,
   9511,   9319,   9126,   8932,   8739,   8545,   8351,   8156,
   7961,   7766,   7571,   7375,   7179,   6982,   6786,   6589,
   6392,   6195,   5997,   5799,   5601,   5403,   5205,   5006,
   4807,   4608,   4409,   4210,   4011,   3811,   3611,   3411,
   3211,   3011,   2811,   2610,   2410,   2209,   2009,   1808,
   1607,   1406,   1206,   1005,    804,    603,    402,    201,
      0,   -201,   -402,   -603,   -804,  -1005,  -1206,  -1406,
  -1607,  -1808,  -2009,  -2209,  -2410,  -2610,  -2811,  -3011,
  -3211,  -3411,  -3611,  -3811,  -4011,  -4210,  -4409,  -4608,
  -4807,  -5006,  -5205,  -5403,  -5601,  -5799,  -5997,  -6195,
  -6392,  -6589,  -6786,  -6982,  -7179,  -7375,  -7571,  -7766,
  -7961,  -8156,  -8351,  -8545,  -8739,  -8932,  -9126,  -9319,
  -9511,  -9703,  -9895, -10087, -10278, -10469, -10659, -10849,
 -11038, -11227, -11416, -11604, -11792, -11980, -12166, -12353,
 -12539, -12724, -12909, -13094, -13278, -13462, -13645, -13827,
 -14009, -14191, -14372, -14552, -14732, -14911, -15090, -15268,
 -15446, -15623, -15799, -15975, -16150, -16325, -16499, -16672,
 -16845, -17017, -17189, -17360, -17530, -17699, -17868, -18036,
 -18204, -18371, -18537, -18702, -18867, -19031, -19194, -19357,
 -19519, -19680, -19840, -20000, -20159, -20317, -20474, -20631,
 -20787, -20942, -21096, -21249, -21402, -21554, -21705, -21855,
 -22004, -22153, -22301, -22448, -22594, -22739, -22883, -23027,
 -23169, -23311, -23452, -23592, -23731, -23869, -24006, -24143,
 -24278, -24413, -24546, -24679, -24811, -24942, -25072, -25201,
 -25329, -25456, -25582, -25707, -25831, -25954, -26077, -26198,
 -26318, -26437, -26556, -26673, -26789, -26905, -27019, -27132,
 -27244, -27355, -27466, -27575, -27683, -27790, -27896, -28001,
 -28105, -28208, -28309, -28410, -28510, -28608, -28706, -28802,
 -28897, -28992, -29085, -29177, -29268, -29358, -29446, -29534,
 -29621, -29706, -29790, -29873, -29955, -30036, -30116, -30195,
 -30272, -30349, -30424, -30498, -30571, -30643, -30713, -30783,
 -30851, -30918, -30984, -31049, -31113, -31175, -31236, -31297,
 -31356, -31413, -31470, -31525, -31580, -31633, -31684, -31735,
 -31785, -31833, -31880, -31926, -31970, -32014, -32056, -32097,
 -32137, -32176, -32213, -32249, -32284, -32318, -32350, -32382,
 -32412, -32441, -32468, -32495, -32520, -32544, -32567, -32588,
 -32609, -32628, -32646, -32662, -32678, -32692, -32705, -32717,
 -32727, -32736, -32744, -32751, -32757, -32761, -32764, -32766,
};

short FIX_MPY(short a, short b)
/* Because all data are integers with respect to 2^15 = 32768, following
   multiplication, products must be divided by 2^15, or right shifted
   by 15 bits. (E.g. consider an input value of 1 multiplied by sin(pi/2). 
   This corresponds to 32768 * 32768, and the product also must be 32768).

   Procedure:
   1. Cast a and b as integers and multiply
   2. Divide by 2^15 and return as a short
   3. To increase accuracy, first the product is divided by 2^14, the last
      bit is checked, and if = 1 then after the final division by 2, add 1 */
{
	int c = (int)a*(int)b;
	return (c >> 15) + ((c >> 14)&1);
}

// Reverses bits to obtain new index (for time decimation)
short new_index (short initial)
{
  short i=Input_Size, final=0;
  while (i > 1)
  {
    i = i >> 1;
    final += (initial&1)*i;
    initial = initial >> 1;
  }
  return final;
}


void fft(short old_Re[], short old_Im[], short Real [], short Imag [])
{
  short m, i, j, l, k, istep, shift,
  	    W_Imag, W_Real, // W = exp(-sqrt(-1)*2*pi/Input)Size
	      qi, qr, ti, tr; // Temporary coefficients

  // Decimation in Time, reverse bits to obtain new indices and swap elements
  Imag[0] = old_Im[0]; // 0th and last elements stay the same
  Real[0] = old_Re[0];
  Imag[Input_Size-1] = old_Im[Input_Size-1];
  Real[Input_Size-1] = old_Re[Input_Size-1];

  for (i=1; i<(Input_Size-1); i++)
  {
    j = new_index(i);
    Imag[i] = old_Im[j];
    Real[i] = old_Re[j];
  }
 
  /* Butterfly computation. See Summary.pdf. For N =  64 point FFT,
     there are log2(N) = 6 stages of butterflies, with N/2 = 32 butterflies
     per stage. In stage 1, every entry is butterflied with its neighbor:
     (the function B() denotes a butterfly computation)

     Stage 1: B(0,1), B(2,3) ... B(62,63).

     The results are  stored in-place (a butterfly is a 2-input 2-output operation).
     In stage 2, the butterfly is performed with every other entry:
     
     Stage 2: B(0,2), B(1,3), B(4,6), B(5,7) ... B(60,62), B(62,63).

     In the third stage, this pattern repeats but skips 4: 
     
     Stage 3: B(0,4), B(1,5), B(2,6), B(3,7),
                  B(8,12), B(9,13), B(10,14), B(11,15) ...

     The pattern repeats skipping 8, 16 and 32. This is accomplished using 2 nested for
     loops. The inner for loop is the actual butterfly computation, while the outer 
     creates the correct indices. The outermost  while loop counts the number of passes 
     (hence executes 6x in the 64 point example). */

  l = 1;
  k = log2_Period-1;
  while (l < Input_Size) // Executes log2(N) times
  {
    if (inverse)
      shift = Input_Size-1; // We must scale down the inverse FFT by 1/N (by its definition)
    else shift = 0;
		
    istep = l << 1;    
    for (m=0; m<l; m++) {
      j = m << k;
      W_Imag =  Sinewave[j+Period/4]; // Find the right sin value
      W_Real = Sinewave[j];    
      if (inverse)
        W_Real = -W_Real;
      if (shift) {
        W_Imag >>= 1;
        W_Real >>= 1;
      }
      for (i=m; i<Input_Size; i+=istep) { // Butterfly computation of index i and index j
        j = i + l; // l increases by 2 every stage, hence if we butterfly i and j this is
								   // first every other one, then skip 2, then skip 4, etc.
        ti = FIX_MPY(W_Imag,Imag[j]) - FIX_MPY(W_Real,Real[j]);
        tr = FIX_MPY(W_Imag,Real[j]) + FIX_MPY(W_Real,Imag[j]);
        qi = Imag[i];
        qr = Real[i];
        if (shift) {
          qi >>= 1;
          qr >>= 1;
        }
        Imag[j] = qi - ti;
        Real[j] = qr - tr;
        Imag[i] = qi + ti;
        Real[i] = qr + tr;
      }
    }
    k--;
    l = istep;
  }
}

int main()
{
  short i, Real[Input_Size], Imag[Input_Size], New_Real[Input_Size], New_Imag[Input_Size];
  int sum=0;

  // Set input, each input value consists of a Real and imaginary part
	for (i=0; i<Input_Size; i++){
	  Imag[i] = 0;
    Real[i] = 10*i;
  }

  fft(Real, Imag, New_Real, New_Imag);

//  for (i=0; i<Input_Size; i++)
//    printf ("%d\t\t%d\n", New_Real[i], New_Imag[i]);

  for (i=0; i<Input_Size; i++) {
    sum += New_Real[i];
    sum += New_Imag[i];
  }

  return sum; // This input was chosen because sum of outputs = 0
}

