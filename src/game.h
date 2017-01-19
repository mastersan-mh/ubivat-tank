/*
 * game.h
 *
 *  Created on: 8 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_GAME_H_
#define SRC_GAME_H_

#include "types.h"
#include "img.h"

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

typedef struct
{
	char name[16];
	char mapfilename[16];
	uint16_t flags;

} ATTR_PACKED game_savedata_header_t;

typedef struct
{
	uint32_t fragstotal;
	uint32_t frags;
	uint32_t scores;
	int16_t health;
	int16_t armor;
	int16_t ammo1;
	int16_t ammo2;
	int16_t ammo3;
} ATTR_PACKED game_savedata_player_t;

//запись иры
typedef struct
{
	bool exist;
	//внутреннее имя записи
	char name[16];
	//имя файла карты
	char mapfilename[16];
	//флаги настройки игры
	uint16_t flags;
} gamesave_descr_t;

typedef enum gamestate_s
{
	GAMESTATE_NOGAME, /* игра не создана */
	GAMESTATE_MISSION_BRIEF,
	GAMESTATE_GAMESAVE,
	GAMESTATE_INGAME,
	GAMESTATE_INTERMISSION,
} gamestate_t;

//состояние игры
typedef struct
{

	item_img_t * i_health;
	item_img_t * i_armor;
	item_img_t * i_star;
	item_img_t * w_water[3];
	/***********************/
	item_img_t * m_i_logo;
	item_img_t * m_i_conback;
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
	//список записей
	gamesave_descr_t saveslist[GAME_SAVESNUM];
	/*******************************************************************/


} game_t;


extern game_t game;
extern unsigned long time_current;
extern long dtime;
extern double dtimed;
extern double dtimed1000;

/* (ms) */
extern long menu_dtime;
/* (ms) */
extern double menu_dtimed;
/*  (s) */
extern double menu_dtimed1000;

extern char * game_dir_conf;
extern char * game_dir_saves;

extern int game_video_dfactor;
extern int game_video_sfactor;

void game_action_win();

void game_init();
void game_done();
void game_main();

int game_pal_get();
void game_record_getsaves();
bool game_record_save(int isave);
int game_record_load(int isave);
int cl_game_create(int flags);
void game_msg_error  (int error);


void game_halt(const char *error, ...)
	__attribute__ ((noreturn));
void game_console_send(const char *error, ...);

#endif /* SRC_GAME_H_ */
