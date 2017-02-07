/*
 * _gr2D.h
 *
 * Created on: 11 окт. 2016 г.
 * Author: mastersan
 */

#ifndef SRC__GR2D_H_
#define SRC__GR2D_H_

#include "img.h"

void gr2D_setimage0(
	int out_x,
	int out_y,
	item_img_t * image
	);

void gr2D_setimage_indexed(
	int x,
	int y,
	unsigned int sx,
	unsigned int sy,
	void * indexed_image
);

extern void gr2D_color_index(int);
extern void gr2D_line_h(int, int, int);
void gr2D_line_v(int, int, int);
void gr2D_rectangle_e(int, int, int, int);
void gr2D_rectangle_f(int, int, int, int);
void gr2D_setpixel(int, int);

#endif /* SRC__GR2D_H_ */
