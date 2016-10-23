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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#include <Z_mem.h>

#ifndef bool
typedef enum {
	false = 0,
	true = 1
}bool;
#endif

typedef void (*actionf_t)();

#define FILENAME_CONFIG "config.cfg"
#define FILENAME_MAPSLIST "maps.lst"
#define FILENAME_PALETTE "palette.pal"
#define c_SAVext  ".sav"
//коды
#define code_levelup "IAMSUPERTANK"
#define code_health  "GIVEHEALTH"
#define code_armor   "GIVEARMOR"
#define code_ammo    "GIVEAMMO"
#define code_all     "GIVEALL"
//разрешенные для ввода символы
#define chars_allowed "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

// карта
#define c_MAPext        ".mut"
// Map Ubivat Tank
#define c_MAPheader     "MUT"

//броня0
#define c_m_w_w0        0x01
//броня1
#define c_m_w_w1        0x02
//кирпич
#define c_m_w_brick     0x04
//вода
#define c_m_water       0x08
//флаг присутствия стены
#define c_m_f_clip      0x10
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
//ускорение игрока
#define c_p_accel       12
//скорость проигрывания кадров:бег
#define c_p_fpsRUN      20
//макс. кол-во игроков
#define c_p_Pmax        128
//body box
#define c_p_MDL_box     16
//позиция изображения
#define c_p_MDL_pos     (-8)
//оружие не используется
#define c_p_WEAP_notused (-1)
//время на перезарядку(мсек)
#define c_p_WEAP_reloadtime (100)
//направлениия
//0 - вверх
#define c_DIR_up        0
//1 - вниз
#define c_DIR_dn        1
//2 - влево
#define c_DIR_lf        2
//3 - вправо
#define c_DIR_rt        3
//время "думания" бота
#define c_BOT_time      150
//расстояние с которого бот будет стремиться вести атаку
#define c_BOT_dist      (c_p_MDL_box*3)

//директория файлов игры
#ifdef _DEBUG
#define BASEDIR         "base/"
#else
#define BASEDIR         "base/"
#endif

//директория записей
#define SAVESDIR       "saves/"
//директория карт
#define MAPSDIR        "maps/"

typedef char Tstring11[11];
typedef char Tstring16[16];

typedef char TstrZ8[9];
typedef char TstrZ11[12];
typedef char TstrZ16[17];
typedef char TstrZ64[65];
typedef char TstrZ128[129];

//идентификатор изображения
#define TBIIid TstrZ11
#define Tstringclass TstrZ16

/*
 * время
 */
typedef struct time_s
{
	//начальное время
	int t0;
	//конечное время
	int t1;
	//изменение времени
	int delta;
	//последнее изменение времени
	int last_delta;
} Ttime;




//координаты
typedef struct
{
	float x,y;
}pos_t;

#include <think.h>


//камера
typedef struct
{
	//координаты
	pos_t orig;
	//позиция на экране
	int x;
	int y;
	//ширина и высота
	int sx;
	int sy;
}camera_t;



extern char *c_strTITLE;
extern char *c_strCORP;
extern char *c_about[];


bool checkchar(char chr);

float sqrf(float v);

void randomize();
int xrand(int hi);

char * str_addch(char * s0, char ch);

#endif /* SRC_TYPES_H_ */

