/*
 * _gr2D.h
 *
 * Created on: 11 окт. 2016 г.
 * Author: mastersan
 */

#ifndef SRC__GR2D_H_
#define SRC__GR2D_H_

#include "img.h"

extern void gr2D_setimage00(
	int out_x,
	int out_y,
	const item_img_t * image,
	int scr_w,
	int scr_h
	);


#define video_image_draw(x, y, image) \
		gr2D_setimage00((x), (y), image_get((image)), 0, 0)

#endif /* SRC__GR2D_H_ */
