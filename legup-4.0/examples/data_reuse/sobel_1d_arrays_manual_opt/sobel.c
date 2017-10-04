#include <stdio.h>
#include <stdlib.h>

#include "sobel.h"

__attribute__ ((noinline))
void sobel() 
{
    unsigned int X=0, Y=1, Z;

    unsigned char row1[512];
    unsigned char row2[512];

    unsigned char window[3][3];

loop3:for(Z=0; Z<COLS; Z++) 
    {  
        row1[Z] = elaine_512_input[Z];
        row2[Z] = elaine_512_input[Z+512];
    }

    /*---------------------------------------------------
        SOBEL ALGORITHM STARTS HERE
    ---------------------------------------------------*/
    #pragma omp parallel for num_threads(2) private(Y)
    for(Y=1; Y<(ROWS-1); Y++)  {
        // Set-up window
    	for(X=0; X<2; X++)  {
            window[0][X+1] = row1[X];
            window[1][X+1] = row2[X];
            window[2][X+1] = elaine_512_input[(Y+1)*512 + X];
            row1[X] = row2[X];
            row2[X] = elaine_512_input[(Y+1)*512 + X];
        }

loop2:  for(X=1; X<(COLS-1); X++)  {
            long sumX = 0;
            long sumY = 0;
            int  SUM = 0;
            int I, J;

            window[0][0] = window[0][1];
            window[1][0] = window[1][1];
            window[2][0] = window[2][1];
            window[0][1] = window[0][2];
            window[1][1] = window[1][2];
            window[2][1] = window[2][2];
            window[0][2] = row1[X+1];
            window[1][2] = row2[X+1];
            window[2][2] = elaine_512_input[512*(Y+1) + X+1];
            row1[X+1]    = row2[X+1];
            row2[X+1]    = window[2][2];

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
                    sumX += (int)( window[J+1][I+1] * GX[3*I + J + 4]);
                    sumY += (int)( window[J+1][I+1] * GY[3*I + J + 4]);
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

