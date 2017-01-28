/*
 *        Ubivat Tank
 *        константы, типы, переменные
 * by Master San
 */

#ifndef SRC_TYPES_H_
#define SRC_TYPES_H_

#define _DEBUG
#define _DEBUG_PLAYERMAXLEVEL

#define MAX_MESSAGE_SIZE 2048

#define ATTR_PACKED __attribute__ ((packed))

#include "config.h"
#include "vec.h"
#include "fonts.h"
#include "Z_mem.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>


#ifndef bool
typedef enum {
	false = 0,
	true = 1
}bool;
#endif

/* тип переменной entity */
typedef enum
{
	ENTITYVARTYPE_INTEGER,
	ENTITYVARTYPE_FLOAT,
	ENTITYVARTYPE_STRING
} entityvartype_t;

typedef int64_t entity_int_t;
typedef float entity_float_t;
typedef char entity_char_t;
/* значение переменной */
typedef union
{
	entity_int_t i64;
	entity_float_t f;
	entity_char_t *string;
} entityvarvalue_t;


//директория файлов игры
#define BASEDIR         UTANKBASEDIR"/base"
//директория карт
#define MAPSDIR        "/maps"

// файлы пользователя
#define GAME_DIR_HOME "/.ubivat-tank"

#define GAME_DIR_CONF  GAME_DIR_HOME
//директория записей
#define GAME_DIR_SAVES GAME_DIR_HOME"/saves"

#define FILENAME_CONFIG "/config.cfg"
#define FILENAME_MAPSLIST "/maps.lst"
#define FILENAME_PALETTE "/palette.pal"
//коды
#define code_levelup "IAMSUPERTANK"
#define code_health  "GIVEHEALTH"
#define code_armor   "GIVEARMOR"
#define code_ammo    "GIVEAMMO"
#define code_all     "GIVEALL"
//разрешенные для ввода символы
#define chars_allowed "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"


//предметы
//размер предмета
#define c_item_MDL_box     16
//позиция изображения
#define c_i_MDL_pos     (-8)
//объекты
//размер объекта
#define c_o_MDL_box     16
//позиция изображения
#define c_o_MDL_pos     (-8)
//body box
#define c_p_MDL_box     16
//позиция изображения
#define c_p_MDL_pos     (-8)
//время на перезарядку, мс
#define c_p_WEAP_reloadtime 1000
//время "думания" бота, мс
#define c_BOT_time      1500
//расстояние с которого бот будет стремиться вести атаку
#define c_BOT_dist      (c_p_MDL_box*3)

typedef void (*actionf_t)();

//камера
typedef struct
{
	//координаты
	vec2_t pos;
	//позиция на экране
	float x;
	float y;
	//ширина и высота
	float sx;
	float sy;
}camera_t;

#define ARRAYSIZE( x ) ( sizeof( (x) )/sizeof( *(x) ) )

extern char * c_strTITLE;
extern char * c_strCORP;
extern coloredtext_t c_about[];

extern bool debug_noAI;

bool checkchar(char chr);

float sqrf(float v);

char * str_addch(char * s0, char ch);

void check_directory(const char * path);

#endif /* SRC_TYPES_H_ */

