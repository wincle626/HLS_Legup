// Gaussian blur image filter
// Author: Andrew Canis
// Date: June 30, 2012
// This kernel uses a lot of the same
// code from sobel

#include <stdio.h>
#include <assert.h>
#include <math.h>

// image file in pgm format
//#include "small.c"
#include "small_striped.c"

// size of filter
#define SIZE 3

// 32-bit fixed point: 19.13
#define DECIMAL_PLACES 13

#define int2fixed(num) ((num) << DECIMAL_PLACES)
//#define fixedmul(a, b) ((((long long)a) * ((long long)b)) >> DECIMAL_PLACES)
#define fixedmul(a, b) (((a) * (b)) >> DECIMAL_PLACES)
#define fixed2int(num) ((num) >> DECIMAL_PLACES)


inline void print_image(int width, int height, int max, unsigned char img[width][height]) {
    int i, j;
    printf("P2\n%d %d\n%d\n", width, height, max);
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            // assume grayscale image
            printf("%d ", img[i][j]);
        }
        printf("\n");
    }
}

//#define FLOAT 1
//#define PRINT_IMG 1

int main() {
    int i, j, m, n;

    // gaussian low pass filter
    const float gaussFloat[SIZE][SIZE] = {
        {0.1019, 0.1154, 0.1019},
        {0.1154, 0.1308, 0.1154},
        {0.1019, 0.1154, 0.1019}
    };

    int gauss[SIZE][SIZE];
    for (m = 0; m < SIZE; m++) {
        for (n = 0; n < SIZE; n++) {
            gauss[m][n] = gaussFloat[m][n] * int2fixed(1);
        }
    }

    /*
    const float gauss[SIZE][SIZE] = {
        {1, 2, 1},
        {2, 5, 2},
        {1, 2, 1},
    };
    */

    unsigned char blur[width-2][height-2];

    int count = 0;

    /*
     * Non-striped memory version
    for (j = 0; j < height-2; j++) {
        for (i = 0; i < width-2; i++) {
#ifdef FLOAT
            float sumFloat = 0;
#endif
            int sum = 0;
            for (m = 0; m < SIZE; m++) {
                for (n = 0; n < SIZE; n++) {
#ifdef FLOAT
                    sumFloat += gaussFloat[m][n]*img[i+m][j+n];
#endif
                    sum += fixedmul(gauss[m][n], int2fixed(img[i+m][j+n]));
                }
            }
            blur[i][j] = fixed2int(sum);
            count += blur[i][j];
            //blur[i+1][j+1] = sum/17;
#ifdef FLOAT
            float diff = fabs((float)blur[i][j]-sumFloat);
            assert (diff < 1.2);
            printf("fixed: %d expected: %f diff: %f\n", blur[i][j], sumFloat, diff);
#endif
        }
    }
    */



    int order = 0;
    int j_stride = 0;
    for (j = 0; j < height-2; j++) {
        // im00 im10 im20
        // im01 im11 im21
        // im02 im12 im22
        unsigned char im00, im01, im02, im10, im11, im12, im20, im21, im22;
        unsigned char tmp00, tmp10, tmp01, tmp11, tmp20, tmp21;
        i = 0;
        int j1, j2, j3;
        if (order == 0) {
            j1 = j2 = j3 = j_stride;
        } else if (order == 1) {
            j2 = j3 = j_stride;
            j1 = j_stride+1;
        } else {
            j3 = j_stride;
            j1 = j2 = j_stride+1;
        }

        im00 = img1[i][j1]; im10 = img1[i+1][j1];
        //assert(img[i][j]==im00);
        im01 = img2[i][j2]; im11 = img2[i+1][j2];
        im02 = img3[i][j3]; im12 = img3[i+1][j3];

        if (order == 0) {
        } else if (order == 1) {
            tmp00 = im00; tmp10 = im10;
            // first row from img2
            im00 = im01; im10 = im11;
            // second row from img3
            im01 = im02; im11 = im12;
            // third row from img1
            im02 = tmp00; im12 = tmp10;
        } else {
            tmp00 = im00; tmp10 = im10;
            tmp01 = im01; tmp11 = im11;
            // first row from img3
            im00 = im02; im10 = im12;
            // second row from img1
            im01 = tmp00; im11 = tmp10;
            // third row from img2
            im02 = tmp01; im12 = tmp11;
        }

        for (i = 0; i < width-2; i++) {
            im20 = img1[i+2][j1];
            im21 = img2[i+2][j2];
            im22 = img3[i+2][j3];

            int pred1 = (order == 1);
            tmp20 = im20;
            // first row from img2
            im20 = (pred1) ? im21 : im20;
            // second row from img3
            im21 = (pred1) ? im22: im21;
            // third row from img1
            im22 = (pred1) ? tmp20: im22;

            tmp20 = im20;
            tmp21 = im21;
            int pred2 = (order == 2);

            // first row from img3
            im20 = (pred2) ? im22 : im20;
            // second row from img1
            im21 = (pred2) ? tmp20 : im21;
            // third row from img2
            im22 = (pred2) ? tmp21 : im22;

            int sum = 0;
            //assert(img[i][j]==im00);
            sum += fixedmul(gauss[0][0], int2fixed(im00));
            sum += fixedmul(gauss[0][1], int2fixed(im01));
            sum += fixedmul(gauss[0][2], int2fixed(im02));
            sum += fixedmul(gauss[1][0], int2fixed(im10));
            sum += fixedmul(gauss[1][1], int2fixed(im11));
            sum += fixedmul(gauss[1][2], int2fixed(im12));
            sum += fixedmul(gauss[2][0], int2fixed(im20));
            sum += fixedmul(gauss[2][1], int2fixed(im21));
            sum += fixedmul(gauss[2][2], int2fixed(im22));

            blur[i][j] = fixed2int(sum);
            //blur[i+1][j+1] = sum/17;
            count += blur[i][j];

#ifdef FLOAT
            float sumFloat = 0;
            for (m = 0; m < SIZE; m++) {
                for (n = 0; n < SIZE; n++) {
                    sumFloat += gaussFloat[m][n]*img[i+m][j+n];
                }
            }
            float diff = fabs((float)blur[i][j]-sumFloat);
            assert (diff < 1.2);
            printf("fixed: %d expected: %f diff: %f\n", blur[i][j], sumFloat, diff);
#endif

            // shift over to the right by 1
            im00 = im10; im10 = im20;
            im01 = im11; im11 = im21;
            im02 = im12; im12 = im22;
        }

        if (order == 2) {
            order = 0;
            j_stride++;
        } else {
            order++;
        }
    }

#ifdef PRINT_IMG
    //print_image(width, height, 255, img);
    print_image(width-2, height-2, 255, blur);
#else

    printf("count: %d\n", count);
    if (count == 1931186) {
        printf(" _____         _____ _____ ______ _____  \n");
        printf("|  __ \\ /\\    / ____/ ____|  ____|  __ \\ \n");
        printf("| |__) /  \\  | (___| (___ | |__  | |  | |\n");
        printf("|  ___/ /\\ \\  \\___ \\\\___ \\|  __| | |  | |\n");
        printf("| |  / ____ \\ ____) |___) | |____| |__| |\n");
        printf("|_| /_/    \\_\\_____/_____/|______|_____/ \n");
    } else {
        printf(" ______      _____ _      ______ _____  \n");
        printf("|  ____/\\   |_   _| |    |  ____|  __ \\ \n");
        printf("| |__ /  \\    | | | |    | |__  | |  | |\n");
        printf("|  __/ /\\ \\   | | | |    |  __| | |  | |\n");
        printf("| | / ____ \\ _| |_| |____| |____| |__| |\n");
        printf("|_|/_/    \\_\\_____|______|______|_____/ \n");
    }

#endif

    return count;
}
