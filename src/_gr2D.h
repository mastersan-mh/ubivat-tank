/*
 * _gr2D.h
 *
 * Created on: 11 окт. 2016 г.
 * Author: mastersan
 */

#ifndef SRC__GR2D_H_
#define SRC__GR2D_H_

#include <img.h>

void gr2D_setimage0(
	int out_x,
	int out_y,
	item_img_t * image
	);
void gr2D_setimage1(
	int out_x,
	int out_y,
	item_img_t * image,
	int get_x,
	int get_y,
	int sx,
	int sy
	);

#endif /* SRC__GR2D_H_ */
