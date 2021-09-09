/*
 * img.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_IMG_H_
#define SRC_IMG_H_

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#define BII_NOERR     0
#define BII_FATAL     1
#define BII_FILEERR   2
#define BII_READERR   3
#define BII_FORMATERR 4
#define BII_UNKNOWN   5

typedef enum
{
	IMG_MENU_LOGO,
	M_CONBACK,
	M_I_INTERLV,
	M_GAME,
	M_G_NEW_P1,
	M_G_NEW_P2,
	M_G_LOAD,
	M_G_SAVE,
	M_CASE,
	M_OPTIONS,
	M_ABOUT,
	M_ABORT,
	M_QUIT,
	M_CUR_0,
	M_CUR_1,
	M_ARROWL,
	M_ARROWR,
	M_LINEL,
	M_LINEM,
	M_LINER,
	TANK0,
	TANK1,
	TANK2,
	TANK3,
	TANK4,

	F_RUS,
	F_USA,
	F_WHITE,

	IMG_WALL_W0,
	IMG_WALL_W1,
	IMG_WALL_BRICK,
	IMG_WATER0,
	IMG_WATER1,
	IMG_WATER2,
	I_HEALTH,
	I_ARMOR,
	I_STAR,
	O_EXIT,
	W_BULL,
	W_ROCKET,
	W_MINE,
	B_BULL,
	B_ROCKET,
	B_MINE,
	E_SMALL,
	E_BIG,
	__IMAGE_NUM
}image_index_t;


//рисунок
typedef struct
{
	//рисунок
	void * pic;
	//размер_рисунка_X
	int sx;
	//размер_рисунка_Y
	int sy;
} BIIpic_t;

/*
 * изображение
 */
typedef struct
{
	//изображение
	GLenum sfactor;
	GLenum dfactor;
	int img_sx;
	int img_sy;
	GLuint texture;
	GLsizei texture_sx;
	GLsizei texture_sy;
	uint8_t * data;
} item_img_t;

extern int BII_errno;

char * IMG_errorGet();

int img_palette_read(const char * filename);

void images_done();

void images_init();

item_img_t * image_get(image_index_t iimage);


#endif /* SRC_IMG_H_ */
