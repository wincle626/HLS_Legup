/*
  mandelbrot.c - Mandelbrot and Julia Set generator
  Copyright (C) 2002 Jack Neely <slack@quackmaster.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <cv.h>
#include <highgui.h>

#define MIN_ITER 2000
#define STEP 200
#define MAX_ITER 4000

#define NUM_THREADS 16

#define splitF   13
#define fourF    4L<<splitF

struct args {
  int xmin;
  int ymin;
  int xmax;
  int ymax;
  int thread_id;
  int num_threads;
  int itr;
  int xres;
  int yres;
  char *ptr;
};

typedef struct _color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} color;

color color_map(int itr, int max_itr)
{
    color c;
    int t;
    
    if (itr == max_itr) {
        memset(&c, 0, 3);
        return c;
    }
    
    t = max_itr - itr;
    c.r = t * 12 % 256;
    c.g = t * 16 % 256;
    c.b = t * 5 % 256;

    return c;
}

int generate_set_hw(int c1x, int c1y, int xres, int yres, int itr, volatile int thread_id)
{
    int i;

    int c2x = c1x, c2y = c1y;
    int cxsq, cysq;
    int t1,t2,condition;

    i=-1;
    do {
        i++;
        t1 = i<itr;
        cxsq = c2x*c2x;
        cysq = c2y*c2y;
        t2 = ((cxsq+cysq)>>splitF) <= fourF;
        c2y = ((2*c2y*c2x) >> splitF) + c1y;
        c2x = ((cxsq - cysq) >> splitF) + c1x;
    } while (t1 && t2);

    return i;
}

void generate_mandelbrot_row(struct args *arg)
{
    int i, j, c1x, c1y;

    c1x = arg->xmin;
    c1y = arg->ymin;

    for (j = 0; j < arg->yres; j++) {
        int result = generate_set_hw(c1x, c1y, arg->xres, arg->yres, arg->itr, arg->thread_id); 
        color c = color_map(result, arg->itr);

        c1y += arg->ymax;

        *(arg->ptr) = c.b;
        *(arg->ptr+1) = c.r;
        *(arg->ptr+2) = c.g;
        arg->ptr += 3;
    }
}

void* generate_mandelbrot_rows(void *data)
{
    struct args *arg = (struct args *)data;
    int i;
    for (i = arg->thread_id; i < arg->xres; i+= arg->num_threads) {
        generate_mandelbrot_row(arg);
        arg->xmin += arg->num_threads * arg->xmax;
        arg->ptr += 3 * arg->yres * (arg->num_threads - 1);
    }
    return NULL;
}

void generate_mandelbrot(int itr, int xres, int yres, IplImage *img)
{
    int xmin, xmax, ymin, ymax;
    int i;

    xmax = 2LL<<splitF;
    xmin = -2LL<<splitF;
    ymax = 2LL<<splitF;
    ymin = -2LL<<splitF;

    xmax = (xmax-xmin)/xres;
    ymax = (ymax-ymin)/yres;

    cvSet(img, CV_RGB(255, 255, 255), NULL);
    char *ptr = (char *)img->imageData;

    pthread_t pool[NUM_THREADS];
    struct args arg_array[NUM_THREADS];

    for (i = 0; i < NUM_THREADS; i++) {
        struct args arg_element = {xmin+i*xmax, ymin, xmax, ymax, i, NUM_THREADS, itr, xres, yres, ptr + 3 * yres * i};
        memcpy(arg_array+i, &arg_element, sizeof(arg_element));
        pthread_create(pool+i, NULL, generate_mandelbrot_rows, (void *)(arg_array + i));
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(pool[i], NULL);
    }
}

void* display_mandelbrot(void *args)
{
  IplImage *img = (IplImage *)args;
  cvNamedWindow("mandelbrot", CV_WINDOW_AUTOSIZE);
  while (1) {
    cvWaitKey(1000/60); // 60Hz refresh rate

    cvShowImage("mandelbrot", (IplImage *)img);
  }
  return NULL;
}

int main()
{
  int i, xres = 1024, yres = 1024;

  IplImage *img = cvCreateImage(cvSize(xres, yres), IPL_DEPTH_8U, 3);

  pthread_t display_thread;
  pthread_create(&display_thread, NULL, display_mandelbrot, (void *)img);

  for (i = MIN_ITER; i <= MAX_ITER; i+= STEP) {
    generate_mandelbrot(i, xres, yres, img);
  }

  puts("Press enter to quit");
  getchar();
}
