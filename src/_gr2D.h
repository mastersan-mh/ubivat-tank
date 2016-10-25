/*
 * _gr2D.h
 *
 * Created on: 11 окт. 2016 г.
 * Author: mastersan
 */

#ifndef SRC__GR2D_H_
#define SRC__GR2D_H_

#include <img.h>

#define gr2D_PI  (3.14159265358979323846)

typedef struct
{
	char current; //текущий цвет
} T2Dcolor;

typedef struct
{
	int x0;      //координата X0
	int y0;      //координата Y0
	int x1;      //координата X1
	int y1;      //координата Y1
}T2DWIN;

typedef struct
{
	T2Dcolor color;    //цвета
} Tgr2D;

extern Tgr2D gr2D;

/********************************************************************/
void gr2D_setpixel(int x, int y);
/********************************************************************/
void gr2D_line (int x0, int y0, int x1, int y1);
void gr2D_line_h (int x0, int x1, int y);
void gr2D_line_v (int x, int y0, int y1);
void gr2D_rectangle_e(int x0, int y0, int sx, int sy);
void gr2D_rectangle_f(int x0, int y0, int sx, int sy);

void gr2D_setline(int x, int y, int length, char * bytemap);
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
