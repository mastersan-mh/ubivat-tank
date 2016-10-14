/*
 * img.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_IMG_H_
#define SRC_IMG_H_

item_img_t * IMG_find(item_img_t * HEAD, const char * IMGname);
int IMG_add(item_img_t ** HEAD, const char * path, const char * IMGname);
void IMG_removeall(item_img_t ** HEAD);
item_img_t * IMG_connect(item_img_t * HEAD, const char * IMGname);


#endif /* SRC_IMG_H_ */
