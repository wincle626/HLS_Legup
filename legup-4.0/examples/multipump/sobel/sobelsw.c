// Sobel edge detection
// Author: Andrew Canis
// Date: June 16, 2012
// To run:
//     gcc -g -lm sobelsw.c; ./a.out
// After check:
//     grad.pgm - image gradient
//     edge.pgm - edges

#include <stdio.h>
#include <assert.h>
#include <math.h>

//#define WIDTH 150
//#define HEIGHT 139

#define THRESHOLD 100

#define IMAGE "small_striped"
//#define IMAGE "large"

#define abs(a) ( ((a) < 0) ? -(a) : (a) )
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )


void write_image(char *fname, int width, int height, int max, int img[width][height]) {
    int i, j;
    FILE *f = fopen(fname, "w");
    fprintf(f, "P2\n%d %d\n%d\n", width, height, max);
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            // assume grayscale image
            fprintf(f, "%d ", img[i][j]);
        }
        fprintf(f, "\n");
    }
    close(f);
}

void write_stripe_array(FILE *f, int stripe, int width, int height, int
        img[width][height]) {

    int i, j, count = 0;
    for (j = stripe; j < height; j+=3) {
        count++;
    }

    fprintf(f, "unsigned char img%d[%d][%d] = {\n", stripe+1, width, count);
    for (i = 0; i < width; i++) {
        fprintf(f, "{");
        for (j = stripe; j < height; j+=3) {
            if (j>stripe) fprintf(f, ", ");
            // assume grayscale image
            fprintf(f, "%d", img[i][j]);
        }
        fprintf(f, "},\n");
    }
    fprintf(f, "};\n");
}

void write_array(char *fname, int width, int height, int max, int
        img[width][height]) {
    int i, j;
    FILE *f = fopen(fname, "w");
    fprintf(f, "int width = %d;\n", width);
    fprintf(f, "int height = %d;\n", height);
    fprintf(f, "int max = %d;\n", max);
    fprintf(f, "unsigned char img[%d][%d] = {\n", width, height);
    for (i = 0; i < width; i++) {
        fprintf(f, "{");
        for (j = 0; j < height; j++) {
            if (j) fprintf(f, ", ");
            // assume grayscale image
            fprintf(f, "%d", img[i][j]);
        }
        fprintf(f, "},\n");
    }
    fprintf(f, "};\n");

    write_stripe_array(f, 0, width, height, img);
    write_stripe_array(f, 1, width, height, img);
    write_stripe_array(f, 2, width, height, img);

    close(f);
}

int main(void) {

    FILE *f = fopen(IMAGE ".pgm", "r");
    assert(f);
    int width, height, max;
    fscanf(f, "P2 %d %d %d", &width, &height, &max);
    int i, j, c;
    int img[width][height];
    int grad[width][height];
    int edge[width][height];
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            fscanf(f, "%d", &img[i][j]);
        }
    }
    close(f);

    for (i = 0; i < width-2; i++) {
        for (j = 0; j < height-2; j++) {
            // Sobel mask for x-direction:
            int Gx = (2 * img[i+2][j+1] + img[i+2][j] + img[i+2][j+2]) -
                     (2 * img[i][j+1] + img[i][j] + img[i][j+2]);
            // Sobel mask for y-direimgtion:
            int Gy = (2*img[i+1][j+2] + img[i][j+2] + img[i+2][j+2]) - 
                     (2*img[i+1][j] + img[i][j] + img[i+2][j]);
           
            // magnitude of the gradient
            int s2 = Gx*Gx + Gy*Gy;
            int s = sqrt(s2)+0.5;
            assert(s >= 0);
            //assert(s <= 255);
            grad[i][j] = min(255, s);

            // get edge based on threshold
            edge[i][j] = 0;
            if (s2 > THRESHOLD*THRESHOLD) {
                edge[i][j] = 1;
            }
        }
    }

    //write_image("orig.pgm", width, height, 255, img);
    write_array(IMAGE ".c", width, height, 1, img);
    write_image("grad.pgm", width, height, 255, grad);
    write_image("edge.pgm", width, height, 1, edge);
}
