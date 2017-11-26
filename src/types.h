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

#include "gamedefs.h"
#include "vec.h"
#include "fonts.h"
#include "Z_mem.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#include <stdbool.h>

#ifndef __bool_true_false_are_defined
typedef enum {
	false = 0,
	true = 1
} bool;
#endif

typedef enum direction_e
{
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT
} direction_t;

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

//время на перезарядку, мс
#define c_p_WEAP_reloadtime 1000
//время "думания" бота, мс
#define BOT_THINK_TIME      1500

//камера
typedef struct
{
	//координаты
	vec2_t origin;
	//позиция на экране
	float x;
	float y;
	//ширина и высота
	float sx;
	float sy;
} camera_t;

#define ARRAYSIZE( x ) ( sizeof( (x) )/sizeof( *(x) ) )
#define SIZEOF_MEMBER(type, member) sizeof( ((type *)0)->member )

/**
 * @brief Coerce value x in [min, max] range.
 */
#define COERCE(min, max, x) ((x) < (min) ? (min) : (x) > (max) ? (max) : (x))


extern char * c_strTITLE;
extern char * c_strCORP;
extern coloredtext_t c_about[];

bool checkchar(char chr);

float sqrf(float v);

char * str_addch(char * s0, char ch);

void check_directory(const char * path);

#endif /* SRC_TYPES_H_ */

