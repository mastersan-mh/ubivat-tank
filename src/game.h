/*
 * game.h
 *
 *  Created on: 8 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_GAME_H_
#define SRC_GAME_H_

#include <types.h>
#include <map.h>
#include <player.h>
#include <actions.h>

#define GAME_LOGO \
		"          _____    ______              __     ______ \n"\
		"||    || |  __ \\  |__  __| ||    ||   /  \\   |__  __|\n"\
		"||    || | |  \\ |    ||    ||    ||  / /\\ \\     ||   \n"\
		"||    || | |__//     ||    ||    || | |__| |    ||   \n"\
		"||    || | |  \\\\     ||    ||    || | ____ |    ||   \n"\
		"\\\\____|| | |__/ |  __||__   \\\\__//  ||    ||    ||   \n"\
		" \\_____| |_____/  |______|   \\__/   ||    ||    ||   \n"\
		"\n"\
		"                       T A N K\n"\
		"\n"

#define GAME_SAVESNUM (8)


struct image_table_ent_s
{
	char * filename;
	char * id;
};

typedef uint32_t control_t[__ACTION_NUM];

//запись иры
typedef struct
{
	//имя файла записи
	char _file[9];
	//имя файла карты
	char Hmap[9];
	//внутреннее имя записи
	TstrZ16 Hname;
	//флаги настройки игры
	char flags;
} gamesave_t;

//состояние игры
typedef struct
{
	//камера игрока 0
	camera_t P0cam;
	//камера игрока 1
	camera_t P1cam;
	//PTR игрока 0
	player_t * P0;
	//PTR игрока 1
	player_t * P1;
	char * msg;

	//флаги состояния игры
	int flags;

	//игра создана т.е. карта загружена
	bool created;
	//находимся ли в процедуре игры
	bool ingame;

	//игрок победил
	bool _win_;

	maplist_t * gamemap;
	maplist_t * custommap;

	item_img_t * i_health;
	item_img_t * i_armor;
	item_img_t * i_star;
	item_img_t * w_w0;
	item_img_t * w_w1;
	item_img_t * w_brick;
	item_img_t * w_water[3];
	/***********************/
	item_img_t * m_i_logo;
	item_img_t * m_i_conback;
	item_img_t * m_i_interlv;
	item_img_t * m_i_game;
	item_img_t * m_i_g_new_p1;
	item_img_t * m_i_g_new_p2;
	item_img_t * m_i_g_load;
	item_img_t * m_i_g_save;
	item_img_t * m_i_case;
	item_img_t * m_i_options;
	item_img_t * m_i_about;
	item_img_t * m_i_abort;
	item_img_t * m_i_quit;
	item_img_t * m_i_cur_0;
	item_img_t * m_i_cur_1;
	item_img_t * m_i_arrowL;
	item_img_t * m_i_arrowR;
	item_img_t * m_i_lineL;
	item_img_t * m_i_lineM;
	item_img_t * m_i_lineR;
	item_img_t * m_i_flagRUS;
	item_img_t * m_i_flagUSA;
	control_t controls;
	//список записей
	gamesave_t saveslist[GAME_SAVESNUM];
	/*******************************************************************/
} game_t;


extern game_t game;
extern long dtime;
extern double ddtime10;

void game_action_enter_mainmenu();

void game_init();
void game_done();
void game_main();

int game_mainproc();

void game_nextmap();


int game_cfg_save();
int game_cfg_new();
int game_cfg_load();

void game_rebind_keys_all();

int game_pal_get();
void game_record_getsaves();
bool game_record_save(gamesave_t * rec);
int game_record_load(gamesave_t * rec);
int game_create();
void game_abort();
void game_msg_error  (int error);

void game_message_send(const char * mess);

void game_halt(const char *error, ...)
	__attribute__ ((noreturn));
void game_console_send(const char *error, ...);

#endif /* SRC_GAME_H_ */
