/*
 *        Ubivat Tank
 *        константы, типы, переменные
 * by Master San
 */

#ifndef SRC_DEFS_H_
#define SRC_DEFS_H_

#define DEBUG

#define MAX_MESSAGE_SIZE 2048


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

#define c_CFGname "config.cfg"
#define FILENAME_MAPSLIST "maps.lst"
#define c_PALname "palette.pal"
#define c_SAVext  ".sav"
//коды
#define code_levelup "IAMSUPERTANK"
#define code_health  "GIVEHEALTH"
#define code_armor   "GIVEARMOR"
#define code_ammo    "GIVEAMMO"
#define code_all     "GIVEALL"
//разрешенные для ввода символы
#define chars_allowed "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

//меню
#define c_m_main        0x00
#define c_m_game        0x10
#define c_m_game_new1P  0x11
#define c_m_game_new2P  0x12
#define c_m_game_load   0x13
#define c_m_game_save   0x14
#define c_m_case        0x20
#define c_m_case_case   0x21
#define c_m_case_newP1  0x22
#define c_m_case_newP2  0x23
#define c_m_options     0x30
#define c_m_about       0x40
#define c_m_abort       0x50
#define c_m_quit        0x60
// заголовок формата BII
#define c_BIIheader     "bii"
// карта
#define c_MAPext        ".mut"
// Map Ubivat Tank
#define c_MAPheader     "MUT"

// размер карты OX
#define c_MAP_sx 66
// размер карты OY
#define c_MAP_sy 50
#define c_MAP_s_player   "SPAWN.PLAYER"
#define c_MAP_s_enemy    "SPAWN.ENEMY"
#define c_MAP_s_boss     "SPAWN.BOSS"
#define c_MAP_i_health   "ITEM.HEALTH"
#define c_MAP_i_armor    "ITEM.ARMOR"
#define c_MAP_i_star     "ITEM.STAR"
#define c_MAP_i_rocket   "ITEM.ROCKET"
#define c_MAP_i_mine     "ITEM.MINE"
#define c_MAP_o_exit     "OBJ.EXIT"
#define c_MAP_o_mess     "OBJ.MESS"

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
//SINGLE/COOP спавн
#define c_s_PLAYER      1
//ENEMY       спавн
#define c_s_ENEMY       2
//BOSS        спавн
#define c_s_BOSS        3
//ID предметов
#define c_i_health      0
#define c_i_armor       1
#define c_i_star        2
#define c_i_rocket      3
#define c_i_mine        4
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
//конец уровня
#define c_o_exit        0
//мессагер
#define c_o_mess        1
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
#ifdef DEBUG
#define BASEDIR         "base/"
#else
#define BASEDIR         "base/"
#endif

//директория записей
#define SAVESDIR       "saves/"
//директория карт
#define MAPSDIR        "maps/"

//рисунок
typedef struct
{
	//рисунок
	void * pic;
	//размер_рисунка_X
	int sx;
	//размер_рисунка_Y
	int sy;
} TBIIpic;

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


/*
 * изображение
 */
typedef struct image_s
{
	struct image_s *next;
	/* идентификатор рисунка */
	TstrZ11 IMGname;
	//изображение
	TBIIpic IMG;
} item_img_t;


//координаты
typedef struct
{
	float x,y;
}pos_t;

#include <ctrl.h>


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




/********************************************************************/


bool checkchar(char chr);

float sqrf(float v);

void randomize();
int xrand(int hi);

int eof(int fd);


extern char *c_strTITLE;
extern char *c_strCORP;
extern char *c_about[];
#endif /* SRC_DEFS_H_ */

