#include <stdio.h>
#include <stdlib.h>

#include "sobel.h"

__attribute__((noinline)) int sobel() {
    unsigned int X, Y;
    int result = 0;
    /*---------------------------------------------------
        SOBEL ALGORITHM STARTS HERE
    ---------------------------------------------------*/
    for(Y=1; Y<ROWS-1; Y++)  {
loop2:	for(X=1; X<COLS-1; X++)  {
            long sumX = 0;
            long sumY = 0;
            int  SUM = 0;
            int  I, J;
			
            /* 3x3 GX Sobel mask. */
            const int GX[3][3] = { {-1,  0,  1},
                                   {-2,  0,  2},
                                   {-1,  0,  1}};

            /* 3x3 GY Sobel mask. */
            const int GY[3][3] = {{ 1,  2,  1},
                                  { 0,  0,  0},
                                  {-1, -2, -1}};

	        /*-------X GRADIENT APPROXIMATION------*/
	        for(I=-1; I<=1; I++)  {
    		    for(J=-1; J<=1; J++)  {
		            sumX += (int)(elaine_512_input[Y+J][X+I] * GX[I+1][J+1]);
		            sumY += (int)(elaine_512_input[Y+J][X+I] * GY[I+1][J+1]);
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
            outdata[Y][X] = 255 - (unsigned char)(SUM);

            result = (outdata[Y][X] == elaine_512_golden_output[Y][X])
                         ? result
                         : result + 1;
    }
}

return result;
}

int main() {
    int i, j;

    int result = 0;

    result = sobel();

    for (i = 0; i < 1; i++) {
    loop1:
        for (j = 0; j < 1; j++) {
            //    for(i=0; i<ROWS; i++) {
            // loop1:  for(j=0; j<COLS; j++){
            if (outdata[i][j] != elaine_512_golden_output[i][j]) {
                result++;
                //                printf("%d %d %d %d\n", i, j,
                //                elaine_512_golden_output[i][j],
                //                outdata[i][j]);
            }
        }
    }

    if (!result)
        printf("PASS!\n");
    else
        printf("FAIL with %d differences\n", result);

    return 0;
}

