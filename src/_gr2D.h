/*
 * _gr2D.h
 *
 * Created on: 11 окт. 2016 г.
 * Author: mastersan
 */

#ifndef SRC__GR2D_H_
#define SRC__GR2D_H_

#include <img.h>

#define gr2D_SCR_sx (320)
#define gr2D_SCR_sy (200)
#define gr2D_PI  (3.14159265358979323846)

typedef struct
{
	char current; //текущий цвет
	int transparent; //прозрачный цвет(-1 -прозрачного цвета нет)
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
	char R,G,B;
} T2DRGBcolor;

typedef T2DRGBcolor T2Dpal[256];
typedef struct
{
	void * BUFdefault;   //основной буфер
	void * BUFcurrent;   //указатель на используемый буфер
	T2DWIN WIN;   //настройки окна в котором разрешено заполнение
	T2Dcolor color;    //цвета
	T2Dpal PAL;      //палитра
} Tgr2D;

extern Tgr2D gr2D;

int gr2D_init320X200X8();
int gr2D_close();
void gr2D_BUFcurrentfill();
void gr2D_BUFcurrent2screen();
/********************************************************************/
void gr2D_setpixel(int x, int y);
void gr2D_WINsetpixel(int x, int y);
char gr2D_getpixel(int x, int y);
/********************************************************************/
void gr2D_setRGBcolor(char color, char r, char g, char b);
void gr2D_getRGBcolor(char color, char * r, char * g, char * b);
void gr2D_setRGBpal (T2Dpal * p);
void gr2D_getRGBpal (T2Dpal * p);
void gr2D_line (int x0, int y0, int x1, int y1);
void gr2D_line_h (int x0, int x1, int y);
void gr2D_line_v (int x, int y0, int y1);
void gr2D_rectangle_e(int x0, int y0, int sx, int sy);
void gr2D_rectangle_f(int x0, int y0, int sx, int sy);
void gr2D_triangle_e (int x0, int y0, int x1, int y1, int x2, int y2);

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
