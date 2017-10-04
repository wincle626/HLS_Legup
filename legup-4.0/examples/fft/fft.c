/* Fixed point 16-bit input-output in place FFT

This is a modification of fft_general.c, working only for the 64 point FFT.
- Time decimation swaps are stored, not calculated
- Only required sine lookup values are stored, not all values
- Inputs made volatile to read one from memory per clock cycle 
- Inverse FFT is still possible

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

#define abs(a) ( ((a) < 0) ? -(a) : (a) )

// Lookup for butterfly calculations
short sin_lookup[126] = {32767,0,  
												32767,0,  0,32767,  
												32767,0,  23169,23169,  0,32767,  -23169,23169,  
												32767,0,  30272,12539,  23169,23169,  12539,30272,  0,32767,  -12539,30272,  -23169,23169,  -30272,12539,  
												32767,0,  32137,6392,  30272,12539,  27244,18204,  23169,23169,  18204,27244,  12539,30272,  6392,32137,  0,32767,  -6392,32137,  -12539,30272,  -18204,27244,  -23169,23169,  
													-27244,18204,  -30272,12539,  -32137,6392,  
												32767,0,  32609,3211,  32137,6392,  31356,9511,  30272,12539,  28897,15446,  27244,18204,  25329,20787,  23169,23169,  20787,25329,  18204,27244,  15446,28897,  12539,30272,  
													9511,31356,  6392,32137,  3211,32609,  0,32767,  -3211,32609,  -6392,32137,  -9511,31356,  -12539,30272,  -15446,28897,  -18204,27244,  -20787,25329,  -23169,23169,  -25329,20787, 
												  -27244,18204,  -28897,15446,  -30272,12539,  -31356,9511,  -32137,6392,  -32609,3211 };


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

void fft(short old_Re[], short old_Im[], short Real [], short Imag [])
{
  short m, i, j, l, sin_index=0, istep, shift,
  	    W_Imag, W_Real, // W = exp(-sqrt(-1)*2*pi/Input)Size
	      qi, qr, ti, tr; // Temporary coefficients

  // Time decimation, swaps are hard-coded, not calculated

	// Note: Each of these corresponds to a load/store. In Legup1.0, the memory
	// controller has 1 port and this takes a lot of clock cycles
	Real[0] = old_Re[0];
	Imag[0] = old_Im[0];
	Real[1] = old_Re[32];
	Imag[1] = old_Im[32];
	Real[2] = old_Re[16];
	Imag[2] = old_Im[16];
	Real[3] = old_Re[48];
	Imag[3] = old_Im[48];
	Real[4] = old_Re[8];
	Imag[4] = old_Im[8];
	Real[5] = old_Re[40];
	Imag[5] = old_Im[40];
	Real[6] = old_Re[24];
	Imag[6] = old_Im[24];
	Real[7] = old_Re[56];
	Imag[7] = old_Im[56];
	Real[8] = old_Re[4];
	Imag[8] = old_Im[4];
	Real[9] = old_Re[36];
	Imag[9] = old_Im[36];
	Real[10] = old_Re[20];
	Imag[10] = old_Im[20];
	Real[11] = old_Re[52];
	Imag[11] = old_Im[52];
	Real[12] = old_Re[12];
	Imag[12] = old_Im[12];
	Real[13] = old_Re[44];
	Imag[13] = old_Im[44];
	Real[14] = old_Re[28];
	Imag[14] = old_Im[28];
	Real[15] = old_Re[60];
	Imag[15] = old_Im[60];
	Real[16] = old_Re[2];
	Imag[16] = old_Im[2];
	Real[17] = old_Re[34];
	Imag[17] = old_Im[34];
	Real[18] = old_Re[18];
	Imag[18] = old_Im[18];
	Real[19] = old_Re[50];
	Imag[19] = old_Im[50];
	Real[20] = old_Re[10];
	Imag[20] = old_Im[10];
	Real[21] = old_Re[42];
	Imag[21] = old_Im[42];
	Real[22] = old_Re[26];
	Imag[22] = old_Im[26];
	Real[23] = old_Re[58];
	Imag[23] = old_Im[58];
	Real[24] = old_Re[6];
	Imag[24] = old_Im[6];
	Real[25] = old_Re[38];
	Imag[25] = old_Im[38];
	Real[26] = old_Re[22];
	Imag[26] = old_Im[22];
	Real[27] = old_Re[54];
	Imag[27] = old_Im[54];
	Real[28] = old_Re[14];
	Imag[28] = old_Im[14];
	Real[29] = old_Re[46];
	Imag[29] = old_Im[46];
	Real[30] = old_Re[30];
	Imag[30] = old_Im[30];
	Real[31] = old_Re[62];
	Imag[31] = old_Im[62];
	Real[32] = old_Re[1];
	Imag[32] = old_Im[1];
	Real[33] = old_Re[33];
	Imag[33] = old_Im[33];
	Real[34] = old_Re[17];
	Imag[34] = old_Im[17];
	Real[35] = old_Re[49];
	Imag[35] = old_Im[49];
	Real[36] = old_Re[9];
	Imag[36] = old_Im[9];
	Real[37] = old_Re[41];
	Imag[37] = old_Im[41];
	Real[38] = old_Re[25];
	Imag[38] = old_Im[25];
	Real[39] = old_Re[57];
	Imag[39] = old_Im[57];
	Real[40] = old_Re[5];
	Imag[40] = old_Im[5];
	Real[41] = old_Re[37];
	Imag[41] = old_Im[37];
	Real[42] = old_Re[21];
	Imag[42] = old_Im[21];
	Real[43] = old_Re[53];
	Imag[43] = old_Im[53];
	Real[44] = old_Re[13];
	Imag[44] = old_Im[13];
	Real[45] = old_Re[45];
	Imag[45] = old_Im[45];
	Real[46] = old_Re[29];
	Imag[46] = old_Im[29];
	Real[47] = old_Re[61];
	Imag[47] = old_Im[61];
	Real[48] = old_Re[3];
	Imag[48] = old_Im[3];
	Real[49] = old_Re[35];
	Imag[49] = old_Im[35];
	Real[50] = old_Re[19];
	Imag[50] = old_Im[19];
	Real[51] = old_Re[51];
	Imag[51] = old_Im[51];
	Real[52] = old_Re[11];
	Imag[52] = old_Im[11];
	Real[53] = old_Re[43];
	Imag[53] = old_Im[43];
	Real[54] = old_Re[27];
	Imag[54] = old_Im[27];
	Real[55] = old_Re[59];
	Imag[55] = old_Im[59];
	Real[56] = old_Re[7];
	Imag[56] = old_Im[7];
	Real[57] = old_Re[39];
	Imag[57] = old_Im[39];
	Real[58] = old_Re[23];
	Imag[58] = old_Im[23];
	Real[59] = old_Re[55];
	Imag[59] = old_Im[55];
	Real[60] = old_Re[15];
	Imag[60] = old_Im[15];
	Real[61] = old_Re[47];
	Imag[61] = old_Im[47];
	Real[62] = old_Re[31];
	Imag[62] = old_Im[31];
	Real[63] = old_Re[63];
	Imag[63] = old_Im[63];

  /* Butterfly computation. See Summaryp.pdf. For N =  64 point 
     FFT, there are log2(N) = 6 stages of butterflies, with N/2 = 32 butterflies
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
     (hence executes 6x in the 64 point example). 

NOTE: In hardware, all 32 butterflies should be in parallel during each stage. 
Currently this is not done by LegUp, as it is unclear that these 
loops can be unrolled. But the required computations do not depend on input values hence 
do not change for any 64-point FFT. Therefore, this set of three loops can be manually 
unrolled to either create six sets of nested for loops (eliminating the while loop) or 
to completely remove the for loops as well.

Therefore while in software using for loops is convenient, the loops can be unrolled 
manually in order to guide LegUp. */

  l = 1;
  while (l < Input_Size) // Executes log2(N) times
  {
    // Everything within the while loop should be in parellel (no dependence)
    if (inverse)
      shift = Input_Size-1; // We must scale down the inverse FFT by 1/N (by its definition)
    else shift = 0;
		
    istep = l << 1;    
    for (m=0; m<l; m++) {
      W_Imag =  sin_lookup[sin_index];
      W_Real = sin_lookup[sin_index+1];    
			sin_index = sin_index+2;
      if (inverse)
        W_Real = -W_Real;
      if (shift) {
        W_Imag >>= 1;
        W_Real >>= 1;
      }
      for (i=m; i<Input_Size; i+=istep) {	// inside this loop is the butterfly
        j = i + l;
				
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
    l = istep;
  }
}

int main()
{
  short i, Real[Input_Size], Imag[Input_Size], New_Real[Input_Size], New_Imag[Input_Size];
	volatile short input_temp;
  int sum = 0;
  
  // Set input, each input value consists of a Real and imaginary part
	for (i=0; i<Input_Size; i++){
		input_temp = 0;
		Imag[i] = input_temp;
		input_temp = 10*i;
    Real[i] = input_temp;
  }

  fft(Real, Imag, New_Real, New_Imag);

//  for (i=0; i<Input_Size; i++)
//    printf ("%d\t\t%d\n", New_Real[i], New_Imag[i]);

  for (i=0; i<Input_Size; i++) {
    sum += abs(New_Real[i]);
    sum += abs(New_Imag[i]);
  }

  printf ("Result: %d\n", sum);
  if (sum == 87100) {
      printf("RESULT: PASS\n");
  } else {
      printf("RESULT: FAIL\n");
  }
  
  return sum; 
}

