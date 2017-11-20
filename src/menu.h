/*
 * menu.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_MENU_H_
#define SRC_MENU_H_

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
    MENU_CUSTOM_SERVERSLIST,
    MENU_OPTIONS,
    MENU_ABOUT,
    MENU_ABORT,
    MENU_QUIT,
    MENU_NUM
} menu_selector_t;



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
extern void menu_event_key_down(bool key_repeat, int key);
extern int menu_handle(int imenu);
extern void menu_draw(int imenu);


#endif /* SRC_MENU_H_ */
