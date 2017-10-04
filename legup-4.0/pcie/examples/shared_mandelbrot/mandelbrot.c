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

#include "legup_mem.h"

#define MIN_ITER 20000
#define STEP 2000
#define MAX_ITER 40000

typedef int color;

inline color color_map(int itr, int max_itr)
{
    color c = 0;
    int t;
    
    if (itr == max_itr) {
        return c;
    }
    
    t = max_itr - itr;
    c |= ((t * 8 + t * 4) & 0xff) << (2*8);
    c |= (t * 16 & 0xff) << (8);
    c |= (t * 4 + t) & 0xff;

    return c;
}

#define splitF   13
#define fourF    4L<<splitF

inline int generate_set_pixel(int c1x, int c1y, int xres, int yres, int itr)
{
    int i;

    int c2x = c1x, c2y = c1y;
    int cxsq, cysq;
    int t1, t2;

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

void generate_set(int xstart, int xend, int xmax, int xmin, int ymax, int ymin,
        int xres, int yres, int itr, char *ptr)
{
    int x, y, c1x = xmin + xstart * xmax, c1y;

    for (x=xstart; x<xend; ++x)
    {
        c1y = ymin;
        for (y=0; y<yres; ++y)
        {
            int result = generate_set_pixel(c1x, c1y, xres, yres, itr); 
            color c = color_map(result, itr);

            c1y += ymax;

            *ptr++ = (char)(c);
            *ptr++ = (char)(c >> (2 * 8));
            *ptr++ = (char)(c >> (8));
        }
        c1x += xmax;
    }
}

void generate_mandelbrot(int itr, int xres, int yres, IplImage *img)
{
    int xmin, xmax, ymin, ymax;
    int xend, i, dividend=2;
    int loop_itr = 1 << dividend;

    xmax = 2LL<<splitF;
    xmin = -2LL<<splitF;
    ymax = 2LL<<splitF;
    ymin = -2LL<<splitF;

    xmax = (xmax-xmin)/xres;
    ymax = (ymax-ymin)/yres;

    cvSet(img, CV_RGB(255, 255, 255), NULL);
    char *ptr = (char *)img->imageData;

    // tentatively set xend to 1/(1<<dividend) of the set, since the on-chip memory can't hold the whole set 
    xend = xres >> dividend;

    // pre-allocate
    char *SHARED_MEM_ptr = malloc_shared(3*xend*yres, ptr, LEGUP_RAM_LOCATION_ONCHIP);

    /* Since the pcie hybrid version reuses the same memory to compute different portions of the image,
    the SW_ONLY executable should use a non-zero OFFSET*/
#ifdef SW_ONLY
#define OFFSET (3*ixend*yres)
#else
#define OFFSET (0)
#endif

    for (i = 0; i < loop_itr; i++) {
        //silly CP (constant propagation)
        int ixend = i * xend;

        generate_set(ixend, xend + ixend, xmax, xmin, ymax, ymin, xres, yres, itr, SHARED_MEM_ptr+OFFSET);

        // post-copy
        memcpy_from_shared(ptr+(3*ixend*yres), SHARED_MEM_ptr, 3*xend*yres);
    }

    // post-free
    free_shared(SHARED_MEM_ptr);
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
