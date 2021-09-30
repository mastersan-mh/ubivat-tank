/**
 *        Ubivat Tank
 *        игра
 * by Master San
 */

#include "defs.h"
#include "eng_actions.h"
#include "eng_conf.h"
#include <system.h>
#include <game.h>
#include <input.h>
#include <actions.h>
#include <menu.h>
#include <map.h>
#include <weap.h>
#include <img.h>
#include <video.h>
#include "sound.h"
#include <_gr2D.h>
#include <fonts.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <player.h>
#include <stdarg.h>
#include <types.h>

#define CHECK(x, y) do{ if( x!= y) return false; }while(0)

static const struct keybind keybinds_permanent[] =
{
        { SDL_SCANCODE_ESCAPE, "menu_main" },
        { SDL_SCANCODE_X     , "sfactor"  },
        { SDL_SCANCODE_C     , "dfactor"  },
        { SDL_SCANCODE_Z     , "win"       },
        { SDL_SCANCODE_U     , "cam_x_inc" },
        { SDL_SCANCODE_J     , "cam_x_dec" },
        { SDL_SCANCODE_I     , "cam_y_inc" },
        { SDL_SCANCODE_K     , "cam_y_dec" },
        { SDL_SCANCODE_O     , "cam_z_inc" },
        { SDL_SCANCODE_L     , "cam_z_dec" },
        { SDL_SCANCODE_Y     , "obj_ang_inc" },
        { SDL_SCANCODE_H     , "obj_ang_dec" },
};

static const struct keybind keybinds_default[] =
{
        { SDL_SCANCODE_UP    , ACTION_PLAYER_MOVE_UP   },
        { SDL_SCANCODE_DOWN  , ACTION_PLAYER_MOVE_DOWN },
        { SDL_SCANCODE_LEFT  , ACTION_PLAYER_MOVE_LEFT },
        { SDL_SCANCODE_RIGHT , ACTION_PLAYER_MOVE_RIGHT},
        { SDL_SCANCODE_SLASH , ACTION_PLAYER_ATTACK_WEAPON1},
        { SDL_SCANCODE_PERIOD, ACTION_PLAYER_ATTACK_WEAPON2},
        { SDL_SCANCODE_COMMA , ACTION_PLAYER_ATTACK_WEAPON3},
        { SDL_SCANCODE_R     , ACTION_PLAYER2_MOVE_UP  },
        { SDL_SCANCODE_F     , ACTION_PLAYER2_MOVE_DOWN},
        { SDL_SCANCODE_D     , ACTION_PLAYER2_MOVE_LEFT},
        { SDL_SCANCODE_G     , ACTION_PLAYER2_MOVE_RIGHT},
        { SDL_SCANCODE_W     , ACTION_PLAYER2_ATTACK_WEAPON1},
        { SDL_SCANCODE_Q     , ACTION_PLAYER2_ATTACK_WEAPON2},
        { SDL_SCANCODE_TAB   , ACTION_PLAYER2_ATTACK_WEAPON3},
};

//состояние игры
game_t game;

struct game_internal
{
    bool quit;
    menu_selector_t imenu;
    menu_selector_t imenu_process;
};

static struct game_internal gamei = {};

int game_video_dfactor = 7;
int game_video_sfactor = 6;

char * game_dir_home;
char * game_dir_conf;
char * game_dir_saves;



static Uint32 P_tick_cb(Uint32 interval, void *param);
static void P_tick(void);

static int game_gameTick(void);

static void game_draw(void);


void game_action_enter_mainmenu(void)
{
	game.ingame = false;
}

void game_action_win(void)
{
    if(!game._win_)
    {
        game._win_ = true;
        game.win_timer = 3 * 100;
    }
}

void game_rebind_keys_all(void)
{
    size_t i;
    for(i = 0; i < ARRAYSIZE(keybinds_default); i++)
    {
        const struct keybind * keybind = &keybinds_default[i];
        eng_conf_key_bind(keybind->key, keybind->action_name);
    }
}

static void P_conf_init(void)
{
    int res;

    game_info("Config init...");
    eng_actions_init();
    eng_conf_init();

    eng_actions_register(actions, ACTION____NUM);

    eng_conf_key_unbindall();
    size_t i;

    for(i = 0; i < ARRAYSIZE(keybinds_permanent); i++)
    {
        const struct keybind * keybind = &keybinds_permanent[i];
        eng_conf_key_bind(keybind->key, keybind->action_name);
    }

    res = eng_conf_load();
    if(res)
    {
        game_rebind_keys_all();
        eng_conf_save();
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


	game.P0         = NULL;
	game.P1         = NULL;
	game.msg        = NULL;
	game.created    = false;
	game.ingame     = false;

	randomize();
    game_info("%s", c_strTITLE);
    game_info("%s", c_strCORP);
    game_info(GAME_LOGO);
	map_init();
	map_load_list();

    game_info("ENTERING GRAPHIC...");
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
	game.m_i_conback  = image_get(M_CONBACK  );
	game.m_i_interlv  = image_get(M_I_INTERLV);
	game.m_i_game     = image_get(M_GAME     );
	game.m_i_g_new_p1 = image_get(M_G_NEW_P1 );
	game.m_i_g_new_p2 = image_get(M_G_NEW_P2 );
	game.m_i_g_load   = image_get(M_G_LOAD   );
	game.m_i_g_save   = image_get(M_G_SAVE   );
	game.m_i_case     = image_get(M_CASE     );
	game.m_i_options  = image_get(M_OPTIONS  );
	game.m_i_about    = image_get(M_ABOUT    );
	game.m_i_abort    = image_get(M_ABORT    );
	game.m_i_quit     = image_get(M_QUIT     );
	game.m_i_cur_0    = image_get(M_CUR_0    );
	game.m_i_cur_1    = image_get(M_CUR_1    );
	game.m_i_arrowL   = image_get(M_ARROWL   );
	game.m_i_arrowR   = image_get(M_ARROWR   );
	game.m_i_lineL    = image_get(M_LINEL    );
	game.m_i_lineM    = image_get(M_LINEM    );
	game.m_i_lineR    = image_get(M_LINER    );
	game.m_i_flagRUS  = image_get(F_RUS      );
	game.m_i_flagUSA  = image_get(F_USA      );
	game.w_water[0]   = image_get(IMG_WATER0     );
	game.w_water[1]   = image_get(IMG_WATER1     );
	game.w_water[2]   = image_get(IMG_WATER2     );
	game.i_health     = image_get(I_HEALTH   );
	game.i_armor      = image_get(I_ARMOR    );
	game.i_star       = image_get(I_STAR     );

	P_conf_init();

	//инициализация оружий
    game_info("Weapons init...");
	strcpy(wtable[0].name, "Pulki");                                     //название оружия
	wtable[0].damage     = 15;                                          //повреждение
	wtable[0].selfdamage = 7;//0;                                       //повреждение
	wtable[0].ammo       = c_WEAP_indefinit;                            //макс кол-во боеприпасов
	wtable[0].radius     = 7;                                           //радиус действия
	wtable[0].range      = -1;                                          //дальность
    wtable[0].bullspeed  = 7.5 / GAME_TICK_PERIOD;
	wtable[0].bullbox    = 2;                                           //bodybox
	wtable[0].icon       = image_get(W_BULL);     //изображение оружия
	strcpy(wtable[1].name, "Rocket");                                    //название оружия
	wtable[1].damage     = 100;                                         //повреждение
	wtable[1].selfdamage = 50;                                          //повреждение
	wtable[1].ammo       = 50;                                          //макс кол-во боеприпасов
	wtable[1].radius     = 11;                                          //радиус действия
	wtable[1].range      = -1;                                          //дальность
    wtable[1].bullspeed  = 8.0 / GAME_TICK_PERIOD;
	wtable[1].bullbox    = 8;                                           //bodybox
	wtable[1].icon       = image_get(W_ROCKET);     //изображение оружия
	strcpy(wtable[2].name, "Mine");                                      //название оружия
	wtable[2].damage     = 200;                                         //повреждение
	wtable[2].selfdamage = 100;                                         //повреждение
	wtable[2].ammo       = 50;                                          //макс кол-во боеприпасов
	wtable[2].radius     = 11;                                          //радиус действия
	wtable[2].range      = 100;                                         //дальность
    wtable[2].bullspeed  = -8.0 / GAME_TICK_PERIOD;
	wtable[2].bullbox    = 8;                                           //bodybox
	wtable[2].icon       = image_get(W_MINE);     //изображение оружия

	uint32_t interval_ms = GAME_TICK_PERIOD;
	SDL_AddTimer(interval_ms, P_tick_cb, NULL);

}

/*
 * подготовка к завершению игры
 */
void game_done()
{
	Z_free(game_dir_home);
	Z_free(game_dir_conf);
	Z_free(game_dir_saves);
	eng_conf_done();
    eng_actions_done();
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

static Uint32 P_tick_cb(Uint32 interval, void *param)
{
    P_tick();
    return interval;
}

static void P_tick(void)
{
    system_time_update();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if(game.ingame)
        {
            switch(event.type)
            {
            case SDL_KEYDOWN:
                input_key_setState(event.key.keysym.scancode, true);
                break;
            case SDL_KEYUP:
                input_key_setState(event.key.keysym.scancode, false);
                break;
            }

            //player_checkcode();

        }
        else
        {
            menu_send_event(&event);
        }

        switch(event.type)
        {
        case SDL_QUIT:gamei.quit = true;break;
        }

        // handle your event here
    }

    if(!game.ingame)
    {
        gamei.imenu_process = gamei.imenu;
        gamei.imenu = menu_handle(gamei.imenu_process);
        switch(gamei.imenu_process)
        {
            case MENU_QUIT: gamei.quit = true;break;
            default: break;
        }
    }
    else
    {
        int ret = game_gameTick();
        if(ret >= 0) gamei.imenu = ret;
    }
}


/*
 *
 */
void game_main()
{
	gamei.imenu = MENU_MAIN;
	gamei.imenu_process = gamei.imenu;


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


    system_time_update();
	while(!gamei.quit)
	{
		video_screen_draw_begin();


		gr2D_setimage3(0, 0, game.i_health);

		if(!game.ingame)
		{
			menu_draw(gamei.imenu_process);
		}
		else
		{
			game_draw();
		}

		video_screen_draw_end();

	};
}

/*
 * главная процедура игры
 */
int game_gameTick()
{
	players_control();
	bull_control();
	explode_control();

    bool win = false;
    if(game._win_)
    {
        if(game.win_timer == 0)
        {
            win = true;
        }
        else
        {
            game.win_timer--;
        }
    }

	if(
			win &&                                           //победа
			(game.P0->charact.health > 0) &&                        //оба игрока должны быть живы
			(!game.P1 || (game.P1 && game.P1->charact.health > 0))  //
	)
	{
		if(game.flags & c_g_f_CASE)
		{
			//игра по выбору
			game_abort();
			return MENU_MAIN;
		}
		else
		{
			//игра по уровням
			if(game_nextmap() == true) return MENU_INTERLEVEL;
		}
	}
	return MENU_MAIN;
}

static void P_draw_win_timer(void)
{
    if(!game._win_)
    {
        return;
    }

    static char str[32];

    sprintf(str, "%lu", (game.win_timer / 100) + 1);
    game_message_send(str);

}

static void game_draw_cam(player_t * player, camera_t * cam, bool playframe)
{

	if(player->bull)
	{
		cam->pos.x = player->bull->pos.x;
		cam->pos.y = player->bull->pos.y;
	}
	else
	{
		cam->pos.x = player->move.pos.x;
		cam->pos.y = player->move.pos.y;
	}

	video_viewport_set(
		cam->x,
		cam->y,
		cam->sx,
		cam->sy
	);

	map_draw(cam);

	player_draw_all(cam);
	explode_draw_all(cam);
	bull_draw_all(cam);
	P_draw_win_timer();

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

	game_draw_cam(game.P0, &game.P0cam, true);

	if(game.P1)
	{
		game_draw_cam(game.P1, &game.P1cam, false);
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
bool game_nextmap(void)
{
	int ret;
	//дисконнект всех монстров
	explode_removeall();
	bull_removeall();
	player_disconnect_monsters();
	//закроем карту
	map_clear();
	game.created    = false;
	game.ingame     = false;
	game._win_      = false;
	//menu_interlevel();

	game.P0->charact.spawned = false;
	//sound_play_stop(game.P0->soundId_move);

	if(game.P1)
	{
		game.P1->charact.spawned = false;
		//sound_play_stop(game.P1->soundId_move);
	}

	game.gamemap = game.gamemap->next;
	if(!game.gamemap)
	{
		// конец игры, последняя карта
		game.gamemap = mapList;
		//дисконнект всех монстров
		player_disconnect_all();
		return false;
	}
	ret = map_load(game.gamemap->map);
	if(ret)
	{
		game_msg_error(ret);
		game_abort();
		return false;
	}
	ret = game_create();
	return true;
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
static bool game_record_save_player(int fd, player_t * player)
{
	switch(player->charact.status)
	{
	case c_p_P0:
	case c_p_P1:
    {
        const char * map_class_name = map_class_names[MAPDATA_MOBJ_SPAWN_PLAYER];
        size_t size = strlen(map_class_name) + 1;
        CHECK(write(fd, map_class_name, size), size);
        break;
    }
	default:
		return false;
	};
    game_savedata_player_t savedata =
    {
            .scores     = player->charact.scores,
            .fragstotal = player->charact.fragstotal,
            .frags      = player->charact.frags,
            .health     = player->charact.health,
            .armor      = player->charact.armor,
            .ammo1      = player->w.ammo[0],
            .ammo2      = player->w.ammo[1],
            .ammo3      = player->w.ammo[2],
            .status     = player->charact.status
    };
    CHECK(write(fd, &savedata, sizeof(savedata)), sizeof(savedata));
	return true;
};

/**
 * чтение игрока
 * @return true | false
 */
static bool game_record_load_player(int fd, player_t * player)
{
    mapdata_mobj_type_t mapdata_mobj_type = map_file_class_get(fd);
    if(mapdata_mobj_type != MAPDATA_MOBJ_SPAWN_PLAYER)
    {
        return false;
    }
    game_savedata_player_t savedata;
    ssize_t c = read(fd, &savedata, sizeof(savedata));
    if(c != sizeof(savedata))return false;
    player->charact.scores     = savedata.scores;
    player->charact.fragstotal = savedata.fragstotal;
    player->charact.frags      = savedata.frags;
    player->charact.health     = savedata.health;
    player->charact.armor      = savedata.armor;
    player->w.ammo[0]          = savedata.ammo1;
    player->w.ammo[1]          = savedata.ammo2;
    player->w.ammo[2]          = savedata.ammo3;
    player->charact.status     = savedata.status;
    player_class_init(player);
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
    fd = open(path,
            O_CREAT | O_WRONLY | O_TRUNC,
            S_IRUSR | S_IWUSR
    );
    Z_free(path);
    if(fd <= 0)
    {
        return false;
    }

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

	game_record_save_player(fd, game.P0);
    if(game.P1)
    {
        game_record_save_player(fd, game.P1);
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

	//создаем игру и спавним всех игроков
	ret = game_create();
	//читаем первого игрока
	game_record_load_player(fd, game.P0);
	//читаем второго игрока
	if(rec->flags & c_g_f_2PLAYERS)
		game_record_load_player(fd, game.P1);
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
    int res;

	int cam_sx = VIDEO_SCREEN_W;
	int cam_sy = VIDEO_SCREEN_H;

	int statusbar_h = 32;
	if(game.created) return 1;
	if((game.flags & c_g_f_2PLAYERS) == 0)
	{
        game.P0cam.pos.x = 0;
        game.P0cam.pos.y = 0;
        game.P0cam.x     = 0;
        game.P0cam.y     = 0;
        game.P0cam.sx    = cam_sx;
        game.P0cam.sy    = cam_sy - statusbar_h;//184
        res = player_connect(c_p_P0);
        if(
                !(res == 0 || res == 2)
        )
        {
            return res;
        }
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

        res = player_connect(c_p_P0);
        if(
                !(res == 0 || res == 2)
        )
        {
            return res;
        }

        res = player_connect(c_p_P1);
        if(
                !(res == 0 || res == 2)
        )
        {
            return res;
        }
	};
	//спавним всех игроков и монстров
	player_spawn_all();
	game.created    = true;
	game.ingame     = false;
	return 0;
}
/*
 * прерывание игры
 */
void game_abort(void)
{
	game.gamemap = mapList;
	//дисконнект всех игроков
	player_disconnect_all();
	explode_removeall();
	bull_removeall();
	//закроем карту
	map_clear();
	game.created    = false;
	game.ingame     = false;
	game._win_      = false;
}

/*
 * сообщения об ошибках
 */
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

void game_error(const char * format, ...)
{
    static char errmsg[MAX_MESSAGE_SIZE];
    va_list argptr;
    va_start(argptr, format);
#ifdef HAVE_VSNPRINTF
    vsnprintf(errmsg, MAX_MESSAGE_SIZE, format, argptr);
#else
    vsprintf(errmsg, format, argptr);
#endif
    va_end(argptr);

    fprintf(stdout, "Error: %s\n", errmsg);
}

void game_warn(const char * format, ...)
{
    static char errmsg[MAX_MESSAGE_SIZE];
    va_list argptr;
    va_start(argptr, format);
#ifdef HAVE_VSNPRINTF
    vsnprintf(errmsg, MAX_MESSAGE_SIZE, format, argptr);
#else
    vsprintf(errmsg, format, argptr);
#endif
    va_end(argptr);

    fprintf(stdout, "Warning: %s\n", errmsg);
}

void game_info(const char * format, ...)
{
    static char errmsg[MAX_MESSAGE_SIZE];
    va_list argptr;
    va_start(argptr, format);
#ifdef HAVE_VSNPRINTF
    vsnprintf(errmsg, MAX_MESSAGE_SIZE, format, argptr);
#else
    vsprintf(errmsg, format, argptr);
#endif
    va_end(argptr);

    fprintf(stdout, "%s\n", errmsg);
}

void game_console_send(const char *error, ...)
{

}

