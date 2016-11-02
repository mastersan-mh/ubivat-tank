/*
 * fonts.h
 *
 * Created on: 11 окт. 2016 г.
 * Author: mastersan
 */

#ifndef SRC__FONTS_H_
#define SRC__FONTS_H_

#define COLOR_1   0x00, 0x00, 0x2a
#define COLOR_4   0x2a, 0x00, 0x00
#define COLOR_7   0x2a, 0x2a, 0x2a
#define COLOR_13  0x3f, 0x15, 0x3f
#define COLOR_15  0x3f, 0x3f, 0x3f
#define COLOR_25  0x20, 0x20, 0x20

#define COLOR_BLUE COLOR_1
#define COLOR_RED  COLOR_4
#define COLOR_GRAY COLOR_7
#define COLOR_LIGHTGRAY COLOR_15
#define COLOR_DARKGRAY COLOR_25

enum text_orient_e
{
	orient_horiz,
	orient_vert
};

void fonts_init();
void fonts_done();


extern void font_color_set(
	uint8_t R,
	uint8_t G,
	uint8_t B
);

int video_print_char(int out_x, int out_y, char ch);

void video_printf(
	int px,
	int py,
	enum text_orient_e orientation,
	const char * format,
	...
	);


#endif /* SRC__FONTS_H_ */
