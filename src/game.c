/**
 *        Ubivat Tank
 *        игра
 * by Master San
 */


#include <system.h>
#include <game.h>
#include <input.h>
#include <actions.h>
#include <menu.h>
#include <map.h>
#include <weap.h>
#include <img.h>
#include <video.h>
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

//состояние игры
game_t game;

/* (ms) */
long dtime;
/* (ms) */
double dtimed;
/*  (s) */
double dtimed1000;

struct image_table_ent_s images_info[] = {
	{BASEDIR"menu/mhlogo.bii" ,"M_LOGO"     },
	{BASEDIR"menu/conback.bii","M_CONBACK"  },
	{BASEDIR"menu/interlv.bii","M_I_INTERLV"},
	{BASEDIR"menu/game.bii"   ,"M_GAME"     },
	{BASEDIR"menu/new_p1.bii" ,"M_G_NEW_P1" },
	{BASEDIR"menu/new_p2.bii" ,"M_G_NEW_P2" },
	{BASEDIR"menu/load.bii"   ,"M_G_LOAD"   },
	{BASEDIR"menu/save.bii"   ,"M_G_SAVE"   },
	{BASEDIR"menu/case.bii"   ,"M_CASE"     },
	{BASEDIR"menu/options.bii","M_OPTIONS"  },
	{BASEDIR"menu/about.bii"  ,"M_ABOUT"    },
	{BASEDIR"menu/abort.bii"  ,"M_ABORT"    },
	{BASEDIR"menu/quit.bii"   ,"M_QUIT"     },
	{BASEDIR"menu/cur_0.bii"  ,"M_CUR_0"    },
	{BASEDIR"menu/cur_1.bii"  ,"M_CUR_1"    },
	{BASEDIR"menu/arrowl.bii" ,"M_ARROWL"   },
	{BASEDIR"menu/arrowr.bii" ,"M_ARROWR"   },
	{BASEDIR"menu/linel.bii"  ,"M_LINEL"    },
	{BASEDIR"menu/linem.bii"  ,"M_LINEM"    },
	{BASEDIR"menu/liner.bii"  ,"M_LINER"    },
	{BASEDIR"pics/tank0.bii"   ,"TANK0"      },
	{BASEDIR"pics/tank1.bii"   ,"TANK1"      },
	{BASEDIR"pics/tank2.bii"   ,"TANK2"      },
	{BASEDIR"pics/tank3.bii"   ,"TANK3"      },
	{BASEDIR"pics/tank4.bii"   ,"TANK4"      },
	{BASEDIR"pics/f_rus.bii"   ,"F_RUS"      },
	{BASEDIR"pics/f_usa.bii"   ,"F_USA"      },
	{BASEDIR"pics/f_white.bii" ,"F_WHITE"    },
	{BASEDIR"pics/w_w0.bii"    ,"W_W0"       },
	{BASEDIR"pics/w_w1.bii"    ,"W_W1"       },
	{BASEDIR"pics/w_brick.bii" ,"W_BRICK"    },
	{BASEDIR"pics/water0.bii"  ,"WATER0"     },
	{BASEDIR"pics/water1.bii"  ,"WATER1"     },
	{BASEDIR"pics/water2.bii"  ,"WATER2"     },
	{BASEDIR"pics/i_health.bii","I_HEALTH"   },
	{BASEDIR"pics/i_armor.bii" ,"I_ARMOR"    },
	{BASEDIR"pics/i_star.bii"  ,"I_STAR"     },
	{BASEDIR"pics/o_exit.bii"  ,"O_EXIT"     },
	{BASEDIR"pics/w_bull.bii"  ,"W_BULL"     },
	{BASEDIR"pics/w_rocket.bii","W_ROCKET"   },
	{BASEDIR"pics/w_mine.bii"  ,"W_MINE"     },
	{BASEDIR"pics/b_bull.bii"  ,"B_BULL"     },
	{BASEDIR"pics/b_rocket.bii","B_ROCKET"   },
	{BASEDIR"pics/b_mine.bii"  ,"B_MINE"     },
	{BASEDIR"pics/e_small.bii" ,"E_SMALL"    },
	{BASEDIR"pics/e_big.bii"   ,"E_BIG"      },
	{NULL, NULL}
};

static void game_draw();


void game_action_enter_mainmenu()
{
	game.ingame = false;
}

void game_action_win()
{
	game._win_ = true;
}

/*
 * открытие файлов с рисунками меню
 */
static void pics_load()
{
	int i;
	struct image_table_ent_s * info;
	int ret;
	for(i = 0, info = &images_info[i]; info->filename != NULL; i++, info = &images_info[i])
	{
		ret = IMG_add(info->filename, info->id);
		if(ret)
		{
			game_halt("Image \"%s\" load error %s", info->filename, IMG_errorGet());
		}
	}
};



void game_rebind_keys_all()
{
	input_key_unbind_all();
	int i;
	for(i = 0; i < __ACTION_NUM; i++)
	{

		input_key_bindAction(game.controls[i], actions[i]);
	}
}

void game_init()
{
	game.P0         = NULL;
	game.P1         = NULL;
	game.msg        = NULL;
	game.created    = false;
	game.ingame     = false;

	randomize();
	printf("%s\n", c_strTITLE);
	printf("%s\n", c_strCORP);
	printf(GAME_LOGO);
	map_init();
	map_load_list();

	printf("ENTERING GRAPHIC...\n");
	if(video_init())
		game_halt("Video init failed");

	if(game_pal_get())
		game_halt("Error load palette %s.", FILENAME_PALETTE);
	//чтение изображений
	printf("Images loading...\n");
	pics_load();
	game.m_i_logo     = IMG_connect("M_LOGO"     );
	game.m_i_conback  = IMG_connect("M_CONBACK"  );
	game.m_i_interlv  = IMG_connect("M_I_INTERLV");
	game.m_i_game     = IMG_connect("M_GAME"     );
	game.m_i_g_new_p1 = IMG_connect("M_G_NEW_P1" );
	game.m_i_g_new_p2 = IMG_connect("M_G_NEW_P2" );
	game.m_i_g_load   = IMG_connect("M_G_LOAD"   );
	game.m_i_g_save   = IMG_connect("M_G_SAVE"   );
	game.m_i_case     = IMG_connect("M_CASE"     );
	game.m_i_options  = IMG_connect("M_OPTIONS"  );
	game.m_i_about    = IMG_connect("M_ABOUT"    );
	game.m_i_abort    = IMG_connect("M_ABORT"    );
	game.m_i_quit     = IMG_connect("M_QUIT"     );
	game.m_i_cur_0    = IMG_connect("M_CUR_0"    );
	game.m_i_cur_1    = IMG_connect("M_CUR_1"    );
	game.m_i_arrowL   = IMG_connect("M_ARROWL"   );
	game.m_i_arrowR   = IMG_connect("M_ARROWR"   );
	game.m_i_lineL    = IMG_connect("M_LINEL"    );
	game.m_i_lineM    = IMG_connect("M_LINEM"    );
	game.m_i_lineR    = IMG_connect("M_LINER"    );
	game.m_i_flagRUS  = IMG_connect("F_RUS"      );
	game.m_i_flagUSA  = IMG_connect("F_USA"      );
	game.w_w0         = IMG_connect("W_W0"       );
	game.w_w1         = IMG_connect("W_W1"       );
	game.w_brick      = IMG_connect("W_BRICK"    );
	game.w_water[0]   = IMG_connect("WATER0"     );
	game.w_water[1]   = IMG_connect("WATER1"     );
	game.w_water[2]   = IMG_connect("WATER2"     );
	game.i_health     = IMG_connect("I_HEALTH"   );
	game.i_armor      = IMG_connect("I_ARMOR"    );
	game.i_star       = IMG_connect("I_STAR"     );

	//чтение конфига
	printf("Config init...\n");
	game_cfg_load();

	//инициализация оружий
	printf("Weapons init...\n");
	strcpy(wtable[0].name, "Pulki");                                     //название оружия
	wtable[0].damage     = 15;                                          //повреждение
	wtable[0].selfdamage = 7;//0;                                       //повреждение
	wtable[0].ammo       = c_WEAP_indefinit;                            //макс кол-во боеприпасов
	wtable[0].radius     = 7;                                           //радиус действия
	wtable[0].range      = -1;                                          //дальность
	wtable[0].bullspeed  = 75;                                         //скорость пули
	wtable[0].bullbox    = 2;                                           //bodybox
	wtable[0].icon       = IMG_connect("W_BULL"     );     //изображение оружия
	strcpy(wtable[1].name, "Rocket");                                    //название оружия
	wtable[1].damage     = 100;                                         //повреждение
	wtable[1].selfdamage = 50;                                          //повреждение
	wtable[1].ammo       = 50;                                          //макс кол-во боеприпасов
	wtable[1].radius     = 11;                                          //радиус действия
	wtable[1].range      = -1;                                          //дальность
	wtable[1].bullspeed  = 80;                                         //скорость пули
	wtable[1].bullbox    = 8;                                           //bodybox
	wtable[1].icon       = IMG_connect("W_ROCKET"   );     //изображение оружия
	strcpy(wtable[2].name, "Mine");                                      //название оружия
	wtable[2].damage     = 200;                                         //повреждение
	wtable[2].selfdamage = 100;                                         //повреждение
	wtable[2].ammo       = 50;                                          //макс кол-во боеприпасов
	wtable[2].radius     = 11;                                          //радиус действия
	wtable[2].range      = 100;                                         //дальность
	wtable[2].bullspeed  = -80;                                        //скорость пули
	wtable[2].bullbox    = 8;                                           //bodybox
	wtable[2].icon       = IMG_connect("W_MINE"     );     //изображение оружия
	input_init();

	game_rebind_keys_all();
}

/*
 * подготовка к завершению игры
 */
void game_done()
{
	input_done();
	video_done();
	//прекратим игру
	game_abort();
	//очистим список карт
	map_list_removeall();
	//очистим память от изображений
	IMG_removeall();
};

/*
 *
 */
void game_main()
{
	bool quit = false;
	menu_selector_t imenu = MENU_MAIN;
	imenu = MENU_MAIN;
	menu_selector_t imenu_process = imenu;

	unsigned long time_prev;
	unsigned long time_current = system_getTime_realTime_ms();
	while(!quit)
	{
		time_prev = time_current;
		time_current = system_getTime_realTime_ms();
		dtime = time_current - time_prev;
		dtimed = (double)dtime;
		dtimed1000 = dtimed/1000.0f;

		//printf("time0 = %ld dtime = %ld\n", time_current, dtime);

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			//printf("event.type = %d\n", event.type);
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
				menu_send_event(&event);

			switch(event.type)
			{
			case SDL_QUIT:quit = true;break;
			}

			// handle your event here
		}
		if(quit)break;
		video_screen_draw_begin();

		if(!game.ingame)
		{
			imenu_process = imenu;
			imenu = menu_handle(imenu_process);
			switch(imenu_process)
			{
			case MENU_QUIT: quit = true;break;
			default: break;
			}
			if(quit)break;
		}
		else
		{
			int ret = game_mainproc();
			if(ret >= 0) imenu = ret;

		}



		if(!game.ingame)
		{
			menu_draw(imenu_process);
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
int game_mainproc()
{
	players_control();
	bull_control();
	explode_control();

	if(
			game._win_ &&                                           //победа
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
			game_nextmap();
			return MENU_INTERLEVEL;
		}
	}
	return MENU_MAIN;
}

void game_draw()
{

	map_draw(&game.P0cam);
	if(game.P1) map_draw(&game.P1cam);


	if(game.P0->bull)
	{
		game.P0cam.pos.x = game.P0->bull->orig.x;
		game.P0cam.pos.y = game.P0->bull->orig.y;
	}
	else
	{
		game.P0cam.pos.x = game.P0->move.orig.x;
		game.P0cam.pos.y = game.P0->move.orig.y;
	}
	player_draw_status(&game.P0cam, game.P0);
	if(game.P1)
	{
		if(game.P1->bull)
		{
			game.P1cam.pos.x = game.P1->bull->orig.x;
			game.P1cam.pos.y = game.P1->bull->orig.y;
		}
		else
		{
			game.P1cam.pos.x = game.P1->move.orig.x;
			game.P1cam.pos.y = game.P1->move.orig.y;
		}
		player_draw_status(&game.P1cam, game.P1);
	}


	player_t * player = playerList;
	while(player)
	{
		player_draw(&game.P0cam, player, true);
		if(game.P1) player_draw(&game.P1cam, player, false);
		player = player->next;
	}

	if(game.msg)
	{
		int count = 0;
		gr2D.color.current = 1;
		while(count<64 && game.msg[count])
		{
			int x = ((count * 8 ) % 128)+96;
			int y = ((count / 16) * 8  )+84;
			gr2Don_setchar(x, y, game.msg[count]);
			count++;
		};
		game.msg = NULL;
	};

}

/*
 * процедура перехода на следующую карту
 */
void game_nextmap()
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
	if(game.P1) game.P1->charact.spawned = false;
	if(game.gamemap->next)
	{
		game.gamemap = game.gamemap->next;
		ret = map_load(game.gamemap->map);
		if(ret)
		{
			game_msg_error(ret);
			game_abort();
			//game.menu = c_m_main;
		}
		else
		{
			ret = game_create();
			if(!ret)
			{
				//game.menu = menu_save();       //сохраним игру в начале уровня
				//menu_prelevel();
			}
		}
	}
	else
	{
		game.gamemap = mapList;
		player_disconnect_all();                //дисконнект всех монстров
		//game.menu = c_m_main;
	}
}


/*
 * запись конфига
 */
int game_cfg_save()
{
	int ret = 0;
	ssize_t count;
	int fd;
    fd = open(
            BASEDIR FILENAME_CONFIG,
            O_CREAT | O_WRONLY,
            S_IRUSR | S_IWUSR
    );
	if(fd <= 0)
	{
		ret = 1;
		goto __end;
	}
	count = write(fd, game.controls, sizeof(game.controls));
	if(count != sizeof(game.controls))
	{
		ret = 2;
		goto __end;
	}
	__end:
	switch(ret)
	{
		case 0:
		case 2:
			close(fd);
		case 1:;
	}
	return ret ? -1 : 0;
};
/*
 * новый конфиг
 */
int game_cfg_new()
{
	game.controls[ACTION_ENTER_MAINMENU   ]     = SDL_SCANCODE_ESCAPE;
	game.controls[ACTION_PLAYER_MOVE_UP   ]     = SDL_SCANCODE_UP;
	game.controls[ACTION_PLAYER_MOVE_DOWN ]     = SDL_SCANCODE_DOWN ;
	game.controls[ACTION_PLAYER_MOVE_LEFT ]     = SDL_SCANCODE_LEFT ;
	game.controls[ACTION_PLAYER_MOVE_RIGHT]     = SDL_SCANCODE_RIGHT;
	game.controls[ACTION_PLAYER_ATTACK_WEAPON1] = SDL_SCANCODE_SLASH;
	game.controls[ACTION_PLAYER_ATTACK_WEAPON2] = SDL_SCANCODE_PERIOD;
	game.controls[ACTION_PLAYER_ATTACK_WEAPON3] = SDL_SCANCODE_COMMA ;
	game.controls[ACTION_PLAYER2_MOVE_UP   ]     = SDL_SCANCODE_R;
	game.controls[ACTION_PLAYER2_MOVE_DOWN ]     = SDL_SCANCODE_F;
	game.controls[ACTION_PLAYER2_MOVE_LEFT ]     = SDL_SCANCODE_D;
	game.controls[ACTION_PLAYER2_MOVE_RIGHT]     = SDL_SCANCODE_G;
	game.controls[ACTION_PLAYER2_ATTACK_WEAPON1] = SDL_SCANCODE_W;
	game.controls[ACTION_PLAYER2_ATTACK_WEAPON2] = SDL_SCANCODE_Q;
	game.controls[ACTION_PLAYER2_ATTACK_WEAPON3] = SDL_SCANCODE_TAB;
	game.controls[ACTION_CHEAT_WIN] = SDL_SCANCODE_Z;
	return game_cfg_save();
}
/********чтение конфига********/
int game_cfg_load()
{
	int ret = 0;
	int fd;
	ssize_t count;
	fd = open(BASEDIR FILENAME_CONFIG, O_RDONLY);
	if(fd < 0) return game_cfg_new();
	count = read(fd, game.controls, sizeof(game.controls));
	if(count != sizeof(game.controls))
	{
		ret = 2;
		goto end;
	}
	end:
	switch(ret)
	{
	case 0:
	case 2:
		close(fd);
	case 1:;
	}
	game.controls[ACTION_ENTER_MAINMENU   ]     = SDL_SCANCODE_ESCAPE;
	game.controls[ACTION_CHEAT_WIN] = SDL_SCANCODE_Z;
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
	char * path = Z_malloc(strlen(BASEDIR SAVESDIR)+strlen(savename) + 1);
	strcpy(path, BASEDIR SAVESDIR);
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
	strncpy(rec->name, header.name, 16);
	strncpy(rec->mapfilename, header.mapfilename, 16);
	rec->flags = header.flags;
	close(fd);
	rec->exist = true;
	Z_free(path);
	return true;
};

static char * _make_gamesave_filename(char * filename, int i)
{
	sprintf(filename, "ut_s%02d." FILENAME_GAMESAVE_EXT, i);
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
        size_t size = strlen(map_class_names[MAP_SPAWN_PLAYER]) + 1;
        CHECK(write(fd, map_class_names[MAP_SPAWN_PLAYER], size), size);
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
    mobj_type_t mobj_type = map_file_class_get(fd);
    if(mobj_type != MAP_SPAWN_PLAYER)
    {
        return false;
    }
    game_savedata_player_t savedata;
    ssize_t c = read(fd, &savedata, sizeof(savedata));
    if(c != sizeof(savedata))
    {
        return false;
    }
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
	char * path;

	//если папка "SAVES" отсутствует, тогда создадим ее
	path = Z_malloc(strlen(BASEDIR SAVESDIR)+1);

	strcpy(path, BASEDIR SAVESDIR);
	DIR * dir = opendir(path);
	if(!dir) mkdir(path, 0755);
	else closedir(dir);

	Z_free(path);

	path = Z_malloc(strlen(BASEDIR SAVESDIR) + strlen(filename) + 1);
	strcpy(path, BASEDIR SAVESDIR);
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
	strncpy(header.name, rec->name, 16);
	strncpy(header.mapfilename, rec->mapfilename, 16);
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
	char filename[16];
	_make_gamesave_filename(filename, isave);
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

	char * path = Z_malloc(sizeof(BASEDIR SAVESDIR) + sizeof(filename) + 1);

	strcpy(path, BASEDIR SAVESDIR);
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
	strncpy(rec->name, header.name, 16);
	strncpy(rec->mapfilename, header.mapfilename, 16);
	rec->flags = header.flags;

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

	int cam_sx = 320;
	int cam_sy = 200;

	int pixels = (cam_sy*8)/200;
	if(game.created) return 1;
	if((game.flags & c_g_f_2PLAYERS) == 0)
	{
        game.P0cam.pos.x = 0;
        game.P0cam.pos.y = 0;
        game.P0cam.x     = 0;
        game.P0cam.y     = 0;
        game.P0cam.sx    = cam_sx;
        game.P0cam.sy    = cam_sy - pixels*2;//184
        res = player_connect(c_p_P0);
        if(res) return res;
	}
	else
	{
		game.P0cam.pos.x = 0;
		game.P0cam.pos.y = 0;
		game.P0cam.x      = cam_sx/2+1;
		game.P0cam.y      = 0;
		game.P0cam.sx     = cam_sx/2-1;
		game.P0cam.sy     = cam_sy - pixels*2;
		game.P1cam.pos.x = 0;
		game.P1cam.pos.y = 0;
		game.P1cam.x      = 0;
		game.P1cam.y      = 0;
		game.P1cam.sx     = cam_sx-1;
		game.P1cam.sy     = cam_sy - pixels*2;
        res = player_connect(c_p_P0);
        if(res) return res;
        res = player_connect(c_p_P1);
        if(res) return res;
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
		gr2D.color.current = 26; gr2D_rectangle_f(x,y,sx,sy);
		gr2D.color.current = 30; gr2D_line_h(x     ,x+sx-1,y     );          //верхний борт
		gr2D.color.current = 22; gr2D_line_h(x     ,x+sx-1,y+sy-1);          //нижний  борт
		gr2D.color.current = 29; gr2D_line_v(x     ,y     ,y+sy-1);          //левый   борт
		gr2D.color.current = 23; gr2D_line_v(x+sx-1,y     ,y+sy-1);          //правый  борт
		gr2D.color.current =  4; gr2Don_settext(x+(sx / 2)-6*8, y+2, orient_horiz, "ERROR: ");
		gr2D.color.current = 15;
		int e;
		if(error <= 5)e = error;
		else e = error - 5;
		if(e > ERR_MAX) e = ERR_MAX;

		gr2Don_settext(x+2, y+16, orient_horiz, errList[e]);

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

void game_console_send(const char *error, ...)
{

}

