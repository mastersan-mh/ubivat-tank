/*
 *        Ubivat Tank
 *        константы, типы, переменные
 * by Master San
 */

#ifndef SRC_TYPES_H_
#define SRC_TYPES_H_

#define _DEBUG

#define MAX_MESSAGE_SIZE 2048

#define ATTR_PACKED __attribute__ ((packed))

#include "config.h"
#include <fonts.h>
#include <Z_mem.h>

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
#define FILENAME_GAMESAVE_EXT "sav"
//коды
#define code_levelup "IAMSUPERTANK"
#define code_health  "GIVEHEALTH"
#define code_armor   "GIVEARMOR"
#define code_ammo    "GIVEAMMO"
#define code_all     "GIVEALL"
//разрешенные для ввода символы
#define chars_allowed "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"


//флаги состояния игры:
//2 игрока
#define c_g_f_2PLAYERS  0x01
//игра по выбору
#define c_g_f_CASE      0x02
//флаги спавнпоинта


//предметы
//размер предмета
#define c_i_MDL_box     16
//позиция изображения
#define c_i_MDL_pos     (-8)
//объекты
//размер объекта
#define c_o_MDL_box     16
//позиция изображения
#define c_o_MDL_pos     (-8)
//оружия
//оружие бесконечно
#define c_WEAP_indefinit 0
//очки
//сколько очков нужно набрать для повышения уровня танка
#define c_score_perclass 200
//очки за 1 убитый танк
#define c_score_pertank 5
//макс. кол-во очков
#define c_score_max     2000000000l
//пуля
//скорость проигрывания кадров взрыва
#define c_bull_FPS      8
//взрыв
//скорость проигрывания кадров взрыва
#define c_explode_FPS   28
//количество кадров взрыва
#define c_explode_Famnt 8
//игрок
//игрок 0
#define c_p_P0          0
//игрок 1
#define c_p_P1          1
//вражеский танк
#define c_p_ENEMY       2
//босс
#define c_p_BOSS        3
//макс. кол-во игроков
#define c_p_Pmax        128
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

typedef float coord_t;
typedef void (*actionf_t)();

//координаты
typedef struct
{
	coord_t x;
	coord_t y;
}pos_t;

#include <think.h>


//камера
typedef struct
{
	//координаты
	pos_t pos;
	//позиция на экране
	int x;
	int y;
	//ширина и высота
	int sx;
	int sy;
}camera_t;

#define ARRAYSIZE( x ) (sizeof((x))/sizeof(*(x)));

extern char * c_strTITLE;
extern char * c_strCORP;
extern coloredtext_t c_about[];

extern bool debug_noAI;

bool checkchar(char chr);

float sqrf(float v);

void randomize();
int xrand(int hi);

char * str_addch(char * s0, char ch);

void check_directory(const char * path);

#endif /* SRC_TYPES_H_ */

