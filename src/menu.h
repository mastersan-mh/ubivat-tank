/*
 * menu.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_MENU_H_
#define SRC_MENU_H_

#include <types.h>
#include <game.h>

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
	MENU_OPTIONS,
	MENU_ABOUT,
	MENU_INTERLEVEL,
	MENU_PRELEVEL,
	MENU_ABORT,
	MENU_QUIT,
	MENU_NUM
}menu_selector_t;


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
	enum
	{
		MENU_GAME_LOAD_INIT,
		MENU_GAME_LOAD_SELECT
	} state;
	int menu;
} menu_game_load_ctx_t;

/* MENU_GAME_SAVE */
typedef struct
{
	enum
	{
		MENU_GAME_SAVE_INIT,
		MENU_GAME_SAVE_SELECT,
		MENU_GAME_SAVE_INPUT,
		MENU_GAME_SAVE_SAVE,
	}state;
	int menu;
	// бэкап редактируемой записи
	gamesave_descr_t rec;
} menu_game_save_ctx_t;

/* MENU_CUSTOM */
typedef struct
{
	int menu;
} menu_custom_ctx_t;

/* MENU_CUSTOM_NEWP1 */

/* MENU_CUSTOM_NEWP2 */

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
typedef struct
{
	int menu;
} menu_interlevel_ctx_t;
/* MENU_PRELEVEL */
typedef struct
{
	int menu;
} menu_prelevel_ctx_t;

/* MENU_ABORT */

/* MENU_QUIT */


/*
 * считывание статуса меню
 */
typedef enum
{
	NOTHING,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	ENTER,
	LEAVE,
	SPACE
} menu_key_t;

/*
	switch(menukey)
	{
	case NOTHING: break;
	case UP     : if(__ctx->menu > 0  ) __ctx->menu--;break;
	case DOWN   : if(__ctx->menu < 8-1) __ctx->menu++;break;
	case LEFT   :
	case RIGHT  :
	case ENTER  :
	case LEAVE  :
	case SPACE  :
	}
*/

typedef struct
{
	void * context;
	int  (* handle) (void * context);
	void (* draw) (const void * context);
} menu_t;

extern menu_t menus[MENU_NUM];

extern void menu_send_event(SDL_Event * event);
extern int menu_handle(int imenu);
extern void menu_draw(int imenu);


#endif /* SRC_MENU_H_ */
