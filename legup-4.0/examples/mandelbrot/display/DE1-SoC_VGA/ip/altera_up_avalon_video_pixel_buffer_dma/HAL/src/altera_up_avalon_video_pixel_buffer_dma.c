/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2006 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
*                                                                             *
******************************************************************************/

#include <errno.h>
#include <io.h>

#include <priv/alt_file.h>

#include "altera_up_avalon_video_pixel_buffer_dma.h"

#define ABS(x)	((x >= 0) ? (x) : (-(x)))

alt_up_pixel_buffer_dma_dev* alt_up_pixel_buffer_dma_open_dev(const char* name) {
  // find the device from the device list 
  // (see altera_hal/HAL/inc/priv/alt_file.h 
  // and altera_hal/HAL/src/alt_find_dev.c 
  // for details)
  alt_up_pixel_buffer_dma_dev *dev = (alt_up_pixel_buffer_dma_dev*)alt_find_dev(name, &alt_dev_list);

  return dev;
}

int alt_up_pixel_buffer_dma_draw(alt_up_pixel_buffer_dma_dev *pixel_buffer, unsigned int color, unsigned int x, unsigned int y)
/* This function draws a pixel to the back buffer.
 */
{
	// boundary check
	if (x >= pixel_buffer->x_resolution || y >= pixel_buffer->y_resolution )
		return -1;

	unsigned int addr = 0;
	/* Check the mode VGA Pixel Buffer is using. */
	if (pixel_buffer->addressing_mode == ALT_UP_PIXEL_BUFFER_XY_ADDRESS_MODE) {
		/* For X-Y addressing mode, the address format is | unused | Y | X |. So shift bits for coordinates X and Y into their respective locations. */
		addr |= ((x & pixel_buffer->x_coord_mask) << pixel_buffer->x_coord_offset);
		addr |= ((y & pixel_buffer->y_coord_mask) << pixel_buffer->y_coord_offset);
	} else {
		/* In a consecutive addressing mode, the pixels are stored in consecutive memory locations. So the address of a pixel at (x,y) can be computed as
		 * (y*x_resolution + x).*/
		addr += ((x & pixel_buffer->x_coord_mask) << pixel_buffer->x_coord_offset);
		addr += (((y & pixel_buffer->y_coord_mask) * pixel_buffer->x_resolution) << pixel_buffer->x_coord_offset);
	}
	/* Now, depending on the color depth, write the pixel color to the specified memory location. */
	if (pixel_buffer->color_mode == ALT_UP_8BIT_COLOR_MODE) {
		IOWR_8DIRECT(pixel_buffer->back_buffer_start_address, addr, color);
	} else if (pixel_buffer->color_mode == ALT_UP_16BIT_COLOR_MODE) {
		IOWR_16DIRECT(pixel_buffer->back_buffer_start_address, addr, color);
	} else {
		IOWR_32DIRECT(pixel_buffer->back_buffer_start_address, addr, color);
	}

	return 0;
}

int alt_up_pixel_buffer_dma_change_back_buffer_address(alt_up_pixel_buffer_dma_dev *pixel_buffer, unsigned int new_address)
/* This function changes the memory address for the back buffer. */
{
	IOWR_32DIRECT(pixel_buffer->base, 4, new_address);
	pixel_buffer->back_buffer_start_address = IORD_32DIRECT(pixel_buffer->base, 4);
	return 0;
}

int alt_up_pixel_buffer_dma_swap_buffers(alt_up_pixel_buffer_dma_dev *pixel_buffer)
/* This function swaps the front and back buffers. At the next refresh cycle the back buffer will be drawn on the screen
 * and will become the front buffer. */
{
	register unsigned int temp = pixel_buffer->back_buffer_start_address;
	IOWR_32DIRECT(pixel_buffer->base, 0, 1);
	pixel_buffer->back_buffer_start_address = pixel_buffer->buffer_start_address;
	pixel_buffer->buffer_start_address = temp;
	return 0;
}

int alt_up_pixel_buffer_dma_check_swap_buffers_status(alt_up_pixel_buffer_dma_dev *pixel_buffer)
/* This function checks if the buffer swap has occured. Since the buffer swap only happens after an entire screen is drawn,
 * it is important to wait for this function to return 0 before proceeding to draw on either buffer. When both front and the back buffers
 * have the same address calling the alt_up_pixel_buffer_dma_swap_buffers(...) function and then waiting for this function to return 0, causes your program to
 * wait for the screen to refresh. */
{
	return (IORD_32DIRECT(pixel_buffer->base, 12) & 0x1);
}

void alt_up_pixel_buffer_dma_clear_screen(alt_up_pixel_buffer_dma_dev *pixel_buffer, int backbuffer)
/* This function clears the screen by setting each pixel to a black color. */
{
	register unsigned int addr;
	register unsigned int limit_x, limit_y;
	
	/* Set up the address to start clearing from and the screen boundaries. */
	if (backbuffer == 1)
		addr = pixel_buffer->back_buffer_start_address;
	else
		addr = pixel_buffer->buffer_start_address;
	limit_x = pixel_buffer->x_resolution;
	/* In 16 and 32-bit color modes we use twice or four times more memory for the display buffer.*/
	if (pixel_buffer->color_mode == ALT_UP_16BIT_COLOR_MODE) {
		limit_x = limit_x << 1;
	} else {
		limit_x = limit_x << 2;
	}	
	limit_y = pixel_buffer->y_resolution;

	if (pixel_buffer->addressing_mode == ALT_UP_PIXEL_BUFFER_XY_ADDRESS_MODE) {
		/* Clear the screen when the VGA is set up in an XY addressing mode. */
		register unsigned int x,y;
		register unsigned int offset_y;
		offset_y = pixel_buffer->y_coord_offset;

		for (y = 0; y < limit_y; y++)
		{
			for (x = 0; x < limit_x; x = x + 4)
			{
				IOWR_32DIRECT(addr, x, 0);
			}
			addr = addr + (1 << offset_y);
		}
	} else {
		/* Clear the screen when the VGA is set up in a linear addressing mode. */
		register int x;
		limit_y = limit_x*limit_y;	

		for (x = 0; x < limit_y; x = x + 4)
		{
			IOWR_32DIRECT(addr, x, 0);
		}
	}
}

void alt_up_pixel_buffer_dma_draw_box(alt_up_pixel_buffer_dma_dev *pixel_buffer, int x0, int y0, int x1, int y1, int color, int backbuffer)
/* This function draws a filled box. */
{
	register unsigned int addr;
	register unsigned int limit_x = pixel_buffer->x_resolution;
	register unsigned int limit_y = pixel_buffer->y_resolution;
	register unsigned int temp;
	register unsigned int l_x = x0;
	register unsigned int r_x = x1;
	register unsigned int t_y = y0;
	register unsigned int b_y = y1;
	register unsigned int local_color = color;
	
	/* Check coordinates */
	if (l_x > r_x)
	{
		temp = l_x;
		l_x = r_x;
		r_x = temp;
	}
	if (t_y > b_y)
	{
		temp = t_y;
		t_y = b_y;
		b_y = temp;
	}
	if ((l_x >= limit_x) || (t_y >= limit_y) || (r_x < 0) || (b_y < 0))
	{
		/* Drawing outside of the window, so don't bother. */
		return;
	}
	/* Clip the box and draw only within the confines of the screen. */
	if (l_x < 0)
	{
		l_x = 0;
	}
	if (r_x >= limit_x)
	{
		r_x = limit_x - 1;
	}
	if (t_y < 0)
	{
		t_y = 0;
	}
	if (b_y >= limit_y)
	{
		b_y = limit_y - 1;
	}

	/* Set up the address to start clearing from and the screen boundaries. */
	if (backbuffer == 1)
		addr = pixel_buffer->back_buffer_start_address;
	else
		addr = pixel_buffer->buffer_start_address;

	/* Draw the box using one of the addressing modes. */
	if (pixel_buffer->addressing_mode == ALT_UP_PIXEL_BUFFER_XY_ADDRESS_MODE) {
		/* Draw a box of a given color on the screen using the XY addressing mode. */
		register unsigned int x,y;
		register unsigned int offset_y;
		offset_y = pixel_buffer->y_coord_offset;
		addr = addr + (t_y << offset_y);
		
		/* This portion of the code is purposefully replicated. This is because having a text for
		 * the mode would unnecessarily slow down the drawing of a box. */
		if (pixel_buffer->color_mode == ALT_UP_8BIT_COLOR_MODE) {
			for (y = t_y; y <= b_y; y++)
			{
				for (x = l_x; x <= r_x; x++)
				{
					IOWR_8DIRECT(addr, x, local_color);
				}
				addr = addr + (1 << offset_y);
			}
		} else if (pixel_buffer->color_mode == ALT_UP_16BIT_COLOR_MODE) {
			for (y = t_y; y <= b_y; y++)
			{
				for (x = l_x; x <= r_x; x++)
				{
					IOWR_16DIRECT(addr, x << 1, local_color);
				}
				addr = addr + (1 << offset_y);
			}
		}
		else
		{
			for (y = t_y; y <= b_y; y++)
			{
				for (x = l_x; x <= r_x; x++)
				{
					IOWR_32DIRECT(addr, x << 2, local_color);
				}
				addr = addr + (1 << offset_y);
			}
		}
	} else {
		/* Draw a box of a given color on the screen using the linear addressing mode. */
		register unsigned int x,y;
		/* This portion of the code is purposefully replicated. This is because having a text for
		 * the mode would unnecessarily slow down the drawing of a box. */
		if (pixel_buffer->color_mode == ALT_UP_8BIT_COLOR_MODE) {
			addr = addr + t_y * limit_x;
			for (y = t_y; y <= b_y; y++)
			{
				for (x = l_x; x <= r_x; x++)
				{
					IOWR_8DIRECT(addr, x, local_color);
				}
				addr = addr + limit_x;
			}
		} else if (pixel_buffer->color_mode == ALT_UP_16BIT_COLOR_MODE) {
			limit_x = limit_x << 1;
			addr = addr + t_y * limit_x;
			for (y = t_y; y <= b_y; y++)
			{
				for (x = l_x; x <= r_x; x++)
				{
					IOWR_16DIRECT(addr, x << 1, local_color);
				}
				addr = addr + limit_x;
			}
		}
		else
		{
			limit_x = limit_x << 2;
			addr = addr + t_y * limit_x;
			for (y = t_y; y <= b_y; y++)
			{
				for (x = l_x; x <= r_x; x++)
				{
					IOWR_32DIRECT(addr, x << 2, local_color);
				}
				addr = addr + limit_x;
			}
		}
	}
}

void alt_up_pixel_buffer_dma_draw_hline(alt_up_pixel_buffer_dma_dev *pixel_buffer, int x0, int x1, int y, int color, int backbuffer)
/* This method draws a horizontal line. This method is faster than using the line method because we know the direction of the line. */
{
	register unsigned int addr;
	register unsigned int limit_x = pixel_buffer->x_resolution;
	register unsigned int limit_y = pixel_buffer->y_resolution;
	register unsigned int temp;
	register unsigned int l_x = x0;
	register unsigned int r_x = x1;
	register unsigned int line_y = y;
	register unsigned int local_color = color;
	
	/* Check coordinates */
	if (l_x > r_x)
	{
		temp = l_x;
		l_x = r_x;
		r_x = temp;
	}
	if ((l_x >= limit_x) || (line_y >= limit_y) || (r_x < 0) || (line_y < 0))
	{
		/* Drawing outside of the window, so don't bother. */
		return;
	}
	/* Clip the box and draw only within the confines of the screen. */
	if (l_x < 0)
	{
		l_x = 0;
	}
	if (r_x >= limit_x)
	{
		r_x = limit_x - 1;
	}

	/* Set up the address to start clearing from and the screen boundaries. */
	if (backbuffer == 1)
		addr = pixel_buffer->back_buffer_start_address;
	else
		addr = pixel_buffer->buffer_start_address;

	/* Draw a horizontal line using one of the addressing modes. */
	if (pixel_buffer->addressing_mode == ALT_UP_PIXEL_BUFFER_XY_ADDRESS_MODE) {
		/* Draw a horizontal line of a given color on the screen using the XY addressing mode. */
		register unsigned int x;
		register unsigned int offset_y;
		offset_y = pixel_buffer->y_coord_offset;
		addr = addr + (line_y << offset_y);
		
		/* This portion of the code is purposefully replicated. This is because having a text for
		 * the mode would unnecessarily slow down the drawing of a horizontal line. */
		if (pixel_buffer->color_mode == ALT_UP_8BIT_COLOR_MODE) {
			for (x = l_x; x <= r_x; x++)
			{
				IOWR_8DIRECT(addr, x, local_color);
			}
		} else if (pixel_buffer->color_mode == ALT_UP_16BIT_COLOR_MODE) {
			for (x = l_x; x <= r_x; x++)
			{
				IOWR_16DIRECT(addr, x << 1, local_color);
			}
		}
		else
		{
			for (x = l_x; x <= r_x; x++)
			{
				IOWR_32DIRECT(addr, x << 2, local_color);
			}
		}
	} else {
		/* Draw a horizontal line of a given color on the screen using the linear addressing mode. */
		register unsigned int x;
		/* This portion of the code is purposefully replicated. This is because having a text for
		 * the mode would unnecessarily slow down the drawing of a box. */
		if (pixel_buffer->color_mode == ALT_UP_8BIT_COLOR_MODE) {
			addr = addr + line_y * limit_x;
			for (x = l_x; x <= r_x; x++)
			{
				IOWR_8DIRECT(addr, x, local_color);
			}
		} else if (pixel_buffer->color_mode == ALT_UP_16BIT_COLOR_MODE) {
			limit_x = limit_x << 1;
			addr = addr + line_y * limit_x;
			for (x = l_x; x <= r_x; x++)
			{
				IOWR_16DIRECT(addr, x << 1, local_color);
			}
		}
		else
		{
			limit_x = limit_x << 2;
			addr = addr + line_y * limit_x;
			for (x = l_x; x <= r_x; x++)
			{
				IOWR_32DIRECT(addr, x << 2, local_color);
			}
			addr = addr + limit_x;
		}
	}
}


void alt_up_pixel_buffer_dma_draw_vline(alt_up_pixel_buffer_dma_dev *pixel_buffer, int x, int y0, int y1, int color, int backbuffer)
/* This method draws a vertical line. This method is faster than using the line method because we know the direction of the line. */

{
	register unsigned int addr;
	register unsigned int limit_x = pixel_buffer->x_resolution;
	register unsigned int limit_y = pixel_buffer->y_resolution;
	register unsigned int temp;
	register unsigned int line_x = x;
	register unsigned int t_y = y0;
	register unsigned int b_y = y1;
	register unsigned int local_color = color;
	
	/* Check coordinates */
	if (t_y > b_y)
	{
		temp = t_y;
		t_y = b_y;
		b_y = temp;
	}
	if ((line_x >= limit_x) || (t_y >= limit_y) || (line_x < 0) || (b_y < 0))
	{
		/* Drawing outside of the window, so don't bother. */
		return;
	}
	/* Clip the box and draw only within the confines of the screen. */
	if (t_y < 0)
	{
		t_y = 0;
	}
	if (b_y >= limit_y)
	{
		b_y = limit_y - 1;
	}

	/* Set up the address to start clearing from and the screen boundaries. */
	if (backbuffer == 1)
		addr = pixel_buffer->back_buffer_start_address;
	else
		addr = pixel_buffer->buffer_start_address;

	/* Draw the vertical line using one of the addressing modes. */
	if (pixel_buffer->addressing_mode == ALT_UP_PIXEL_BUFFER_XY_ADDRESS_MODE) {
		/* Draw a vertical line of a given color on the screen using the XY addressing mode. */
		register unsigned int y;
		register unsigned int offset_y;
		offset_y = pixel_buffer->y_coord_offset;
		addr = addr + (t_y << offset_y);
		
		/* This portion of the code is purposefully replicated. This is because having a text for
		 * the mode would unnecessarily slow down the drawing of a box. */
		if (pixel_buffer->color_mode == ALT_UP_8BIT_COLOR_MODE) {
			for (y = t_y; y <= b_y; y++)
			{
				IOWR_8DIRECT(addr, line_x, local_color);
				addr = addr + (1 << offset_y);
			}
		} else if (pixel_buffer->color_mode == ALT_UP_16BIT_COLOR_MODE) {
			for (y = t_y; y <= b_y; y++)
			{
				IOWR_16DIRECT(addr, line_x << 1, local_color);
				addr = addr + (1 << offset_y);
			}
		}
		else
		{
			for (y = t_y; y <= b_y; y++)
			{
				IOWR_32DIRECT(addr, line_x << 2, local_color);
				addr = addr + (1 << offset_y);
			}
		}
	} else {
		/* Draw a vertical line of a given color on the screen using the linear addressing mode. */
		register unsigned int y;
		/* This portion of the code is purposefully replicated. This is because having a text for
		 * the mode would unnecessarily slow down the drawing of a box. */
		if (pixel_buffer->color_mode == ALT_UP_8BIT_COLOR_MODE) {
			addr = addr + t_y * limit_x;
			for (y = t_y; y <= b_y; y++)
			{
				IOWR_8DIRECT(addr, line_x, local_color);
				addr = addr + limit_x;
			}
		} else if (pixel_buffer->color_mode == ALT_UP_16BIT_COLOR_MODE) {
			limit_x = limit_x << 1;
			addr = addr + t_y * limit_x;
			for (y = t_y; y <= b_y; y++)
			{
				IOWR_16DIRECT(addr, line_x << 1, local_color);
				addr = addr + limit_x;
			}
		}
		else
		{
			limit_x = limit_x << 2;
			addr = addr + t_y * limit_x;
			for (y = t_y; y <= b_y; y++)
			{
				IOWR_32DIRECT(addr, line_x << 2, local_color);
				addr = addr + limit_x;
			}
		}
	}
}

void alt_up_pixel_buffer_dma_draw_rectangle(alt_up_pixel_buffer_dma_dev *pixel_buffer, int x0, int y0, int x1, int y1, int color, int backbuffer)
{
	alt_up_pixel_buffer_dma_draw_hline(pixel_buffer, x0, x1, y0, color, backbuffer);
	alt_up_pixel_buffer_dma_draw_hline(pixel_buffer, x0, x1, y1, color, backbuffer);
	alt_up_pixel_buffer_dma_draw_vline(pixel_buffer, x0, y0, y1, color, backbuffer);
	alt_up_pixel_buffer_dma_draw_vline(pixel_buffer, x1, y0, y1, color, backbuffer);
}

void helper_plot_pixel(register unsigned int buffer_start, register int line_size, register int x, register int y, register int color, register int mode)
/* This is a helper function that draws a pixel at a given location. Note that no boundary checks are made,
 * so drawing off-screen may cause unpredictable side effects. */
{
	if (mode == 0)
		IOWR_8DIRECT(buffer_start, line_size*y+x, color);
	else if (mode == 1)
		IOWR_16DIRECT(buffer_start, (line_size*y+x) << 1, color);
	else
		IOWR_32DIRECT(buffer_start, (line_size*y+x) << 2, color);
}

void alt_up_pixel_buffer_dma_draw_line(alt_up_pixel_buffer_dma_dev *pixel_buffer, int x0, int y0, int x1, int y1, int color, int backbuffer)
/* This function draws a line between points (x0, y0) and (x1, y1). The function does not check if it draws a pixel within screen boundaries.
 * users should ensure that the line is drawn within the screen boundaries. */
{
	register int x_0 = x0;
	register int y_0 = y0;
	register int x_1 = x1;
	register int y_1 = y1;
	register char steep = (ABS(y_1 - y_0) > ABS(x_1 - x_0)) ? 1 : 0;
	register int deltax, deltay, error, ystep, x, y;
	register int color_mode =	(pixel_buffer->color_mode == ALT_UP_8BIT_COLOR_MODE) ? 0 :
								(pixel_buffer->color_mode == ALT_UP_16BIT_COLOR_MODE) ? 1 : 2;
	register int line_color = color;
	register unsigned int buffer_start;
	register int line_size = (pixel_buffer->addressing_mode == ALT_UP_PIXEL_BUFFER_XY_ADDRESS_MODE) ? (1 << (pixel_buffer->y_coord_offset-color_mode)) : pixel_buffer->x_resolution;

	if (backbuffer == 1)
		buffer_start = pixel_buffer->back_buffer_start_address;
	else
		buffer_start = pixel_buffer->buffer_start_address;

	/* Preprocessing inputs */
	if (steep > 0) {
		// Swap x_0 and y_0
		error = x_0;
		x_0 = y_0;
		y_0 = error;
		// Swap x_1 and y_1
		error = x_1;
		x_1 = y_1;
		y_1 = error;
	}
	if (x_0 > x_1) {
		// Swap x_0 and x_1
		error = x_0;
		x_0 = x_1;
		x_1 = error;
		// Swap y_0 and y_1
		error = y_0;
		y_0 = y_1;
		y_1 = error;
	}

	/* Setup local variables */
	deltax = x_1 - x_0;
	deltay = ABS(y_1 - y_0);
	error = -(deltax / 2); 
	y = y_0;
	if (y_0 < y_1)
		ystep = 1;
	else
		ystep = -1;

	/* Draw a line - either go along the x axis (steep = 0) or along the y axis (steep = 1). The code is replicated to
	 * compile well on low optimization levels. */
	if (steep == 1)
	{
		for (x=x_0; x <= x_1; x++) {
			helper_plot_pixel(buffer_start, line_size, y, x, line_color, color_mode);
			error = error + deltay;
			if (error > 0) {
				y = y + ystep;
				error = error - deltax;
			}
		}
	}
	else
	{
		for (x=x_0; x <= x_1; x++) {
			helper_plot_pixel(buffer_start, line_size, x, y, line_color, color_mode);
			error = error + deltay;
			if (error > 0) {
				y = y + ystep;
				error = error - deltax;
			}
		}
	}
}

