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
	IMG_MENU_CONBACK,
	IMG_MENU_I_INTERLV,
	IMG_MENU_GAME,
	IMG_MENU_G_NEW_P1,
	IMG_MENU_G_NEW_P2,
	IMG_MENU_G_LOAD,
	IMG_MENU_G_SAVE,
	IMG_MENU_CASE,
	IMG_MENU_CASE_SERVERCONNECT,
	IMG_MENU_OPTIONS,
	IMG_MENU_ABOUT,
	IMG_MENU_ABORT,
	IMG_MENU_QUIT,
	IMG_MENU_CUR_0,
	IMG_MENU_CUR_1,
	IMG_MENU_ARROWL,
	IMG_MENU_ARROWR,
	IMG_MENU_LINEL,
	IMG_MENU_LINEM,
	IMG_MENU_LINER,
	IMG_TANK0,
	IMG_TANK1,
	IMG_TANK2,
	IMG_TANK3,
	IMG_TANK4,

	IMG_FLAG_RUS,
	IMG_FLAG_USA,
	IMG_FLAG_WHITE,

	IMG_WALL_W0,
	IMG_WALL_W1,
	IMG_WALL_BRICK,
	IMG_WATER0,
	IMG_WATER1,
	IMG_WATER2,
	IMG_ITEM_HEALTH,
	IMG_ITEM_ARMOR,
	IMG_ITEM_STAR,
	IMG_OBJ_EXIT,
	IMG_WEAPON_ARTILLERY,
	IMG_WEAPON_MISSILE,
	IMG_WEAPON_MINE,
	IMG_BULL_ARTILLERY,
	IMG_BULL_MISSILE,
	IMG_BULL_MINE,
	IMG_EXPLODE_SMALL,
	IMG_EXPLODE_BIG,
	IMG_HUD_ICON_TANK0,
	IMG_HUD_ICON_TANK1,
	IMG_HUD_ICON_TANK2,
	IMG_HUD_ICON_TANK3,
	IMG_HUD_ICON_TANK4,
	IMG__NUM
} image_index_t;


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

const item_img_t * image_get(image_index_t iimage);


#endif /* SRC_IMG_H_ */
