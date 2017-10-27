/**
 *        Ubivat Tank
 *        игра
 * by Master San
 */

#include "types.h"
#include "g_events.h"
#include "system.h"
#include "game.h"
#include "client.h"
#include "cl_input.h"
#include "cl_game.h"
#include "g_conf.h"
#include "actions.h"
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

void game_init(void)
{
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

    server_init();
    cl_game_init();

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
    input_init();
    gconf_newbind();
    gconf_load();
    gconf_rebind_all();

    model_resources_register();

#include "entities_init.h"

}

/*
 * подготовка к завершению игры
 */
void game_done(void)
{
    Z_free(game_dir_home);
    Z_free(game_dir_conf);
    Z_free(game_dir_saves);
    input_done();
    sound_precache_free();
    sound_done();
    video_done();
    //прекратим игру
    cl_done();
    server_done();
    //очистим список карт
    map_list_removeall();
    //очистим память от изображений
    images_done();
};


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

    while(!cl_game_quit_get())
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

        server_handle();
        client_handle();

        font_color_set3i(COLOR_15);
        video_printf(10,10, "FPS = %d", fps);
        video_screen_draw_end();

    }
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
