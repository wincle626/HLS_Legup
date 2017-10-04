#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "legup_mem.h"
 
void set_texture();
 
typedef struct {unsigned char r, g, b;} rgb_t;
rgb_t **tex = NULL;
rgb_t *SHARED_tex = NULL;
int gwin;
GLuint texture;
int width, height;
int tex_w, tex_h;
double scale = 1./256;
double cx = -0.743643887037158704752191506114774, cy = 0.131825904205311970493132056385139;
int max_iter = 2048;
 
void render()
{
	double	x = (double)width /tex_w,
		y = (double)height/tex_h;
 
	glClear(GL_COLOR_BUFFER_BIT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
 
	glBindTexture(GL_TEXTURE_2D, texture);
 
	glBegin(GL_QUADS);
 
	glTexCoord2f(0, 0); glVertex2i(0, 0);
	glTexCoord2f(x, 0); glVertex2i(width, 0);
	glTexCoord2f(x, y); glVertex2i(width, height);
	glTexCoord2f(0, y); glVertex2i(0, height);
 
	glEnd();
 
	glFlush();
	glFinish();
}
 
void hsv_to_rgb(int hue, int min, int max, rgb_t *p)
{
	if (min == max) max = min + 1;
	int h = (int)(1e-4 + 4.0 * (hue - min) / (max - min)) % 6;
#	define VAL 255
	double c = VAL;
	int i = (int)c;
#	define ABS(x) ((x < 0) ? -x : x)
	int X = (int)(c - ABS((int)(c*(1e-4 + 4.0 * (hue -min) / (max - min)) - c)));
 
	p->r = p->g = p->b = 0;
 
	switch(h) {
	case 0: p->r = i; p->g = X; return;
	case 1:	p->r = X; p->g = i; return;
	case 2: p->g = i; p->b = X; return;
	case 3: p->g = X; p->b = i; return;
	case 4: p->r = X; p->b = i; return;
	default:p->r = i; p->b = X;
	}
}
 
void calc_mandel(double mandel_scale, double mandel_cx, double mandel_cy, rgb_t *mandel_tex, int mandel_tex_w,
		int mandel_max_iter, int mandel_width, int mandel_height_start, int mandel_height_end, int mandel_height)
{
	int i, j, iter, min, max;
	double x, y, zx, zy, zx2, zy2;
	for (i = mandel_height_start; i < mandel_height_end; i++) {
		y = (i - mandel_height/2) * mandel_scale + mandel_cy;
		for (j = 0; j  < mandel_width; j++, mandel_tex++) {
			x = (j - mandel_width/2) * mandel_scale + mandel_cx;
			iter = 0;
 
			if ((x + 1)*(x + 1) + y * y < 1/16) iter = mandel_max_iter;
 
			zx = zy = zx2 = zy2 = 0;
			for (; iter < mandel_max_iter && zx2 + zy2 < 4; iter++) {
				zy = 2 * zx * zy + y;
				zx = zx2 - zy2 + x;
				zx2 = zx * zx;
				zy2 = zy * zy;
			}
			hsv_to_rgb(iter, 0, mandel_max_iter, mandel_tex);
		}
		mandel_tex += (mandel_tex_w - mandel_width);
	}
}
 
#define MIN(a,b) ((a <= b) ? a : b)
#define BLOCK_SIZE 750000

void alloc_tex()
{
	int i, ow = tex_w, oh = tex_h;
 
	for (tex_w = 1; tex_w < width;  tex_w <<= 1);
	for (tex_h = 1; tex_h < height; tex_h <<= 1);
 
	if (tex_h != oh || tex_w != ow) {
		tex = realloc(tex, tex_h * tex_w * 3 + tex_h * sizeof(rgb_t*));
		free_shared(SHARED_tex);
		SHARED_tex = malloc_shared(MIN(tex_h * tex_w * 3, BLOCK_SIZE), tex + tex_h, LEGUP_RAM_LOCATION_ONCHIP);
	}
 
	for (tex[0] = (rgb_t *)(tex + tex_h), i = 1; i < tex_h; i++)
		tex[i] = tex[i - 1] + tex_w;
}
 
void set_texture()
{
	alloc_tex();

	int height_delta = BLOCK_SIZE / tex_w / 3;
	int i;

	/* Since the pcie hybrid version reuses the same memory to compute different portions of the image,
	the SW_ONLY executable should use a non-zero OFFSET*/
#ifdef SW_ONLY
#define OFFSET (i*tex_w)
#else
#define OFFSET (0)
#endif
	for (i = 0; i < height; i += height_delta) {
		calc_mandel(scale, cx, cy, SHARED_tex+OFFSET, tex_w, max_iter, width, i, MIN(i+height_delta,height), height);
		memcpy_from_shared(tex[i], SHARED_tex, height_delta * tex_w * 3);
	}
 
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, tex_w, tex_h,
		0, GL_RGB, GL_UNSIGNED_BYTE, tex[0]);
 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	render();
}
 
 
void resize(int w, int h)
{
	width = w;
	height = h;
 
	glViewport(0, 0, w, h);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);
 
	set_texture();
}

void idle()
{
	scale *= 0.75;

	if (scale < 1e-14) {
		glFinish();
		glutDestroyWindow(gwin);
		return;
	}

	set_texture();
}
 
void init_gfx(int *c, char **v)
{
	glutInit(c, v);
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(1024, 1024);
	glutDisplayFunc(render);
 
	gwin = glutCreateWindow("Mandelbrot");
 
	glutReshapeFunc(resize);
	glutIdleFunc(idle);
	glGenTextures(1, &texture);
	set_texture();
}
 
int main(int c, char **v)
{
	init_gfx(&c, v);
 
	glutMainLoop();
	return 0;
}
