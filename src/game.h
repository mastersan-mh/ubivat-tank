/*
 * game.h
 *
 *  Created on: 8 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_GAME_H_
#define SRC_GAME_H_

#include <sys/queue.h>
#include "types.h"
#include "net.h"
#include "img.h"
#include "menu.h"
#include "map.h"
#include "actions.h"

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

/* флаги состояния игры */
/* 2 игрока */
#define GAMEFLAG_2PLAYERS   0x01
/* игра по выбору */
#define GAMEFLAG_CUSTOMGAME 0x02

#define GAME_SERVERS_FOREACH(server) \
    CIRCLEQ_FOREACH(server, &game.servers, list)

struct game_server_s
{
    CIRCLEQ_ENTRY(game_server_s) list;
    net_addr_t net_addr;
    int clients_num;
};

typedef struct
{
    bool quit;
    bool show_menu;
    bool handle;
    bool remotegame;
    menu_selector_t imenu_process;
    menu_selector_t imenu;
    maplist_t * custommap;

#define GAME_SERVERS_NUM 64
    size_t servers_num;
    CIRCLEQ_HEAD(game_servers_head_s, game_server_s) servers;

} game_t;

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

extern bool game_quit_get(void);
extern void game_quit_set(void);

extern void game_init(void);
extern void game_done(void);
extern void game_main(void);

extern int game_create(int flags);
extern void game_connect(const net_addr_t * net_addr, int players_num);
extern void game_stop(void);
extern void game_abort(void);
extern void game_tick(void);

extern void game_draw(void);

extern void game_action_showmenu(const char * action);
extern void game_menu_show(menu_selector_t imenu);
extern void game_menu_hide(void);

extern int game_pal_get(void);
extern void game_record_getsaves(void);
extern bool game_record_save(int isave);
extern int game_record_load(int isave);
extern void game_msg_error(int error);

extern void game_server_add(const net_addr_t * sender, int clients_num);
extern void game_servers_freeall(void);


void game_halt(const char *error, ...)
__attribute__ ((noreturn));
void game_console_send(const char *error, ...);

#endif /* SRC_GAME_H_ */
