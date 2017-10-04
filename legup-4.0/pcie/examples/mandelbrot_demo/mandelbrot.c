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
#include <stdbool.h>

#include <cv.h>
#include <highgui.h>

#include "legup_mem.h"

#define MIN_ITER 0
#define STEP 1
#define MAX_ITER 63

#define NUM_THREADS 24

struct args {
  int dividend;
  int xmax;
  int xmin;
  int ymax;
  int ymin;
  int itr;
  int xres;
  int yres;
  int *ptr;
  int *SHARED_MEM_ptr;
  int thread_id;
  int num_threads;
};

typedef int color;

inline color color_map(int itr, int max_itr)
{
    color c = 0;
    int t;
    
    if (itr == max_itr) {
        return c;
    }
    
    t = max_itr - itr;
    c |= ((t * 8 + t * 4) & 0xff) << (8);
    c |= (t * 16 & 0xff) << (2*8);
    c |= (t * 4 + t) & 0xff;

    return c;
}

#define splitF   13
#define fourF    4L<<splitF

inline int generate_set_pixel(int c1x, int c1y, int xres, int yres, int itr)
{
    int i;

    int c2x = c1x, c2y = c1y;
    int cx_squared, cy_squared;
    bool t1, t2;

    i=-1;
    do {
        i++;
        t1 = i<itr;
        cx_squared = c2x*c2x;
        cy_squared = c2y*c2y;
        t2 = ((cx_squared+cy_squared) >> splitF) <= fourF;
        c2y = ((2*c2y*c2x) >> splitF) + c1y;
        c2x = ((cx_squared - cy_squared) >> splitF) + c1x;
    } while (t1 && t2);

    return i;
}

void generate_set(int xstart, int xend, int xmax, int xmin, int ymax, int ymin,
        int xres, int yres, int itr, int *ptr)
{
    int x, y, c1x = xmin + xstart * xmax, c1y;
    long long *lptr = (long long *)ptr;

    for (x=xstart; x<xend; ++x)
    {
        c1y = ymin;
        color last_color = 0;
        for (y=0; y<yres; ++y)
        {
            int result = generate_set_pixel(c1x, c1y, xres, yres, itr); 
            color c = color_map(result, itr);

            c1y += ymax;

            if (y & 1) {
                *lptr++ = (long long)(c) << 32 | (long long) last_color;
            } else {
                last_color = c;
            }
        }
        c1x += xmax;
    }
}

void* pthread_generate_set(void *data)
{
    struct args *arg = (struct args *)data;

    int xpartition, i, row_size;
    int loop_itr = 1 << arg->dividend;

    // tentatively set xpartition to 1/(1<<dividend) of the set, since the on-chip memory can't hold the whole set 
    xpartition = arg->xres >> arg->dividend;
    row_size = (xpartition+arg->num_threads-1) / arg->num_threads;

    /* Since the pcie hybrid version reuses the same memory to compute different portions of the image,
    the SW_ONLY executable should use a non-zero OFFSET*/
#ifdef SW_ONLY
#define OFFSET (i*xpartition*arg->yres)
#else
#define OFFSET (0)
#endif

    for (i = 0; i < loop_itr; i++) {
        int xstart = i*xpartition + arg->thread_id * row_size;
        int xend = MIN((i+1)*xpartition, i*xpartition + (arg->thread_id+1) * row_size);
        generate_set(xstart, xend, arg->xmax, arg->xmin, arg->ymax, arg->ymin, arg->xres, arg->yres, arg->itr, arg->SHARED_MEM_ptr + arg->yres*arg->thread_id*row_size + OFFSET);

        // post-copy
        memcpy_from_shared(arg->ptr + arg->yres*xstart, arg->SHARED_MEM_ptr + arg->yres*arg->thread_id*row_size, 4*arg->yres*(xend-xstart));
    }

   return NULL;
}

void generate_mandelbrot(int itr, int xres, int yres, IplImage *img)
{
    int xmin, xmax, ymin, ymax;
    int i, dividend = 1;

    xmax = 0x5200;
    xmin = 0xffffd200;
    ymax = 2LL<<splitF;
    ymin = -2LL<<splitF;

    xmax = (xmax-xmin)/xres;
    ymax = (ymax-ymin)/yres;

    xmax -= itr;
    ymax -= itr;
    ymin += itr*yres/2;

    int *ptr = (int *)img->imageData;

    // pre-allocate
    int *SHARED_MEM_ptr = malloc_shared((4*xres*yres) >> dividend, ptr, LEGUP_RAM_LOCATION_ONCHIP);

    pthread_t pool[NUM_THREADS];
    struct args arg_array[NUM_THREADS];

    for (i = 0; i < NUM_THREADS; i++) {
        struct args arg_element = {dividend, xmax, xmin, ymax, ymin, 2000, xres, yres, ptr, SHARED_MEM_ptr, i, NUM_THREADS};
        memcpy(arg_array+i, &arg_element, sizeof(arg_element));
        pthread_create(pool+i, NULL, pthread_generate_set, (void *)(arg_array + i));
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(pool[i], NULL);
    }

    // post-free
    free_shared(SHARED_MEM_ptr);
}

void* display_mandelbrot(void *args)
{
  IplImage **render_img_ptr = (IplImage **)args;
  cvNamedWindow("mandelbrot", CV_WINDOW_AUTOSIZE);
  while (*render_img_ptr) {
    cvWaitKey(1000/60); // 60Hz refresh rate

    cvShowImage("mandelbrot", *render_img_ptr);
  }
  return NULL;
}

int main()
{
  int i, xres = 512, yres = 512;

  IplImage *img1 = cvCreateImage(cvSize(xres, yres), IPL_DEPTH_8U, 4);
  IplImage *img2 = cvCreateImage(cvSize(xres, yres), IPL_DEPTH_8U, 4);
  IplImage *render_img = img1;

  pthread_t display_thread;
  pthread_create(&display_thread, NULL, display_mandelbrot, (void *)&render_img);

  for (i = MIN_ITER; i <= MAX_ITER; i+= STEP) {
    generate_mandelbrot(i, xres, yres, (render_img == img1) ? img2 : img1);
    render_img = ((render_img == img1) ? img2 : img1);
  }

  puts("Press enter to quit");
  getchar();

  render_img = NULL;

  pthread_join(display_thread, NULL);
}
