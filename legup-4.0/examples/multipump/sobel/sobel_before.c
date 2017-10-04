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

//#include "large.c"
#include "small.c"

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

    for (i = 0; i < width-2; i++) {
        // im00 im01 im02
        // im10 im11 im12
        // im20 im21 im22
        unsigned char im00, im01, im02, im10, im11, im12, im20, im21, im22;
        j = 0;
        im00 = img[i][j];   im01 = img[i][j+1];
        im10 = img[i+1][j]; im11 = img[i+1][j+1];
        im20 = img[i+2][j]; im21 = img[i+2][j+1];

        for (j = 0; j < height-2; j++) {
            im02 = img[i][j+2];
            im12 = img[i+1][j+2];
            im22 = img[i+2][j+2];

#if 0
            assert(im00 == img[i][j]);
            assert(im01 == img[i][j+1]);
            assert(im10 == img[i+1][j]);
            assert(im11 == img[i+1][j+1]);
            assert(im20 == img[i+2][j]);
            assert(im21 == img[i+2][j+1]);
#endif

            // Sobel mask for x-direction:
            short Gx = (2 * im21 + im20 + im22) -
                     (2 * im01 + im00 + im02);
            // Sobel mask for y-direimgtion:
            short Gy = (2*im12 + im02 + im22) - 
                     (2*im10 + im00 + im20);
           
            // magnitude of the gradient squared
            int s2 = Gx*Gx + Gy*Gy;

            // get edge based on threshold
            edge[i][j] = 0;
            if (s2 > THRESHOLD*THRESHOLD) {
                edge[i][j] = 1;
            }

            // shift over to the right by 1
            im00 = im01; im01 = im02;
            im10 = im11; im11 = im12;
            im20 = im21; im21 = im22;
        }
    }

    //print_image(width, height, 255, img);
    //print_image(width-2, height-2, 1, edge);
    return edge[width-3][height-3];
}
