#include <stdio.h>
#include <stdlib.h>

#include "sobel.h"

__attribute__ ((noinline))
//void sobel(unsigned char * indata, 
//    unsigned char * outdata) 
void sobel() 
{
    unsigned int X, Y;

    /*---------------------------------------------------
        SOBEL ALGORITHM STARTS HERE
    ---------------------------------------------------*/
    #pragma omp parallel for num_threads(2) private(Y)
    for(Y=1; Y<(ROWS-1); Y++)  {
loop2:  for(X=1; X<(COLS-1); X++)  {
            long sumX = 0;
            long sumY = 0;
            int  SUM = 0;
            int I, J;

            /* 3x3 GX Sobel mask. */
            const int GX[9] = { -1,  0,  1,
                                -2,  0,  2,
                                -1,  0,  1};

            /* 3x3 GY Sobel mask. */
            const int GY[9] = { 1,  2,  1,
                                0,  0,  0,
                               -1, -2, -1};

            /*-------X GRADIENT APPROXIMATION------*/
            for(I=-1; I<=1; I++)  {
                for(J=-1; J<=1; J++)  {
                    sumX += (int)( elaine_512_input[(Y+J)*512 + X+I] * GX[3*I + J + 4]);
                    sumY += (int)( elaine_512_input[(Y+J)*512 + X+I] * GY[3*I + J + 4]);
                }
            }


            /*-------X GRADIENT BOUNDS------*/
            sumX = sumX > 255 ? 255 : sumX;
            sumX = sumX < 0 ? 0 : sumX;

            /*-------Y GRADIENT BOUNDS-------*/
            sumY = sumY > 255 ? 255 : sumY;
            sumY = sumY < 0 ? 0 : sumY;

            /*---GRADIENT MAGNITUDE APPROXIMATION ----*/
            SUM = sumX + sumY; 

            /* make edges black and background white */
            outdata[512*Y+X] = 255 - (unsigned char)(SUM);  
        }
    }
}

int main()
{

    int i, Y=0 , X=-1;

    unsigned int result = 0;

    //sobel(elaine_512_input, outdata);
    sobel();


    for(Y=0; Y<ROWS; Y++)  {
loop1:	for(X=0; X<COLS; X++)  {
            if( outdata[512*Y+X] != elaine_512_golden_output[512*Y+X])
                result++; 
        }
    }

    if (!result)
        printf("PASS!\n");
    else
        printf("FAIL with %d differences\n", result);

    return 0;
}

