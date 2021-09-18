/*
 * fonts.h
 *
 * Created on: 11 окт. 2016 г.
 * Author: mastersan
 */

#ifndef SRC__FONTS_H_
#define SRC__FONTS_H_

#include <inttypes.h>

#define COLOR_0   0x00, 0x00, 0x00
#define COLOR_1   0x00, 0x00, 0x2a
#define COLOR_4   0x2a, 0x00, 0x00
#define COLOR_7   0x2a, 0x2a, 0x2a
#define COLOR_13  0x3f, 0x15, 0x3f
#define COLOR_15  0x3f, 0x3f, 0x3f
#define COLOR_25  0x20, 0x20, 0x20 /* white */

#define COLOR_BLUE COLOR_1
#define COLOR_RED  COLOR_4
#define COLOR_GRAY COLOR_7
#define COLOR_LIGHTGRAY COLOR_15
#define COLOR_DARKGRAY COLOR_25

typedef struct
{
	char r;
	char g;
	char b;
} color_t;

typedef struct
{
	color_t color;
	char * text;
} coloredtext_t;

enum text_orient_e
{
	orient_horiz,
	orient_vert
};

void fonts_test();

void fonts_init();
void fonts_done();


extern void font_color_set3i(
	uint8_t r,
	uint8_t g,
	uint8_t b
);
extern void font_color_sets(
	color_t * color
);

extern void font_color_setindex(
	uint8_t index
);

void video_print_char(int out_x, int out_y, char ch);

void video_printf(
	int px,
	int py,
	enum text_orient_e orientation,
	const char * format,
	...
);

/*
 * @param pixels_width    Wide of print field, pixels
 */
void video_printf_wide(
	int x,
	int y,
	int pixels_width,
	const char * format,
	...
);


#endif /* SRC__FONTS_H_ */
