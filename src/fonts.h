/*
 * fonts.h
 *
 * Created on: 11 окт. 2016 г.
 * Author: mastersan
 */

#ifndef SRC__FONTS_H_
#define SRC__FONTS_H_

enum text_orient_e
{
	orient_horiz,
	orient_vert
};

void fonts_init();
void fonts_done();


int video_print_char(int out_x, int out_y, char ch);

void video_printf(
	int px,
	int py,
	enum text_orient_e orientation,
	const char * format,
	...
	);

#endif /* SRC__FONTS_H_ */
