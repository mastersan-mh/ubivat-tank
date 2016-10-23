/**
 *        Ubivat Tank
 *        игра
 * by Master San
 */


#include <game.h>
#include <actions.h>
#include <menu.h>
#include <map.h>
#include <weap.h>
#include <video.h>
#include <_gr2D.h>
#include <_gr2Don.h>
#include <x10_str.h>
#include <x10_time.h>
#include <x10_kbrd.h>
#include <x10_str.h>

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

//состояние игры
game_t game;


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


/*
 * открытие файлов с рисунками меню
 */
void pics_load()
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

void game_init()
{
	game.P0         = NULL;
	game.P1         = NULL;
	game.msg        = NULL;
	game.created    = false;
	game.allowpress = true;
	game.quit       = false;
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
	video_transparent_set(255);


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
	if(game_cfg_load())
	{
		game_halt("config read error.");
	}
	action_init();

	//инициализация оружий
	printf("Weapons init...\n");
	strcpy(wtable[0].name, "Pulki");                                     //название оружия
	wtable[0].damage     = 15;                                          //повреждение
	wtable[0].selfdamage = 7;//0;                                       //повреждение
	wtable[0].ammo       = c_WEAP_indefinit;                            //макс кол-во боеприпасов
	wtable[0].radius     = 7;                                           //радиус действия
	wtable[0].range      = -1;                                          //дальность
	wtable[0].bullspeed  = 750;                                         //скорость пули
	wtable[0].bullbox    = 2;                                           //bodybox
	wtable[0].icon       = IMG_connect("W_BULL"     );     //изображение оружия
	strcpy(wtable[1].name, "Rocket");                                    //название оружия
	wtable[1].damage     = 100;                                         //повреждение
	wtable[1].selfdamage = 50;                                          //повреждение
	wtable[1].ammo       = 50;                                          //макс кол-во боеприпасов
	wtable[1].radius     = 11;                                          //радиус действия
	wtable[1].range      = -1;                                          //дальность
	wtable[1].bullspeed  = 800;                                         //скорость пули
	wtable[1].bullbox    = 8;                                           //bodybox
	wtable[1].icon       = IMG_connect("W_ROCKET"   );     //изображение оружия
	strcpy(wtable[2].name, "Mine");                                      //название оружия
	wtable[2].damage     = 200;                                         //повреждение
	wtable[2].selfdamage = 100;                                         //повреждение
	wtable[2].ammo       = 50;                                          //макс кол-во боеприпасов
	wtable[2].radius     = 11;                                          //радиус действия
	wtable[2].range      = 100;                                         //дальность
	wtable[2].bullspeed  = -800;                                        //скорость пули
	wtable[2].bullbox    = 8;                                           //bodybox
	wtable[2].icon       = IMG_connect("W_MINE"     );     //изображение оружия
	//инициализация графики


	gr2D.color.current = 0;
	//очищаем буфер
	gr2D_BUFcurrentfill();

}

/*
 * подготовка к завершению игры
 */
void game_done()
{
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
	menu_selector_t menu = c_m_main;
	int menu_cur = 0;
	while(!quit)
	{

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			printf("event.type = %d\n", event.type);
			switch(event.type)
			{
			case SDL_QUIT:quit = true;break;
			}
			// handle your event here
		}
		if(quit)break;
		video_scree_draw_begin();


		switch(menu)
		{
		case c_m_main : menu_main_draw(&menu_cur);break;
		default: ;
		}

		//video_draw();


		video_scree_draw_end();

/*
		switch(game.menu)
		{
		case c_m_main      : game.menu = menu_main();break;
		case c_m_game      : game.menu = menu_game();break;
		case c_m_game_new1P:
			if(game.created) game.menu = c_m_main;
			else
			{
				game.gamemap = mapList;
				ret = map_load(game.gamemap->map);
				if(ret) game_msg_error(ret);
				else
				{
					game.flags = 0;
					ret = game_create();
					menu_prelv();
				};
			}
			break;
		case c_m_game_new2P:
			if(game.created) game.menu = c_m_main;
			else
			{
				game.gamemap = mapList;
				ret = map_load(game.gamemap->map);
				if(ret) game_msg_error(ret);
				else
				{
					game.flags = c_g_f_2PLAYERS;
					ret = game_create();
					menu_prelv();
				}
			}
			break;
		case c_m_game_load : game.menu = menu_load();break;
		case c_m_case      : game.menu = menu_case();break;
		case c_m_case_case : game.menu = menu_case();break;
		case c_m_case_newP1:
			if(game.created) game.menu = c_m_main;
			else
			{
				ret = map_load(game.casemap->map);
				if(ret) game_msg_error(ret);
				else
				{
					game.flags = c_g_f_CASE;
					ret = game_create();
				}
			}
			break;
		case c_m_case_newP2:
			if(game.created) game.menu = c_m_main;
			else
			{
				ret = map_load(game.casemap->map);
				if(ret) game_msg_error(ret);
				else
				{
					game.flags = c_g_f_2PLAYERS | c_g_f_CASE;
					ret = game_create();
				};
			};
			break;
		case c_m_options   : game.menu = menu_options();break;
		case c_m_about     : game.menu = menu_about();break;
		case c_m_abort     :
			game_abort();
			game.menu = c_m_main;
			game.allowpress = false;
			break;
		case c_m_quit: game.quit = true;break;
		}
		if(ret != 0)
		{
			game.menu = c_m_main;
			game_abort();
			ret = 0;
			game.allowpress = true;
		}
		else
		{
			if(game.created && game.ingame)
			{
				game_time_reset();
				game_mainproc();                                                     //главная процедура игры
			};
		};
		gr2D_BUFcurrent2screen();
		*/
	};
}


/*
 * главная процедура игры
 */
void game_mainproc()
{
	player_t *player;
	int count;
	int x,y;

	while(game.ingame)
	{
		player = playerList;
		while(player)
		{
			if(player->charact.status==c_p_BOSS || player->charact.status==c_p_ENEMY)
			{
				ctrl_enemy(player);
			}
			else {
				if(player->charact.status==c_p_P1)
				{
					if(player->bull) { game.P1cam.orig.x = player->bull->orig.x;game.P1cam.orig.y = player->bull->orig.y;}
					else             { game.P1cam.orig.x = player->move.orig.x ;game.P1cam.orig.y = player->move.orig.y ;};
					ctrl_human(1,player);
					player_draw_status(&game.P1cam, game.P1);
				}
				else
				{
					bull_control();
					if(player->bull) { game.P0cam.orig.x = player->bull->orig.x;game.P0cam.orig.y = player->bull->orig.y;}
					else             { game.P0cam.orig.x = player->move.orig.x ;game.P0cam.orig.y = player->move.orig.y ;};
					player_checkcode();
					ctrl_human(0,player);
					explode_control();
					player_draw_status(&game.P0cam,game.P0);
					if(game.msg)
					{
						count = 0;
						gr2D.color.current = 1;
						while(count<64 && game.msg[count])
						{
							x = ((count << 3)% 128)+96;
							y = ((count >> 4)<< 3)+84;
							gr2Don_setchar(x,y,game.msg[count]);
							count++;
						};
						game.msg = NULL;
					};
					gr2D_BUFcurrent2screen();
					gr2D.color.current = 0;
					gr2D_BUFcurrentfill();
					map_draw(&game.P0cam);                                            //рисуем карту игрока0
					if(game.P1) map_draw(&game.P1cam);                       //рисуем карту игрока1
				}
			}
			player_draw(&game.P0cam, player, true);
			if(game.P1) player_draw(&game.P1cam,player,false);
			player_control(player);
			player = player->next;
		}
		if(kbrd.port)
		{
			switch(kbrd.port)
			{
			case KP0_ESCAPE_1:
				if(game.allowpress)
				{
					game.ingame     = false;
					//game.menu       = c_m_main;
					game.allowpress = false;
				};
				break;
			case KP0_ESCAPE_0: game.allowpress = true;break;
			}
		}
		if(
				(game._win_) &&                                                 //победа
				(game.P0->charact.health>0) &&                                     //оба игрока должны быть живы
				((!game.P1) || ((game.P1) && (game.P1->charact.health>0)))  //
		)
		{
			if(game.flags & c_g_f_CASE) game_nextmap();                 //игра по уровням
			else
			{                                                         //игра по выбору
				menu_interlevel();
				game_abort();
			}
		}
	}
}

/********процедура перехода на следующую карту********/
void game_nextmap()
{
	int ret;
	player_disconnect_monsters();                                          //дисконнект всех монстров
	explode_removeall();
	bull_removeall();
	map_close();                                   //закроем карту
	game.created    = false;
	game.ingame     = false;
	game._win_      = false;
	menu_interlevel();
	game.P0->charact.spawned = false;
	if(game.P1) game.P1->charact.spawned = false;
	if(game.gamemap->next)
	{
		game.gamemap = game.gamemap->next;
		ret = map_load(game.gamemap->map);
		if(ret != 0) {
			game_msg_error(ret);
			game_abort();
			//game.menu = c_m_main;
			game.allowpress = false;
		}
		else
		{
			ret = game_create();
			if(!ret)
			{
				//game.menu = menu_save();       //сохраним игру в начале уровня
				menu_prelv();
				game_time_reset();
			}
		}
	}
	else
	{
		game.gamemap = mapList;
		player_disconnect_all();                //дисконнект всех монстров
		//game.menu = c_m_main;
		game.allowpress = false;
	}
}


/*
 * запись конфига
 */
int game_cfg_save()
{
	int ret = 0;
	ssize_t c;
	int fd;
	fd = open(BASEDIR FILENAME_CONFIG, O_CREAT | O_WRONLY);
	if(fd <= 0)
	{
		ret = 1;
		goto __end;
	}
	c = write(fd, game.controlP0, sizeof(game.controlP0));
	if(c != sizeof(game.controlP0))
	{
		ret = 2;
		goto __end;
	}
	c = write(fd, game.controlP1, sizeof(game.controlP1));
	if(c != sizeof(game.controlP0))
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
	//игрок 0
	game.controlP0[0]  = KP0_UPARROW_1   ;//вперед
	game.controlP0[1]  = KP0_UPARROW_0   ;//
	game.controlP0[2]  = KP0_DOWNARROW_1 ;//назад
	game.controlP0[3]  = KP0_DOWNARROW_0 ;//
	game.controlP0[4]  = KP0_LEFTARROW_1 ;//влево
	game.controlP0[5]  = KP0_LEFTARROW_0 ;//
	game.controlP0[6]  = KP0_RIGHTARROW_1;//вправо
	game.controlP0[7]  = KP0_RIGHTARROW_0;//
	game.controlP0[8]  = KP0_SLASH1_1    ;//пульки
	game.controlP0[9]  = KP0_SLASH1_0    ;//
	game.controlP0[10] = KP0_POINT_1     ;//ракета
	game.controlP0[11] = KP0_POINT_0     ;//
	game.controlP0[12] = KP0_SEMIPOINT_1 ;//мина
	game.controlP0[13] = KP0_SEMIPOINT_0 ;//
	//игрок 1
	game.controlP1[0]  = KP0_R_1         ;
	game.controlP1[1]  = KP0_R_0         ;
	game.controlP1[2]  = KP0_F_1         ;
	game.controlP1[3]  = KP0_F_0         ;
	game.controlP1[4]  = KP0_D_1         ;
	game.controlP1[5]  = KP0_D_0         ;
	game.controlP1[6]  = KP0_G_1         ;
	game.controlP1[7]  = KP0_G_0         ;
	game.controlP1[8]  = KP0_W_1         ;
	game.controlP1[9]  = KP0_W_0         ;
	game.controlP1[10] = KP0_Q_1         ;
	game.controlP1[11] = KP0_Q_0         ;
	game.controlP1[12] = KP0_TAB_1       ;
	game.controlP1[13] = KP0_TAB_0       ;
	return game_cfg_save();
}
/********чтение конфига********/
int game_cfg_load()
{
	int ret = 0;
	int fd;
	ssize_t c;
	fd = open(BASEDIR FILENAME_CONFIG, O_RDONLY);
	if(fd < 0) return game_cfg_new();
	c = read(fd, game.controlP0, sizeof(game.controlP0));
	if(c != sizeof(game.controlP0))
	{
		ret = 2;
		goto end;
	}
	c = read(fd, game.controlP1, sizeof(game.controlP1));
	if(c != sizeof(game.controlP1))
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
	return ret ? -1 : 0;
}
/********чтение файла палитры********/
//game_PAL_get=0 -успешно
//game_PAL_get=1 -файл не найден
//game_PAL_get=2 -ошибка чтения файла
int game_pal_get()
{
	int ret = 0;
	int fd;
	fd = open(BASEDIR FILENAME_PALETTE, O_RDONLY);
	if(fd < 0)
	{
		ret = 1;
		goto end;
	}
	if(read(fd, gr2D.PAL, 768) != 768)
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
	return ret ? -1 : 0;
}
/********переустановка времени у всех объектов на карте********/
void game_time_reset()
{
	player_t * player = playerList;
	while(player)
	{
		player->time.delta      = 0;
		player->time.last_delta = 0;
		time_Sget();
		player->time.t0         = time.s*100+time.hs;
		player->time.t1         = player->time.t0;
		player = player->next;
	}
	bull_t * bull = bullList;
	while(bull)
	{
		bull->delta_s = 0;                                                   //изменение расстояния
		bull->time.delta      = 0;
		bull->time.last_delta = 0;
		time_Sget();
		bull->time.t0  = time.s*100+time.hs;                                 //системное время в сотых долях секунд
		bull->time.t1         = bull->time.t0;
		bull = bull->next;
	}
	explode_t * explode = explList;
	while(explode)
	{
		explode->time.delta      = 0;
		explode->time.last_delta = 0;
		time_Sget();
		explode->time.t0         = time.s*100+time.hs;
		explode->time.t1         = explode->time.t0;
		explode = explode->next;
	}
}
/**
 * @description чтение заголовка записи
 * @return true | false
 */
static bool game_record_load_info(const char * savename, gamesave_t * rec)
{
	int fd;
	char * path = Z_malloc(strlen(BASEDIR)+strlen(FILENAME_CONFIG) + 1);
	strcpy(rec->_file, savename);
	strcpy(path, BASEDIR);
	strcat(path, SAVESDIR);
	strcat(path, savename);
	strcat(path, c_SAVext);
	fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		return false;
	}
	memset(rec, 0, sizeof(*rec));
	//имя файла карты
	read(fd, rec->Hmap , 9 );rec->Hmap [8]  = 0;
	//внутреннее имя записи
	read(fd, rec->Hname, 17);rec->Hname[16] = 0;
	//флаги состояния игры
	read(fd, &rec->flags, 1 );
	close(fd);
	return true;
};
/*
 * формируем листинг записей
 */
void game_record_getsaves()
{
	int i;
	char _file[9];

	memset(game.saveslist, 0, sizeof(game.saveslist[0])*GAME_SAVESNUM);
	for(i = 0; i < GAME_SAVESNUM; i++)
	{
		game.saveslist[i].Hname[0] = 0xFF;
		sprintf(_file, "UT_S%02x", i);
		game_record_load_info(_file, &(game.saveslist[i]));
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
		write(fd, c_MAP_s_player, 13);
		break;
	default:
		return false;
	};
	write(fd, &player->charact.scores    , 4);
	write(fd, &player->charact.status    , 1);
	write(fd, &player->charact.health    , 2);
	write(fd, &player->charact.armor     , 2);
	write(fd, &player->charact.fragstotal, 4);
	write(fd, &player->charact.frags     , 4);
	write(fd, &player->w.ammo            , 6);
	return true;
};

/**
 * чтение игрока
 * @return true | false
 */
static bool game_record_load_player(int fd, player_t * player)
{
	char class_string[33];
	map_file_class_get(fd, class_string);
	if(strcmp(class_string,c_MAP_s_player) != 0)
	{
		return false;
	}
	read(fd, &player->charact.scores    , 4);
	read(fd, &player->charact.status    , 1);
	read(fd, &player->charact.health    , 2);
	read(fd, &player->charact.armor     , 2);
	read(fd, &player->charact.fragstotal, 4);
	read(fd, &player->charact.frags     , 4);
	read(fd, &player->w.ammo            , 6);
	player_class_init(player);
	return true;
};
/**
 * сохраниние записи
 * @return true| false
 */
bool game_record_save(gamesave_t * rec)
{
	int fd;
	char * path;

	//если папка "SAVES" отсутствует, тогда создадим ее
	path = Z_malloc(
		strlen(BASEDIR)+
		strlen(SAVESDIR)+
		1
		);

	strcpy(path, BASEDIR);
	strcat(path, SAVESDIR);
	DIR * dir = opendir(path);
	if(!dir) mkdir(path, 0755);
	else closedir(dir);

	Z_free(path);

	path = Z_malloc(
		strlen(BASEDIR)+
		strlen(SAVESDIR)+
		strlen(rec->_file)+
		strlen(c_SAVext)+
		1
		);
	strcpy(path, BASEDIR);
	strcat(path, SAVESDIR);
	strcat(path, rec->_file);
	strcat(path, c_SAVext);
	strcpy(rec->Hmap, map._file);

	fd = open(path, O_CREAT | O_WRONLY);
	//имя файла карты
	write(fd, rec->Hmap , 9);
	//внутреннее имя записи
	write(fd, rec->Hname,17);
	//флаги настройки игры
	write(fd, &rec->flags, 1);
	//сохраним 0-го игрока
	game_record_save_player(fd, game.P0);
	//сохраним 1-го игрока
	if(game.P1) game_record_save_player(fd, game.P1);
	close(fd);
	Z_free(path);
	return true;
};
/*
 * чтение сохранённой игры
 * @return = 0 - успешно
 * @return = 1 - запись отсутствует
 * @return = 2 - карта отсутствует в списке карт
 * @return = 3 - ошибка чтения карты
 */
int game_record_load(gamesave_t * rec)
{
	int ret;
	int fd_rec;

	if(rec->Hname[0] == 0xFF)
		return 1;
	if(!(rec->flags & c_g_f_CASE))
	{
		game.gamemap = mapList;
		while(game.gamemap)
		{
			if(strcmp(rec->Hmap, game.gamemap->map)==0)
				return 2;
			game.gamemap = game.gamemap->next;
		}
	};
	//закроем отктытую карту
	map_close();
	//прочитаем карту
	ret = map_load(rec->Hmap);
	if(ret != 0) return 3;
	char * path = Z_malloc(
		sizeof(BASEDIR)+
		sizeof(SAVESDIR)+
		sizeof(rec->_file)+
		sizeof(c_SAVext)+
		1
	);

	strcpy(path, BASEDIR);
	strcat(path, SAVESDIR);
	strcat(path, rec->_file);
	strcat(path, c_SAVext);
	fd_rec = open(path, O_RDONLY);
	//имя файла карты
	read(fd_rec, rec->Hmap , 9);rec->Hmap [ 8] = 0;
	//внутреннее имя записи
	read(fd_rec, rec->Hname,17);rec->Hname[16] = 0;
	//флаги настройки игры
	read(fd_rec, &rec->flags, 1);game.flags = rec->flags;
	//создаем игру и спавним всех игроков
	ret = game_create();
	//читаем первого игрока
	game_record_load_player(fd_rec, game.P0);
	//читаем второго игрока
	if(rec->flags & c_g_f_2PLAYERS)
		game_record_load_player(fd_rec, game.P1);
	close(fd_rec);
	Z_free(path);
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
	int ret;
	if(game.created) return 1;
	if((game.flags & c_g_f_2PLAYERS) == 0)
	{
		game.P0cam.orig.x = 0;
		game.P0cam.orig.y = 0;
		game.P0cam.x      = 0;
		game.P0cam.y      = 0;
		game.P0cam.sx     = 320;
		game.P0cam.sy     = 184;
		ret = player_connect(c_p_P0);
		if(ret)return ret;
	}
	else
	{
		game.P0cam.orig.x = 0;
		game.P0cam.orig.y = 0;
		game.P0cam.x      = 160+1;
		game.P0cam.y      = 0;
		game.P0cam.sx     = 160-1;
		game.P0cam.sy     = 184;
		game.P1cam.orig.x = 0;
		game.P1cam.orig.y = 0;
		game.P1cam.x      = 0;
		game.P1cam.y      = 0;
		game.P1cam.sx     = 160-1;
		game.P1cam.sy     = 184;
		ret = player_connect(c_p_P0);
		if(ret)return ret;
		ret = player_connect(c_p_P1);
		if(ret)return ret;
	};
	//спавним всех игроков и монстров
	player_spawn_all();
	game.created    = true;
	game.ingame     = true;
	game.allowpress = true;
	return 0;
}
/*
 * прерывание игры
 */
void game_abort()
{
	game.gamemap = mapList;
	//дисконнект всех игроков
	player_disconnect_all();
	explode_removeall();
	bull_removeall();
	//закроем карту
	map_close();
	game.created    = false;
	game.ingame     = false;
	game._win_      = false;
	//game.menu       = c_m_main;
	game.allowpress = false;
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
		gr2D.color.current = 0;
		gr2D_BUFcurrentfill();
		gr2D.color.current = 26; gr2D_rectangle_f(x,y,sx,sy);
		gr2D.color.current = 30; gr2D_line_h(x     ,x+sx-1,y     );          //верхний борт
		gr2D.color.current = 22; gr2D_line_h(x     ,x+sx-1,y+sy-1);          //нижний  борт
		gr2D.color.current = 29; gr2D_line_v(x     ,y     ,y+sy-1);          //левый   борт
		gr2D.color.current = 23; gr2D_line_v(x+sx-1,y     ,y+sy-1);          //правый  борт
		gr2D.color.current =  4; gr2Don_settextZ(x+(sx / 2)-6*8,y+2,"ERROR: ");
		gr2D.color.current = 15;
		int e;
		if(error <= 5)e = error;
		else e = error - 5;
		if(e > ERR_MAX) e = ERR_MAX;

		gr2Don_settextZ(x+2, y+16, errList[e]);

		kbrd_readport();
		if(kbrd.port != 0)
		{
			if(game.allowpress && kbrd.port <= 127 )
			{
				do
				{
					kbrd_readport();
				} while(kbrd.port != KP0_ENTER_1);
				quit = true;
			};
			if( !game.allowpress && 127 < kbrd.port ) game.allowpress = true;
		}
	gr2D_BUFcurrent2screen();
	}
}

void game_message_send(const char * mess)
{
	game.msg = (char*)mess;
};



void game_halt(const char *error, ...)
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

