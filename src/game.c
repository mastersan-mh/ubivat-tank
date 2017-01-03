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

keyconfig_t keyconfig1[] =
{
		{ SDL_SCANCODE_UP    , "+move_north" },
		{ SDL_SCANCODE_DOWN  , "+move_south" },
		{ SDL_SCANCODE_LEFT  , "+move_west"  },
		{ SDL_SCANCODE_RIGHT , "+move_east"  },
		{ SDL_SCANCODE_SLASH , "+attack_artillery" },
		{ SDL_SCANCODE_PERIOD, "+attack_missile"   },
		{ SDL_SCANCODE_COMMA , "+attack_mine"      }
};

keyconfig_t keyconfig2[] =
{
		{ SDL_SCANCODE_R   , "+move_north" },
		{ SDL_SCANCODE_F   , "+move_south" },
		{ SDL_SCANCODE_D   , "+move_west"  },
		{ SDL_SCANCODE_G   , "+move_east"  },
		{ SDL_SCANCODE_W   , "+attack_artillery" },
		{ SDL_SCANCODE_Q   , "+attack_missile"   },
		{ SDL_SCANCODE_TAB , "+attack_mine"      }
};

uint32_t controls[ACTION_NUM] =
{
		[ACTION_ENTER_MAINMENU] = SDL_SCANCODE_ESCAPE,
		[ACTION_CHEAT_WIN     ] = SDL_SCANCODE_Z,
		[ACTION_SFACTOR] = SDL_SCANCODE_X,
		[ACTION_DFACTOR] = SDL_SCANCODE_C

};

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

/*
 * заставка между уровнями
 */
static void game_state_intermission_draw()
{
	static image_index_t list[] =
	{
			IMG_TANK0,
			IMG_TANK1,
			IMG_TANK2,
			IMG_TANK3,
			IMG_TANK4
	};
	gr2D_setimage0(0, 0, game.m_i_interlv);
	font_color_set3i(COLOR_15);
	video_printf(108,191,0,"НАЖМИ ПРОБЕЛ");

	int i;
	int num = server_client_num_get();

	int refy;
	if(num == 0) refy = 84;
	else refy = 76;

	font_color_set3i(COLOR_15);
	video_printf(48 + 8 * 00, refy      , orient_horiz, "ОЧКИ      ФРАГИ      ВСЕГО ФРАГОВ");

	for(i = 0; i < num; i++)
	{
		host_client_t * client = server_client_get(i);
		item_img_t * img = image_get(list[client->storedata.level]);
		gr2D_setimage1(26       , refy + 8 +     16 * i, img, 0, 0, c_p_MDL_box, c_p_MDL_box);
		font_color_set3i(COLOR_15);
		video_printf(48 + 8 *  0, refy + 8 + 4 + 16 * i, orient_horiz, "%d" , client->storedata.scores);
		video_printf(48 + 8 * 10, refy + 8 + 4 + 16 * i, orient_horiz, "%ld", client->storedata.frags);
		video_printf(48 + 8 * 21, refy + 8 + 4 + 16 * i, orient_horiz, "%ld", client->storedata.fragstotal);
	}
}

/*
 * информация об уровне
 */
static void game_state_missionbrief_draw()
{
	gr2D_setimage0(0,0,game.m_i_interlv);
	font_color_set3i(COLOR_15);
	video_printf(160-06*4 ,8*5, orient_horiz, "КАРТА:");
	font_color_set3i(COLOR_15);
	video_printf(160-16*4, 8*7 , orient_horiz, map.name);
	video_printf(160-07*4, 8*10, orient_horiz, "ЗАДАЧА:");
	video_printf(108     , 191 , orient_horiz, "НАЖМИ ПРОБЕЛ");
	video_printf_wide(160 - 8 * 8, 8 * 12, 8 * 16, map.brief);
}

static void game_gameTick();

static void game_draw();


void game_action_enter_mainmenu()
{
	game.show_menu = true;
}

void game_action_win()
{
	game._win_ = true;
}

void game_rebind_keys_all()
{
	input_key_unbind_all();
	size_t i;
	for(i = 0; i < ACTION_NUM; i++)
	{
		input_key_bindAction(controls[i], actions[i]);
	}

	for(i = 0; i < ARRAYSIZE(keyconfig1); i++)
	{
		input_key_bind_act(keyconfig1[i].key, keyconfig1[i].action);
	}

	for(i = 0; i < ARRAYSIZE(keyconfig2); i++)
	{
		input_key_bind_act(keyconfig2[i].key, keyconfig2[i].action);
	}

}

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

	game.msg       = NULL;
	game.state     = GAMESTATE_NOGAME;
	game.show_menu = true;

	randomize();
	printf("%s\n", c_strTITLE);
	printf("%s\n", c_strCORP);
	printf(GAME_LOGO);
	map_init();
	map_load_list();

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
	game.m_i_interlv  = image_get(IMG_MENU_I_INTERLV);
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
	game_cfg_load();

	input_init();

	game_rebind_keys_all();

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
	game_abort();
	//очистим список карт
	map_list_removeall();
	//очистим память от изображений
	images_done();
};

void client_event_pump(menu_selector_t * imenu)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		//printf("event.type = %d\n", event.type);
		switch(event.type)
		{
			case SDL_KEYDOWN:
				input_key_setState(event.key.keysym.scancode, true);
				if(game.state == GAMESTATE_INTERMISSION)
				{
					*imenu = MENU_GAME_SAVE;
				}
				break;
			case SDL_KEYUP:
				input_key_setState(event.key.keysym.scancode, false);
				break;
		}
		//player_checkcode();
	}
}

void client_events_send()
{

}


/*
 *
 */
void game_main()
{
	bool quit = false;
	menu_selector_t imenu = MENU_MAIN;
	menu_selector_t imenu_process = imenu;

	gamestate_t gamestate_prev = game.state;


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

	while(!quit)
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

		server();

		client();

		//printf("time0 = %ld dtime = %ld\n", time_current, dtime);

		if(game.show_menu)
		{
			menu_events_pump();
		}
		else
		{
			client_event_pump(&imenu);
		}

		client_events_send();

		if(quit)break;

		video_screen_draw_begin();

		if(game.show_menu)
		{
			imenu_process = imenu;
			imenu = menu_handle(imenu_process);
			switch(imenu_process)
			{
			case MENU_QUIT: quit = true;break;
			default: break;
			}
			menu_draw(imenu_process);
		}
		else
		{
			switch(game.state)
			{
			case GAMESTATE_NOGAME:
				gamestate_prev = game.state;
				break;
			case GAMESTATE_INGAME:
				if(gamestate_prev != game.state)
				{
					if(game.sound_playId)
					{
						sound_play_stop(game.sound_playId);
						game.sound_playId = 0;
					}
				}
				gamestate_prev = game.state;
				game_gameTick();
				game_draw();
				break;
			case GAMESTATE_MISSION_BRIEF:
				if(gamestate_prev != game.state)
				{
					sound_play_stop(game.sound_playId);
					if(!game.sound_playId)
						game.sound_playId = sound_play_start(SOUND_MUSIC1, -1);
				}
				gamestate_prev = game.state;
				game_state_missionbrief_draw();
				break;
			case GAMESTATE_INTERMISSION:
				if(gamestate_prev != game.state)
				{
					sound_play_stop(game.sound_playId);
					if(!game.sound_playId)
						game.sound_playId = sound_play_start(SOUND_MUSIC1, -1);
				}
				gamestate_prev = game.state;
				game_state_intermission_draw();
				imenu_process = MENU_GAME_SAVE;
				gamestate_prev = game.state;
				break;
			}
		}

		font_color_set3i(COLOR_15);
		video_printf(10,10, orient_horiz, "FPS = %d", fps);
		video_screen_draw_end();
	};
}

/*
 * главная процедура игры
 */
void game_gameTick()
{
	entities_handle();

	if( game._win_)
	{
		int i;
		bool alive = true;
		int num = server_client_num_get();
		// проверим что все игроки живы
		for(i = 0; i < num; i++)
		{
			if( ((player_t *)server_client_get(i)->entity->data)->items[ITEM_HEALTH] < 0 )
				alive = false;
		}
		if( alive )
		{
			if(game.flags & c_g_f_CASE)
			{
				//игра по выбору
				game_abort();
			}
			else
			{
				server_store_clients_info();
				//игра по уровням
				if(game_nextmap() == true)
					game.state = GAMESTATE_INTERMISSION;
			}
		}
	}
}


static void game_draw_cam(entity_t * player, camera_t * cam)
{

	player_t * pl = player->data;

	if(pl->bull)
	{
		cam->pos.x = pl->bull->pos.x;
		cam->pos.y = pl->bull->pos.y;
	}
	else
	{
		cam->pos.x = player->pos.x;
		cam->pos.y = player->pos.y;
	}

	video_viewport_set(
		cam->x,
		cam->y,
		cam->sx,
		cam->sy
	);

	map_draw(cam);

	video_viewport_set(
		0.0f,
		0.0f,
		VIDEO_SCREEN_W,
		VIDEO_SCREEN_H
	);
	player_draw_status(cam, player);
}

void game_draw()
{
	int i;
	int num = server_client_num_get();
	for(i = 0; i < num; i++)
	{
		camera_t * cam;
		if(i == 0) cam = &game.P0cam;
		else cam = &game.P1cam;

		entity_t * ent =server_client_get(i)->entity;
		if(ent)
			game_draw_cam(ent, cam);

	}

	video_viewport_set(
		0.0f,
		0.0f,
		VIDEO_SCREEN_W,
		VIDEO_SCREEN_H
	);

	if(game.msg)
	{
		font_color_set3i(COLOR_1);
		video_printf_wide(96, 84, 128, game.msg);
		game.msg = NULL;
	};

}

/*
 * процедура перехода на следующую карту
 */
/*
 * запись конфига
 */
int game_cfg_save()
{
	int ret = 0;
	size_t i;

	char * path = Z_malloc(strlen(game_dir_conf) + strlen(FILENAME_CONFIG) + 1);
	strcpy(path, game_dir_conf);
	strcat(path, FILENAME_CONFIG);
	FILE * f = fopen(path, "w");
	Z_free(path);
	if(!f)
	{
		ret = 1;
		goto __end;
	}

	for(i = 0; i < ARRAYSIZE(keyconfig1); i++)
	{
		if(fprintf(f, "bind %d %s\n", keyconfig1[i].key, keyconfig1[i].action) !=0 )
		{
			ret = 2;
			goto __end;
		}
	}

	for(i = 0; i < ARRAYSIZE(keyconfig2); i++)
	{
		if(fprintf(f, "bind2 %d %s\n", keyconfig2[i].key, keyconfig2[i].action) !=0 )
		{
			ret = 2;
			goto __end;
		}
	}

	__end:
	switch(ret)
	{
		case 0:
		case 2:
			fclose(f);
		case 1:;
	}
	return ret ? -1 : 0;
};
/*
 * новый конфиг
 */
int game_cfg_new()
{
	return game_cfg_save();
}
/********чтение конфига********/
int game_cfg_load()
{
	int ret = 0;
	FILE * f;
	ssize_t count;

	char * path = Z_malloc(strlen(game_dir_conf) + strlen(FILENAME_CONFIG) + 1);
	strcpy(path, game_dir_conf);
	strcat(path, FILENAME_CONFIG);
	f = fopen(path, "r");
	Z_free(path);
	if(!f)
		return game_cfg_new();

	char * line;
	size_t line_size = 0;
	ssize_t value;

	static const char delims[] = " \t";

	char * ptrptr;

	do
	{
		line = NULL;
		value = getline(&line, &line_size, f);

		char * tok = strtok_r(line, delims, &ptrptr);

		keyconfig_t * keyconfig;

		if(strcmp(tok, "bind"))
		{
			keyconfig = keyconfig1;
		}
		else if(strcmp(tok, "bind2"))
		{
			keyconfig = keyconfig2;
		}

		tok = strtok_r(NULL, delims, &ptrptr);
		int key = atoi(tok);
		tok = strtok_r(NULL, delims, &ptrptr);
		char * action = tok;
#error here

/*
		if(count != sizeof(game.controls))
		{
			ret = 2;
			goto end;
		}
*/

		free(line);
	} while (value > 0);

	end:
	switch(ret)
	{
	case 0:
	case 2:
		fclose(fd);
	case 1:;
	}
	return ret ? -1 : 0;
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
	int num = server_client_num_get();
	for(i = 0; i < num; i++)
	{
		game_record_save_player(fd, server_client_get(i)->entity);
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

	if(!rec->exist) return 1;
	if(!(rec->flags & c_g_f_CASE))
	{
		game.gamemap = mapList;
		while(game.gamemap)
		{
			if(!strcmp(rec->mapfilename, game.gamemap->map))break;
			game.gamemap = game.gamemap->next;
		}
		if(!game.gamemap) return 2;
	};
	//закроем открытую карту
	map_clear();

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
	game.flags = rec->flags;

	//прочитаем карту
	ret = map_load(rec->mapfilename);
	if(ret) return 3;

	// создаем игру
	ret = game_create();
	// спавним всех игроков

	int player_num;
	if(rec->flags & c_g_f_2PLAYERS) player_num = 2;
	else player_num = 1;

	int i;
	for(i = 0 ; i < player_num; i++)
	{
		int id = client_connect();
		server_spawn_client(id);
	}

	for(i = 0; i < player_num; i++)
	{
		game_record_load_player(fd, server_client_get(i)->entity);
	}
	close(fd);
	return 0;
}
/*
 * создане игры
 *
 * @return = 0 - игра создана успешно
 * @return = 1 - игра уже создана
 * @return = 2 - ошибка создания серверного игрока
 * @return = 3 - ошибка создания серверного игрока
 */
int game_create()
{
	game.gamemap = mapList;

	float cam_sx = (float)VIDEO_SCREEN_W * (float)VIDEO_SCALEX / (float)VIDEO_SCALE;
	float cam_sy = (float)VIDEO_SCREEN_H * (float)VIDEO_SCALEY / (float)VIDEO_SCALE;

	float statusbar_h = 32.0f * (float)VIDEO_SCALEY / (float)VIDEO_SCALE;
	if((game.flags & c_g_f_2PLAYERS) == 0)
	{
		game.P0cam.pos.x = 0;
		game.P0cam.pos.y = 0;
		game.P0cam.x     = 0;
		game.P0cam.y     = 0;
		game.P0cam.sx    = cam_sx;
		game.P0cam.sy    = cam_sy - statusbar_h;//184
	}
	else
	{
		game.P0cam.pos.x = 0;
		game.P0cam.pos.y = 0;
		game.P0cam.x     = cam_sx/2 + 1;
		game.P0cam.y     = 0;
		game.P0cam.sx    = cam_sx/2 - 1;
		game.P0cam.sy    = cam_sy - statusbar_h;
		game.P1cam.pos.x = 0;
		game.P1cam.pos.y = 0;
		game.P1cam.x     = 0;
		game.P1cam.y     = 0;
		game.P1cam.sx    = cam_sx/2 - 1;
		game.P1cam.sy    = cam_sy - statusbar_h;
	};
	game.show_menu = false;
	game.state  = GAMESTATE_MISSION_BRIEF;
	game.paused = false;

	server_start();

	return 0;
}
/*
 * прерывание игры
 */
void game_abort(void)
{
	//дисконнект всех игроков

	server_disconnect_clients();

	server_stop();

	//закроем карту
	map_clear();
	game.state = GAMESTATE_NOGAME;
	game._win_ = false;
}

/*
 * сообщения об ошибках
 */
bool game_nextmap()
{
	int ret;
	server_unspawn_client(0);
	server_unspawn_client(1);

	//закроем карту
	map_clear();
	game.state = GAMESTATE_MISSION_BRIEF;
	game._win_      = false;
	//menu_interlevel();

	game.gamemap = game.gamemap->next;
	if(!game.gamemap)
	{
		// конец игры, последняя карта
		game_abort();
		return false;
	}
	ret = map_load(game.gamemap->map);
	if(ret)
	{
		game_msg_error(ret);
		game_abort();
		return false;
	}

	server_spawn_client(0);
	server_spawn_client(1);

	server_restore_clients_info();

	return true;
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
		video_printf(x+(sx / 2)-6*8, y+2, orient_horiz, "ERROR: ");
		font_color_set3i(COLOR_15);
		int e;
		if(error <= 5)e = error;
		else e = error - 5;
		if(e > ERR_MAX) e = ERR_MAX;

		video_printf(x+2, y+16, orient_horiz, errList[e]);

		do{ } while(true);
	}
}

void game_message_send(const char * mess)
{
	game.msg = (char*)mess;
};


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
