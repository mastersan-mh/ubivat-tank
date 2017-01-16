/**
 *        Ubivat Tank
 *        игра
 * by Master San
 */

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
#include "ent_spawn.h"
#include "ent_bull.h"
#include "ent_explode.h"
#include "ent_message.h"
#include "ent_exit.h"
#include "ent_items.h"
#include "ent_player.h"
#include "ent_weap.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <types.h>

//состояние игры
game_t game;

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

void game_init()
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
	game.m_i_logo     = image_get(IMG_MENU_LOGO     );
	game.m_i_conback  = image_get(IMG_MENU_CONBACK  );
	game.m_i_game     = image_get(IMG_MENU_GAME     );
	game.m_i_g_new_p1 = image_get(IMG_MENU_G_NEW_P1 );
	game.m_i_g_new_p2 = image_get(IMG_MENU_G_NEW_P2 );
	game.m_i_g_load   = image_get(IMG_MENU_G_LOAD   );
	game.m_i_g_save   = image_get(IMG_MENU_G_SAVE   );
	game.m_i_case     = image_get(IMG_MENU_CASE     );
	game.m_i_options  = image_get(IMG_MENU_OPTIONS  );
	game.m_i_about    = image_get(IMG_MENU_ABOUT    );
	game.m_i_abort    = image_get(IMG_MENU_ABORT    );
	game.m_i_quit     = image_get(IMG_MENU_QUIT     );
	game.m_i_cur_0    = image_get(IMG_MENU_CUR_0    );
	game.m_i_cur_1    = image_get(IMG_MENU_CUR_1    );
	game.m_i_arrowL   = image_get(IMG_MENU_ARROWL   );
	game.m_i_arrowR   = image_get(IMG_MENU_ARROWR   );
	game.m_i_lineL    = image_get(IMG_MENU_LINEL    );
	game.m_i_lineM    = image_get(IMG_MENU_LINEM    );
	game.m_i_lineR    = image_get(IMG_MENU_LINER    );
	game.m_i_flagRUS  = image_get(IMG_FLAG_RUS      );
	game.m_i_flagUSA  = image_get(IMG_FLAG_USA      );
	game.w_water[0]   = image_get(IMG_WATER0     );
	game.w_water[1]   = image_get(IMG_WATER1     );
	game.w_water[2]   = image_get(IMG_WATER2     );
	game.i_health     = image_get(IMG_ITEM_HEALTH);
	game.i_armor      = image_get(IMG_ITEM_ARMOR );
	game.i_star       = image_get(IMG_ITEM_STAR  );

	//чтение конфига
	printf("Config init...\n");
	input_init();
	gconf_load();
	gconf_rebind_all();

	model_resources_register();

	entity_bull_init();
	entity_explode_init();
	entity_spawn_init();
	entity_player_init();
	entity_message_init();
	entity_exit_init();
	entity_items_init();

}

/*
 * подготовка к завершению игры
 */
void game_done()
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
void game_main()
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

		server();
		client();

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
int game_pal_get()
{
	return img_palette_read(BASEDIR FILENAME_PALETTE);
}
/**
 * @description чтение заголовка записи
 * @return true | false
 */
static bool game_record_load_info(const char * savename, gamesave_descr_t * rec)
{
	int fd;
	char * path = Z_malloc(strlen(game_dir_saves)+strlen(savename) + 1);
	strcpy(path, game_dir_saves);
	strcat(path, savename);
	fd = open(path, O_RDONLY);
	Z_free(path);
	if(fd < 0) return false;
	memset(rec, 0, sizeof(*rec));
	game_savedata_header_t header;
	ssize_t count = read(fd, &header, sizeof(header));
	if(count != sizeof(header))
	{
		close(fd);
		return false;
	}
	strncpy(rec->name, header.name, 15);
	strncpy(rec->mapfilename, header.mapfilename, 15);
	rec->flags = header.flags;
	close(fd);
	rec->exist = true;
	return true;
};

static char * _make_gamesave_filename(char * filename, int i)
{
	sprintf(filename, "/ut_s%02d." FILENAME_GAMESAVE_EXT, i);
	return filename;
}

/*
 * формируем листинг записей
 */
void game_record_getsaves()
{
	int i;
	char filename[16];

	memset(game.saveslist, 0, sizeof(game.saveslist[0])*GAME_SAVESNUM);
	for(i = 0; i < GAME_SAVESNUM; i++)
	{
		game_record_load_info(_make_gamesave_filename(filename, i), &(game.saveslist[i]));
	};
};

/**
 * запись игрока
 * @return = true | false
 */
static bool game_record_save_player(int fd, entity_t * player)
{
	write(fd, map_class_names[MAPDATA_MOBJ_SPAWN_PLAYER], strlen(map_class_names[MAPDATA_MOBJ_SPAWN_PLAYER])+1);
	player_t * pl = player->data;
	game_savedata_player_t savedata =
	{
		.fragstotal = pl->charact.fragstotal,
		.frags      = pl->charact.frags,
		.scores     = pl->items[ITEM_SCORES],
		.health     = pl->items[ITEM_HEALTH],
		.armor      = pl->items[ITEM_ARMOR],
		.ammo1      = pl->items[ITEM_AMMO_ARTILLERY],
		.ammo2      = pl->items[ITEM_AMMO_MISSILE],
		.ammo3      = pl->items[ITEM_AMMO_MINE]
	};
	write(fd, &savedata, sizeof(savedata));
	return true;
};

/**
 * чтение игрока
 * @return true | false
 */
static bool game_record_load_player(int fd, entity_t * player)
{
	player_t * pl = player->data;
	mapdata_entity_type_t mapdata_mobj_type = map_file_class_get(fd);
	if(mapdata_mobj_type != MAPDATA_MOBJ_SPAWN_PLAYER) return false;
	game_savedata_player_t savedata;
	ssize_t c = read(fd, &savedata, sizeof(savedata));
	if(c != sizeof(savedata))return false;
	pl->charact.fragstotal = savedata.fragstotal;
	pl->charact.frags      = savedata.frags;
	pl->items[ITEM_SCORES] = savedata.scores;
	pl->items[ITEM_HEALTH] = savedata.health;
	pl->items[ITEM_ARMOR]  = savedata.armor;
	pl->items[ITEM_AMMO_ARTILLERY] = savedata.ammo1;
	pl->items[ITEM_AMMO_MISSILE]   = savedata.ammo2;
	pl->items[ITEM_AMMO_MINE]      = savedata.ammo3;
	player_class_init(player, player->data);
	return true;
};
/**
 * сохраниние записи
 * @return true| false
 */
bool game_record_save(int isave)
{
	gamesave_descr_t * rec = &game.saveslist[isave];
	int fd;
	char filename[16];
	_make_gamesave_filename(filename, isave);

	check_directory(game_dir_saves);

	char * path;
	path = Z_malloc(strlen(game_dir_saves) + strlen(filename) + 1);
	strcpy(path, game_dir_saves);
	strcat(path, filename);
	strcpy(rec->mapfilename, map._file);
	ssize_t count;
	//int ret = unlink(path);
	fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	Z_free(path);
	if(fd <= 0)return false;

	game_savedata_header_t header;
	strncpy(header.name, rec->name, 15);
	strncpy(header.mapfilename, rec->mapfilename, 15);
	header.flags = rec->flags;
	count = write(fd, &header, sizeof(header));
	if(count != sizeof(header))
	{
		close(fd);
		return false;
	}

	int i;
	int num = host_client_num_get();
	for(i = 0; i < num; i++)
	{
		game_record_save_player(fd, host_client_get(i)->entity);
	}

	close(fd);
	return true;
};
/*
 * чтение сохранённой игры
 * @return = 0 - успешно
 * @return = 1 - запись отсутствует
 * @return = 2 - карта отсутствует в списке карт
 * @return = 3 - ошибка чтения карты
 */
int game_record_load(int isave)
{
	gamesave_descr_t * rec = &game.saveslist[isave];
	int ret;
	int fd;

	//закроем открытую карту
	map_clear();

	if(!rec->exist) return 1;
	/*
	if(!(rec->flags & GAMEFLAG_CUSTOMGAME))
	{
		game.gamemap = map_find(rec->mapfilename);
		if(!game.gamemap) return 2;
	};
*/
	char filename[16];
	_make_gamesave_filename(filename, isave);
	char * path = Z_malloc(strlen(game_dir_saves) + strlen(filename) + 1);
	strcpy(path, game_dir_saves);
	strcat(path, filename);
	fd = open(path, O_RDONLY);
	Z_free(path);
	if(fd <= 0)
	{
		game_halt("gamesave load error");
		return -1;
	}
	game_savedata_header_t header;
	ssize_t count = read(fd, &header, sizeof(header));
	if(count != sizeof(header))
	{
		close(fd);
		return false;
	}
	strncpy(rec->name, header.name, 15);
	strncpy(rec->mapfilename, header.mapfilename, 15);
	rec->flags = header.flags;

	//прочитаем карту
	ret = map_load(rec->mapfilename);
	if(ret) return 3;

	// создаем игру
	ret = cl_game_create(rec->flags);
	// спавним всех игроков

	int player_num;
	if(rec->flags & GAMEFLAG_2PLAYERS) player_num = 2;
	else player_num = 1;

	int i;
	for(i = 0 ; i < player_num; i++)
	{
		int id = client_connect();
		host_client_spawn_id(id);
	}

	clients_initcams();

	for(i = 0; i < player_num; i++)
	{
		game_record_load_player(fd, host_client_get(i)->entity);
	}
	close(fd);
	return 0;
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
