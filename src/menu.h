/*
 * menu.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_MENU_H_
#define SRC_MENU_H_

#include "g_gamesave.h"
#include "types.h"

#include <SDL2/SDL.h>

//меню
typedef enum
{
	MENU_MAIN,
	MENU_GAME,
	MENU_GAME_NEW1P,
	MENU_GAME_NEW2P,
	MENU_GAME_LOAD,
	MENU_GAME_SAVE,
	MENU_CUSTOM,
	MENU_CUSTOM_NEWP1,
	MENU_CUSTOM_NEWP2,
	MENU_CUSTOM_CONNECT,
	MENU_OPTIONS,
	MENU_ABOUT,
	MENU_ABORT,
	MENU_QUIT,
	MENU_NUM
} menu_selector_t;


/* MENU_MAIN */
typedef struct
{
	int menu;
} menu_main_ctx_t;


/* MENU_GAME */
typedef struct
{
	int menu;
} menu_game_ctx_t;

/* MENU_GAME_NEW1P */

/* MENU_GAME_NEW2P */

/* MENU_GAME_LOAD */
typedef struct
{
	int menu;
} menu_game_load_ctx_t;

/* MENU_GAME_SAVE */
typedef struct
{
	enum
	{
		MENU_GAME_SAVE_SELECT,
		MENU_GAME_SAVE_INPUT,
		MENU_GAME_SAVE_SAVE,
	} state;
	int menu;
	// бэкап редактируемой записи
	gamesave_descr_t rec;
} menu_game_save_ctx_t;

/* MENU_CUSTOM */
typedef struct
{
	int menu;
} menu_custom_ctx_t;

/* MENU_CUSTOM_CONNECT */
typedef struct
{
	int menu;
} menu_custom_connect_ctx_t;

/* MENU_CUSTOM_NEWP1 */

/* MENU_CUSTOM_NEWP2 */

/* MENU_CUSTOM_CONNECT */

/* MENU_OPTIONS */
typedef struct
{
	enum{
		MENU_OPTIONS_SELECT,
		MENU_OPTIONS_WAIT_KEY
	} state;
	int menu;
	int column;
} menu_options_ctx_t;
/* MENU_ABOUT */
typedef struct
{
	int menu;
} menu_about_ctx_t;
/* MENU_INTERLEVEL */

/* MENU_PRELEVEL */

/* MENU_ABORT */

/* MENU_QUIT */

typedef SDL_Scancode buffer_key_t;

typedef enum
{
	MENU_ACTION_NOTHING,
	MENU_ACTION_FIRSTENTRY, /* Первый вход в меню */
	MENU_ACTION_UP,
	MENU_ACTION_DOWN,
	MENU_ACTION_LEFT,
	MENU_ACTION_RIGHT,
	MENU_ACTION_ENTER, /* Переход вглубь меню */
	MENU_ACTION_LEAVE,
	MENU_ACTION_SPACE
} menu_action_t;

typedef struct
{
	void * context;
	int  (* handle) (buffer_key_t scancode, menu_action_t action, void * context);
	void (* draw) (const void * context);
} menu_t;

extern menu_t menus[MENU_NUM];

extern void menu_events_pump();
extern int menu_handle(int imenu);
extern void menu_draw(int imenu);


#endif /* SRC_MENU_H_ */
