/*
 * Ubivat Tank
 * Главное тело
 * by Master San
 */

#include <defs.h>
#include <game.h>
#include <img.h>
#include <map.h>
#include <plr.h>
#include <weap.h>
#include <menu.h>
#include <ctrl.h>

#include <_gr2D.h>
#include <_gr2Don.h>
#include <x10_str.h>
#include <x10_kbrd.h>
#include <x10_time.h>

#include <stdio.h>
#include <string.h>
//игрок


player_t *player;

/*
 * подготовка к завершению игры
 */
void game_done()
{
	//прекратим игру
	game_abort();
	//очистим список карт
	map_list_removeall();
	//очистим память от изображений
	IMG_removeall(&game.HEADimg);
};
/********процедура перехода на следующую карту********/
void game_nextmap()
{
	player_disconnect_monsters();                                          //дисконнект всех монстров
	explode_removeall();
	bull_removeall();
	map_close();                                   //закроем карту
	game.created    = false;
	game.ingame     = false;
	game._win_      = false;
	menu_interlv();
	game.P0->charact.spawned = false;
	if(game.P1) game.P1->charact.spawned = false;
	if(game.gamemap->next)
	{
		game.gamemap = game.gamemap->next;
		game.error = map_load(game.gamemap->map);
		if(game.error != 0) {
			game_msg_error(game.error);
			game_abort();
			game.menu = c_m_main;
			game.allowpress = false;
		}
		else
		{
			game.error = game_create();
			if(!game.error)
			{
				game.menu = menu_save();       //сохраним игру в начале уровня
				menu_prelv();
				game_time_reset();
			}
		}
	}
	else
	{
		game.gamemap = mapList;
		player_disconnect_all();                //дисконнект всех монстров
		game.menu = c_m_main;
		game.allowpress = false;
	};
};
/*
 * открытие файлов с рисунками меню
 */
int pics_menu_load()
{
	int error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/mhlogo" ,"M_LOGO"     );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/conback","M_CONBACK"  );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/intervl","M_I_INTERLV");if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/game"   ,"M_GAME"     );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/new_p1" ,"M_G_NEW_P1" );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/new_p2" ,"M_G_NEW_P2" );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/load"   ,"M_G_LOAD"   );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/save"   ,"M_G_SAVE"   );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/case"   ,"M_CASE"     );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/options","M_OPTIONS"  );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/about"  ,"M_ABOUT"    );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/abort"  ,"M_ABORT"    );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/quit"   ,"M_QUIT"     );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/cur_0"  ,"M_CUR_0"    );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/cur_1"  ,"M_CUR_1"    );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/arrowl" ,"M_ARROWL"   );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/arrowr" ,"M_ARROWR"   );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/linel"  ,"M_LINEL"    );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/linem"  ,"M_LINEM"    );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"menu/liner"  ,"M_LINER"    );if(error) return error;
	return 0;
};
/*
 * открытие файлов с рисунками
 */
int pics_load()
{
	int error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/tank0"   ,"TANK0"      );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/tank1"   ,"TANK1"      );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/tank2"   ,"TANK2"      );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/tank3"   ,"TANK3"      );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/tank4"   ,"TANK4"      );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/f_rus"   ,"F_RUS"      );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/f_usa"   ,"F_USA"      );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/F_white" ,"F_WHITE"    );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/w_w0"    ,"W_W0"       );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/w_w1"    ,"W_W1"       );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/w_brick" ,"W_BRICK"    );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/water0"  ,"WATER0"     );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/water1"  ,"WATER1"     );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/water2"  ,"WATER2"     );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/i_health","I_HEALTH"   );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/i_armor" ,"I_ARMOR"    );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/i_star"  ,"I_STAR"     );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/o_exit"  ,"O_EXIT"     );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/w_bull"  ,"W_BULL"     );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/w_rocket","W_ROCKET"   );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/w_mine"  ,"W_MINE"     );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/b_bull"  ,"B_BULL"     );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/b_rocket","B_ROCKET"   );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/b_mine"  ,"B_MINE"     );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/e_small" ,"E_SMALL"    );if(error) return error;
	error = IMG_add(&game.HEADimg,BASEDIR"pics/e_big"   ,"E_BIG"      );if(error) return error;
	return 0;
}


/*
 * управление вражеским игроком
 */
void ctrl_enemy(player_t * player)
{
	if(0<player->charact.health)
	{
		ctrl_AI_checkdanger(player);
		if(!player->brain.danger)
		{
			if(!game.P1)
			{
				if(game.P0->charact.health<=0) player->w.attack = 0;
				else {
					if(!player->bull &&  !player->w.attack) ctrl_AI_findenemy(player, game.P0);
					ctrl_AI_attack(player,game.P0);
				};
			}
			else
			{
				if(xrand(2)==0)
				{
					if(0<game.P0->charact.health)
					{
						if( !player->bull && !player->w.attack) ctrl_AI_findenemy(player,game.P0);
						ctrl_AI_attack(player,game.P0);
					}
				}
				else
				{
					if(0 < game.P1->charact.health)
					{
						if(!player->bull && !player->w.attack) ctrl_AI_findenemy(player,game.P1);
						ctrl_AI_attack(player,game.P1);
					}
				}
			}
		}
	}
}
/*
 * главная процедура игры
 */
void game_mainproc()
{
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
					if(game.mess)
					{
						count = 0;
						gr2D.color.current = 1;
						while(count<64 && game.mess[count])
						{
							x = ((count << 3)% 128)+96;
							y = ((count >> 4)<< 3)+84;
							gr2Don_setchar(x,y,game.mess[count]);
							count++;
						};
						game.mess = NULL;
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
					game.menu       = c_m_main;
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
				menu_interlv();
				game_abort();
			};
		};
	};
};

void game_main()
{
	randomize();
	printf("%s\n", c_strTITLE);
	printf("%s\n", c_strCORP);
	printf("          _____    ______              __     ______ \n");
	printf("||    || |  __ \\  |__  __| ||    ||   /  \\   |__  __|\n");
	printf("||    || | |  \\ |    ||    ||    ||  / /\\ \\     ||   \n");
	printf("||    || | |__//     ||    ||    || | |__| |    ||   \n");
	printf("||    || | |  \\\\     ||    ||    || | ____ |    ||   \n");
	printf("\\\\____|| | |__/ |  __||__   \\\\__//  ||    ||    ||   \n");
	printf(" \\_____| |_____/  |______|   \\__/   ||    ||    ||   \n");
	printf("\n");
	printf("                       T A N K\n");
	printf("\n");
	//чтение списка карт
	printf("Maps list loading: ");
	if(map_load_list())
	{
		game_halt("Could not load %s", FILENAME_MAPSLIST);
	}
	if(!mapList)
	{
		game_halt("Maps list is empty");
	}
	printf("OK");
	//инициализация переменных
	printf("Variables init     : ");
	game.P0         = NULL;
	game.P1         = NULL;
	bullList   = NULL;
	explList   = NULL;
	game.HEADimg    = NULL;
	game.mess       = NULL;
	game.created    = false;
	game.allowpress = true;
	game.quit       = false;
	game.ingame     = false;
	game.menu       = c_m_main;
	game.error      = 0;
	map.HEADspawn   = NULL;
	map.HEADitem    = NULL;
	map.HEADobj     = NULL;
	printf("OK");
	//чтение изображений меню
	printf("Menu images loading: ");
	game.error = pics_menu_load();
	if(!game.error) printf("OK");
	else
	{
		game_done();
		printf("FAULT. HALTING.");
		game_msg_error_fatal(game.error);
		return;
	};
	//чтение изображений
	printf("Images loading     : ");
	game.error = pics_load();
	if(!game.error) printf("OK");
	else
	{
		game_done();
		printf("FALSE. HALTING.");
		game_msg_error_fatal(game.error);
		return;
	};
	//чтение конфига
	printf("Config init        : ");
	game_cfg_load();
	printf("OK");
	//инициализация оружий
	printf("Weapons init     : ");
	strcpy(wtable[0].name, "Pulki");                                     //название оружия
	wtable[0].damage     = 15;                                          //повреждение
	wtable[0].selfdamage = 7;//0;                                       //повреждение
	wtable[0].ammo       = c_WEAP_indefinit;                            //макс кол-во боеприпасов
	wtable[0].radius     = 7;                                           //радиус действия
	wtable[0].range      = -1;                                          //дальность
	wtable[0].bullspeed  = 750;                                         //скорость пули
	wtable[0].bullbox    = 2;                                           //bodybox
	wtable[0].icon       = IMG_connect(game.HEADimg,"W_BULL"     );     //изображение оружия
	strcpy(wtable[1].name, "Rocket");                                    //название оружия
	wtable[1].damage     = 100;                                         //повреждение
	wtable[1].selfdamage = 50;                                          //повреждение
	wtable[1].ammo       = 50;                                          //макс кол-во боеприпасов
	wtable[1].radius     = 11;                                          //радиус действия
	wtable[1].range      = -1;                                          //дальность
	wtable[1].bullspeed  = 800;                                         //скорость пули
	wtable[1].bullbox    = 8;                                           //bodybox
	wtable[1].icon       = IMG_connect(game.HEADimg,"W_ROCKET"   );     //изображение оружия
	strcpy(wtable[2].name, "Mine");                                      //название оружия
	wtable[2].damage     = 200;                                         //повреждение
	wtable[2].selfdamage = 100;                                         //повреждение
	wtable[2].ammo       = 50;                                          //макс кол-во боеприпасов
	wtable[2].radius     = 11;                                          //радиус действия
	wtable[2].range      = 100;                                         //дальность
	wtable[2].bullspeed  = -800;                                        //скорость пули
	wtable[2].bullbox    = 8;                                           //bodybox
	wtable[2].icon       = IMG_connect(game.HEADimg,"W_MINE"     );     //изображение оружия
	printf("OK");
	//инициализация графики
	printf("ENTERING GRAPHIC");
	gr2D_init320X200X8();
	if(game_PAL_get())
	{
		game_done();
		gr2D_close();
		printf("Error load palette %s. HALTING.", c_PALname);
		return;
	};
	gr2D_setRGBpal(&gr2D.PAL);
	gr2D.color.current = 0;
	//очищаем буфер
	gr2D_BUFcurrentfill();
	gr2D.color.transparent = 255;
	game.m_i_logo     = IMG_connect(game.HEADimg,"M_LOGO"     );
	game.m_i_conback  = IMG_connect(game.HEADimg,"M_CONBACK"  );
	game.m_i_interlv  = IMG_connect(game.HEADimg,"M_I_INTERLV");
	game.m_i_game     = IMG_connect(game.HEADimg,"M_GAME"     );
	game.m_i_g_new_p1 = IMG_connect(game.HEADimg,"M_G_NEW_P1" );
	game.m_i_g_new_p2 = IMG_connect(game.HEADimg,"M_G_NEW_P2" );
	game.m_i_g_load   = IMG_connect(game.HEADimg,"M_G_LOAD"   );
	game.m_i_g_save   = IMG_connect(game.HEADimg,"M_G_SAVE"   );
	game.m_i_case     = IMG_connect(game.HEADimg,"M_CASE"     );
	game.m_i_options  = IMG_connect(game.HEADimg,"M_OPTIONS"  );
	game.m_i_about    = IMG_connect(game.HEADimg,"M_ABOUT"    );
	game.m_i_abort    = IMG_connect(game.HEADimg,"M_ABORT"    );
	game.m_i_quit     = IMG_connect(game.HEADimg,"M_QUIT"     );
	game.m_i_cur_0    = IMG_connect(game.HEADimg,"M_CUR_0"    );
	game.m_i_cur_1    = IMG_connect(game.HEADimg,"M_CUR_1"    );
	game.m_i_arrowL   = IMG_connect(game.HEADimg,"M_ARROWL"   );
	game.m_i_arrowR   = IMG_connect(game.HEADimg,"M_ARROWR"   );
	game.m_i_lineL    = IMG_connect(game.HEADimg,"M_LINEL"    );
	game.m_i_lineM    = IMG_connect(game.HEADimg,"M_LINEM"    );
	game.m_i_lineR    = IMG_connect(game.HEADimg,"M_LINER"    );
	game.m_i_flagRUS  = IMG_connect(game.HEADimg,"F_RUS"      );
	game.m_i_flagUSA  = IMG_connect(game.HEADimg,"F_USA"      );
	game.w_w0         = IMG_connect(game.HEADimg,"W_W0"       );
	game.w_w1         = IMG_connect(game.HEADimg,"W_W1"       );
	game.w_brick      = IMG_connect(game.HEADimg,"W_BRICK"    );
	game.w_water[0]   = IMG_connect(game.HEADimg,"WATER0"     );
	game.w_water[1]   = IMG_connect(game.HEADimg,"WATER1"     );
	game.w_water[2]   = IMG_connect(game.HEADimg,"WATER2"     );
	game.i_health     = IMG_connect(game.HEADimg,"I_HEALTH"   );
	game.i_armor      = IMG_connect(game.HEADimg,"I_ARMOR"    );
	game.i_star       = IMG_connect(game.HEADimg,"I_STAR"     );
	//опустошим буфер клавы
	kbrd_readport();
	kbrd.port = 0;
	while(!game.quit)
	{
		switch(game.menu)
		{
		case c_m_main      : game.menu = menu_main();break;
		case c_m_game      : game.menu = menu_game();break;
		case c_m_game_new1P:
			if(game.created) game.menu = c_m_main;
			else
			{
				game.gamemap = mapList;
				game.error = map_load(game.gamemap->map);
				if(game.error) game_msg_error(game.error);
				else
				{
					game.flags = 0;
					game.error = game_create();
					menu_prelv();
				};
			}
			break;
		case c_m_game_new2P:
			if(game.created) game.menu = c_m_main;
			else
			{
				game.gamemap = mapList;
				game.error = map_load(game.gamemap->map);
				if(game.error) game_msg_error(game.error);
				else
				{
					game.flags = c_g_f_2PLAYERS;
					game.error = game_create();
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
				game.error = map_load(game.casemap->map);
				if(game.error) game_msg_error(game.error);
				else
				{
					game.flags = c_g_f_CASE;
					game.error = game_create();
				}
			}
			break;
		case c_m_case_newP2:
			if(game.created) game.menu = c_m_main;
			else
			{
				game.error = map_load(game.casemap->map);
				if(game.error != 0) game_msg_error(game.error);
				else
				{
					game.flags = c_g_f_2PLAYERS | c_g_f_CASE;
					game.error = game_create();
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
		if(game.error != 0)
		{
			game.menu = c_m_main;
			game_abort();
			game.error = 0;
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
	};
	game_done();
	gr2D_close();
}

int main()
{
	game_main();
	return 0;
}

