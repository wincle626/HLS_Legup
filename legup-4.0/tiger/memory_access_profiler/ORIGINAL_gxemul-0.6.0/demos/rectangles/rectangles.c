/*
 *  $Id: rectangles.c,v 1.5 2006-07-09 07:53:33 debug Exp $
 *
 *  GXemul demo:  Random rectangles
 *
 *  This file is in the Public Domain.
 */

#include "dev_fb.h"


#ifdef MIPS
/*  Note: The ugly cast to a signed int (32-bit) causes the address to be
	sign-extended correctly on MIPS when compiled in 64-bit mode  */ 
#define PHYSADDR_OFFSET         ((signed int)0xa0000000)
#else
#define PHYSADDR_OFFSET         0
#endif


/*  Framebuffer linear memory and controller base addresss:  */
#define FB_BASE			(PHYSADDR_OFFSET + DEV_FB_ADDRESS)
#define FBCTRL_BASE		(PHYSADDR_OFFSET + DEV_FBCTRL_ADDRESS)


#define	XRES	800
#define	YRES	600


void my_memset(unsigned char *a, int x, int len)
{
	while (len-- > 0)
		*a++ = x;
}


void draw_rectangle(int x1, int y1, int x2, int y2, int c)
{
	int y, len;

	for (y=y1; y<=y2; y++) {
		len = 3 * (x2-x1+1);
		if (len > 0) {
			my_memset((unsigned char *)FB_BASE +
			    3 * (XRES * y + x1), c, len);
		}
	}
}


unsigned int my_random()
{
	static int a = 0x124879b;
	static int b = 0xb7856fa2;
	int c = a ^ (b * 51);
	a = 17 * c - (b * 171);
	return c;
}


void fbctrl_write_port(int p)
{
	*(volatile int *)(FBCTRL_BASE + DEV_FBCTRL_PORT) = p;
}


void fbctrl_write_data(int d)
{
	*(volatile int *)(FBCTRL_BASE + DEV_FBCTRL_DATA) = d;
}


void fbctrl_set_x1(int v)
{
	fbctrl_write_port(DEV_FBCTRL_PORT_X1);
	fbctrl_write_data(v);
}


void fbctrl_set_y1(int v)
{
	fbctrl_write_port(DEV_FBCTRL_PORT_Y1);
	fbctrl_write_data(v);
}


void fbctrl_command(int c)
{
	fbctrl_write_port(DEV_FBCTRL_PORT_COMMAND);
	fbctrl_write_data(c);
}


void change_resolution(int xres, int yres)
{
	fbctrl_set_x1(xres);
	fbctrl_set_y1(yres);
	fbctrl_command(DEV_FBCTRL_COMMAND_SET_RESOLUTION);
}


void f(void)
{
	/*  Change to the resolution we want:  */
	change_resolution(XRES, YRES);

	/*  Draw random rectangles forever:  */
	for (;;)  {
		draw_rectangle(my_random() % XRES, my_random() % YRES,
		    my_random() % XRES, my_random() % YRES, my_random());
	}
}

