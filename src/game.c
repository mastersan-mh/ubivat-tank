/**
 *        Ubivat Tank
 *        игра
 * by Master San
 */

#include "types.h"
#include "g_events.h"
#include "system.h"
#include "game.h"
#include "server.h"
#include "client.h"
#include "cl_input.h"
#include "cl_game.h"
#include "client_events.h"
#include "g_conf.h"
#include "menu.h"
#include "map.h"
#include "img.h"
#include "video.h"
#include "sound.h"
#include "_gr2D.h"
#include "fonts.h"
#include "model_resources.h"
#include "entity.h"
#include "actions.h"

// entities
#include "entities_includes.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

game_t game = {};

int game_video_dfactor = 7;
int game_video_sfactor = 6;

/* текущее время, мс */
unsigned long time_current;
/* (ms) */
long dtime;
/* (ms) */
double dtimed;
/*  (s) */
double dtimed1000;

/* (ms) */
long menu_dtime;
/* (ms) */
double menu_dtimed;
/*  (s) */
double menu_dtimed1000;


char * game_dir_home;
char * game_dir_conf;
char * game_dir_saves;

bool game_quit_get(void)
{
    return !server_running() && !client_running() && game.quit;
}

void game_quit_set(void)
{
    server_stop();
    client_stop();
    game.quit = true;
}

void game_init(void)
{
    game.quit = false;
    game.show_menu = true;
    game.imenu = MENU_MAIN;
    game.imenu_process = game.imenu;


    char * home_dir = getenv("HOME");

    game_dir_home = Z_malloc(strlen(home_dir) + strlen(GAME_DIR_HOME) + 1);
    strcpy(game_dir_home, home_dir);
    strcat(game_dir_home, GAME_DIR_HOME);

    game_dir_conf = Z_malloc(strlen(home_dir) + strlen(GAME_DIR_CONF) + 1);
    strcpy(game_dir_conf, home_dir);
    strcat(game_dir_conf, GAME_DIR_CONF);

    game_dir_saves = Z_malloc(strlen(home_dir) + strlen(GAME_DIR_SAVES) + 1);
    strcpy(game_dir_saves, home_dir);
    strcat(game_dir_saves, GAME_DIR_SAVES);

    check_directory(game_dir_home);
    check_directory(game_dir_conf);
    check_directory(game_dir_saves);


    randomize();
    printf("%s\n", c_strTITLE);
    printf("%s\n", c_strCORP);
    printf(GAME_LOGO);
    map_init();

    map_load_list();
    game.custommap = mapList;

    server_init();
    client_init();

    printf("ENTERING GRAPHIC...\n");
    if(video_init())
        game_halt("Video init failed");


    snd_format_t requested =
    {
            //.freq = 11025,
            .freq = 22050,
            //.freq = 44100,
            //.width = 2,
            .width = 2,
            .channels = 1
    };

    sound_init(&requested);

    sound_precache();


    if(game_pal_get())
        game_halt("Error load palette %s.", FILENAME_PALETTE);
    //чтение изображений
    images_init();

    //чтение конфига
    printf("Config init...\n");
    client_input_init();
    gconf_newbinds();
    gconf_load();

    model_resources_register();

#include "entities_init.h"

    game_menu_show(MENU_MAIN);


    CIRCLEQ_INIT(&game.servers);
    game.servers_num = 0;

}

void game_server_add(const net_addr_t * sender, int clients_num)
{
    struct game_server_s * server;
    GAME_SERVERS_FOREACH(server)
    {
        if(memcmp(&server->net_addr, sender, sizeof(net_addr_t)) == 0)
        {
            server->clients_num = clients_num;
            return;
        }
    }
    if(game.servers_num >= GAME_SERVERS_NUM)
        return;
    server = Z_malloc(sizeof(struct game_server_s));
    server->net_addr = *sender;
    server->clients_num = clients_num;
    CIRCLEQ_INSERT_TAIL(&game.servers, server, list);
    game.servers_num++;
}

void game_servers_freeall(void)
{
    struct game_server_s * server;
    while(!CIRCLEQ_EMPTY(&game.servers))
    {
        server = CIRCLEQ_FIRST(&game.servers);
        CIRCLEQ_REMOVE(&game.servers, server, list);
        Z_free(server);
    }
}

/*
 * подготовка к завершению игры
 */
void game_done(void)
{
    game_servers_freeall();

    Z_free(game_dir_home);
    Z_free(game_dir_conf);
    Z_free(game_dir_saves);
    client_input_done();
    sound_precache_free();
    sound_done();
    video_done();
    //прекратим игру
    client_done();
    server_done();
    //очистим список карт
    map_list_removeall();
    //очистим память от изображений
    images_done();
};

static void game_events_pump(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        //printf("event.type = %d\n", event.type);
        switch(event.type)
        {
            case SDL_KEYDOWN:
                if(game.show_menu)
                    menu_event_key_down(event.key.repeat, event.key.keysym.scancode);
                else
                    client_event_local_key_input(event.key.repeat, event.key.keysym.scancode, true);
                break;
            case SDL_KEYUP:
                if(!game.show_menu)
                    client_event_local_key_input(event.key.repeat, event.key.keysym.scancode, false);
                break;
            case SDL_QUIT:
                break;
            default:
                break;
        }
    }
}

/*
 *
 */
void game_main(void)
{

    //sound_play_start(SOUND_FIRE1);
    //sound_play_start(SOUND_MUSIC1, -1);
    //SDL_Delay(3000);
    //sound_play_start(SOUND_START);
    //sound_play_start(SOUND_MUSIC2);
    /*
	// выстрелов/мин: 650
	for(int i = 0; i< 100; i++)
	{
		sound_play_start(SOUND_FIRE1, 1);
		SDL_Delay(92);
	}
     */

    unsigned long time_prev;
    time_current = system_getTime_realTime_ms();
    unsigned long time_second = 0; // отсчёт секунды
    int frames = 0;
    int fps = 0;

    long int cycletimeeventer = 0;

    client_start();

    while(!game_quit_get())
    {
        time_prev = time_current;
        time_current = system_getTime_realTime_ms();
        dtime = time_current - time_prev;
        dtimed = (double)dtime;
        dtimed1000 = dtimed/1000.0f;

        menu_dtime = dtime;
        menu_dtimed = dtimed;
        menu_dtimed1000 = dtimed1000;

        time_second += dtime;
        frames++;
        if(time_second >= 1000)
        {
            time_second = 0;
            fps = frames;
            frames = 0;
        }

        cycletimeeventer += dtime;
        if(cycletimeeventer > 20)
        {
            cycletimeeventer = 0;
        }

        video_screen_draw_begin();

        game_events_pump();
        game_handle();
        game_draw();


        font_color_set3i(COLOR_15);
        video_printf(10,10, "FPS = %d", fps);
        video_screen_draw_end();

    }
}

/**
 * @brief создание игры
 *
 * @return = 0 - игра создана успешно
 * @return = 1 - игра уже создана
 * @return = 2 - ошибка создания серверного игрока
 * @return = 3 - ошибка создания серверного игрока
 */
int game_create(int flags)
{
    server_start(flags);
    client_flags_set(flags);
    game_menu_hide();
    return 0;
}

void game_abort(void)
{
    server_stop();
}

void game_handle(void)
{
    server_handle();
    client_handle();

    if(game.show_menu)
    {
        game.imenu_process = game.imenu;
        game.imenu = menu_handle(game.imenu_process);
    }
}

void game_draw(void)
{
    if(game.show_menu)
    {
        menu_draw(game.imenu_process);
    }
    else
    {
        client_game_draw();
    }
}

void game_action_showmenu(const char * action)
{
    game.show_menu = true;
}

void game_menu_show(menu_selector_t imenu)
{
    game.show_menu = true;
    game.imenu     = imenu;
}

void game_menu_hide(void)
{
    game.show_menu = false;
}

/*
 * чтение файла палитры
 * @return = 0 -успешно
 * @return = 1 -файл не найден
 * @return = 2 -ошибка чтения файла
 */
int game_pal_get(void)
{
    return img_palette_read(BASEDIR FILENAME_PALETTE);
}





void game_msg_error(int error)
{
#define sx (256)
#define sy (32)
#define x (160 - (sx / 2))
#define y (100 - (sy / 2))
#define ERR_MAX 10
    static char *errList[ERR_MAX] = {
            "NO ERRORS.", // 0
            "Map not found.", // 1
            "Map load error.", // 2
            "Map load error.", // 3
            "Could not find spawn point.", // 4
            "Could not find spawn point.", // 5
            "Record not found.", // 11
            "Map have no found in maps list.", // 12
            "Map load error.", // 13
            "Unknown."
    };
    bool quit = false;

    while(!quit)
    {
        font_color_set3i(COLOR_4);
        video_printf(x+(sx / 2)-6*8, y+2, "ERROR: ");
        font_color_set3i(COLOR_15);
        int e;
        if(error <= 5)e = error;
        else e = error - 5;
        if(e > ERR_MAX) e = ERR_MAX;

        video_printf(x+2, y+16, errList[e]);

        do{ } while(true);
    }
}

void game_console_send(const char *error, ...)
{
    static char errmsg[MAX_MESSAGE_SIZE];
    va_list argptr;
    va_start(argptr, error);
#ifdef HAVE_VSNPRINTF
    vsnprintf(errmsg, MAX_MESSAGE_SIZE, error, argptr);
#else
    vsprintf(errmsg, error, argptr);
#endif
    va_end(argptr);

    fprintf(stdout, "%s\n", errmsg);

}


void game_halt(const char * error, ...)
{
    int __errno_ = errno;
    static char errmsg[MAX_MESSAGE_SIZE];
    va_list argptr;
    va_start(argptr, error);
#ifdef HAVE_VSNPRINTF
    vsnprintf(errmsg, MAX_MESSAGE_SIZE, error, argptr);
#else
    vsprintf(errmsg, error, argptr);
#endif
    va_end(argptr);

    if(__errno_)
    {
        fprintf(stdout, "%s: %s\n", errmsg, strerror(__errno_));
    }
    else
    {
        fprintf(stdout, "%s\n", errmsg);
    }
    exit(1);
}
