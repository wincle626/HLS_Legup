// Sobel edge detection
// Author: Andrew Canis
// Date: June 16, 2012

#include <stdio.h>
#include <assert.h>
#include <math.h>

//#define WIDTH 150
//#define HEIGHT 139

#define THRESHOLD 100

#define abs(a) ( ((a) < 0) ? -(a) : (a) )
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )

#define COUNT

//#include "large.c"
//#include "small.c"
#include "small_striped.c"

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

int main(void) {

    int i, j;
    unsigned char edge[width-2][height-2];

    int order = 0;
    int j_stride = 0;
#ifdef COUNT
    int count = 0;
#endif
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
            //im00 = img1[i][j_stride]; im10 = img1[i+1][j_stride];
            //im01 = img2[i][j_stride]; im11 = img2[i+1][j_stride];
            //im02 = img3[i][j_stride]; im12 = img3[i+1][j_stride];
        } else if (order == 1) {
            j2 = j3 = j_stride;
            j1 = j_stride+1;
            //im00 = img2[i][j_stride]; im10 = img2[i+1][j_stride];
            //im01 = img3[i][j_stride]; im11 = img3[i+1][j_stride];
            //im02 = img1[i][j_stride+1]; im12 = img1[i+1][j_stride+1];
        } else {
            j3 = j_stride;
            j1 = j2 = j_stride+1;
            //im00 = img3[i][j_stride]; im10 = img3[i+1][j_stride];
            //im01 = img1[i][j_stride+1]; im11 = img1[i+1][j_stride+1];
            //im02 = img2[i][j_stride+1]; im12 = img2[i+1][j_stride+1];
        }

        im00 = img1[i][j1]; im10 = img1[i+1][j1];
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
            /*
             * requires pointers that point to different
             * memory which LOCAL_RAMS can't handle
            if (order == 0) {
                im20 = img1[i+2][j_stride]; 
                im21 = img2[i+2][j_stride];
                im22 = img3[i+2][j_stride];
            } else if (order == 1) {
                im20 = img2[i+2][j_stride]; 
                im21 = img3[i+2][j_stride];
                im22 = img1[i+2][j_stride+1];
            } else {
                im20 = img3[i+2][j_stride]; 
                im21 = img1[i+2][j_stride+1];
                im22 = img2[i+2][j_stride+1];
            }
            */
            //assert(i+2 < 150);
            //assert(j_stride < 46);

            im20 = img1[i+2][j1];
            //im20 = 0;
            im21 = img2[i+2][j2];
            //im21 = 0;
            im22 = img3[i+2][j3];
            //im22 = 0;

            /*
             * for loop pipelining we perform
             * this if-else statement with 
             * predicates
            if (order == 0) {
            } else if (order == 1) {
                tmp20 = im20;
                // first row from img2
                im20 = im21;
                // second row from img3
                im21 = im22;
                // third row from img1
                im22 = tmp20;
            } else {
                tmp20 = im20;
                tmp21 = im21;
                // first row from img3
                im20 = im22;
                // second row from img1
                im21 = tmp20;
                // third row from img2
                im22 = tmp21;
            }
            */


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

            /* TEMP
            */



#if 0
            assert(im00 == img[i][j]);
            assert(im01 == img[i][j+1]);
            assert(im10 == img[i+1][j]);
            assert(im11 == img[i+1][j+1]);
            assert(im20 == img[i+2][j]);
            assert(im21 == img[i+2][j+1]);
#endif
            int e = 0;
            short Gx = 0;
            short Gy = 0;
            int s2 = 0;

            // Sobel mask for x-direction:
            Gx = (2 * im21 + im20 + im22) -
                     (2 * im01 + im00 + im02);
            // Sobel mask for y-direimgtion:
            Gy = (2*im12 + im02 + im22) - 
                     (2*im10 + im00 + im20);
           
            // magnitude of the gradient squared
            s2 = Gx*Gx + Gy*Gy;

            // get edge based on threshold
            e = (s2 > THRESHOLD*THRESHOLD);
            edge[i][j] = e;

            /* had to move this outside loop
            count += e;
            */

            //edge[i][j] = Gx;
            //printf("i: %d Gx: %d E: %d\n", i, (int)Gx, e);

            //printf("i: %d Gx: %d E: %d\n", i, (int)Gx, e);

            //printf("im00: %d im01: %d im02: %d\n", // im10: %d im11: %d im12: %d im20: %d im21: %d im22: %d\n",
            /*
            printf("i: %d im00: %d im10: %d im20: %d im01: %d im11: %d im21: %d, im02: %d im12: %d im22: %d Gx: %d Gy: %d s2: %d E: %d\n",
                i,
                im00,
                im10,
                im20,
                im01,
                im11,
                im21,
                im02,
                im12,
                im22,
                (int)Gx,
                (int)Gy,
                s2,
                e);
                //im10,
                //im11,
                //im20,
                //im21,
                //im22);
                */

            // shift over to the right by 1
            im00 = im10; im10 = im20;
            im01 = im11; im11 = im21;
            im02 = im12; im12 = im22;
            /* TEMP
            */
        }

        if (order == 2) {
            order = 0;
            j_stride++;
        } else {
            order++;
        }
    }

    //print_image(width, height, 255, img);
    //print_image(width-2, height-2, 1, edge);
#ifdef COUNT

    for (j = 0; j < height-2; j++) {
        for (i = 0; i < width-2; i++) {
            count+=edge[i][j];
        }
    }
    /*
    */
    printf("Count: %d\n", count);
    //if (count == 7239) {
    if (count == 7119) {
    // TEMP
    //if (count == 6527) {
    //if (count == 14053) {
    //if (count == 1938287) {
    //if (count == 2937083) {
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
    return edge[width-3][height-3];
}
