/*
 * menu.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_MENU_H_
#define SRC_MENU_H_

#include <types.h>

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
} menu_main_context_t;


/* MENU_GAME */
typedef struct
{
	int menu;
} menu_game_context_t;

/* MENU_GAME_NEW1P */

/* MENU_GAME_NEW2P */

/* MENU_GAME_LOAD */
typedef struct
{
	int menu;
} menu_game_load_context_t;

/* MENU_GAME_SAVE */
typedef struct
{
	int menu_cur;
	bool f_input;
} menu_game_save_context_t;

/* MENU_CUSTOM */
typedef struct
{
	int menu;
} menu_custom_context_t;

/* MENU_CUSTOM_NEWP1 */

/* MENU_CUSTOM_NEWP2 */

/* MENU_OPTIONS */
typedef struct
{
	int menu;
	int cur_pl;
	bool wait_a_key;
} menu_options_context_t;
/* MENU_ABOUT */
typedef struct
{
	int menu;
} menu_about_context_t;
/* MENU_INTERLEVEL */
typedef struct
{
	int menu;
} menu_interlevel_context_t;
/* MENU_PRELEVEL */
typedef struct
{
	int menu;
} menu_prelevel_context_t;

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
	int maxmenu;
	void * context;
	int  (* handle) (menu_key_t status, void * context);
	void (* draw) (const void * context);
} menu_t;

extern menu_t menus[MENU_NUM];

extern void menu_send_event(SDL_Event * event);
extern int menu_handle(int imenu);
extern void menu_draw(int imenu);


#endif /* SRC_MENU_H_ */
