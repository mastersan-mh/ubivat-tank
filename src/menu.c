/*
 *        Ubivat Tank
 *        обработка меню
 * by Master San
 */
#include <menu.h>
#include <game.h>
#include <img.h>
#include <_gr2D.h>
#include <_gr2Don.h>
#include <types.h>
#include <x10_time.h>
#include <x10_kbrd.h>

/* MENU_MAIN */
menu_main_context_t menu_main_context = {};

/* MENU_GAME */
menu_game_context_t menu_game_context = {};

/* MENU_GAME_NEW1P */

/* MENU_GAME_NEW2P */

/* MENU_GAME_LOAD */
menu_game_load_context_t menu_load_context = {};

/* MENU_GAME_SAVE */
menu_game_save_context_t menu_save_context = {};

/* MENU_CUSTOM */
menu_custom_context_t menu_custom_context = {};

/* MENU_CUSTOM_NEWP1 */

/* MENU_CUSTOM_NEWP2 */

/* MENU_OPTIONS */
menu_options_context_t menu_options_context = {};
/* MENU_ABOUT */
menu_about_context_t menu_about_context = {};
/* MENU_INTERLEVEL */
menu_interlevel_context_t menu_interlevel_context = {};
/* MENU_PRELEVEL */
menu_prelevel_context_t menu_prelevel_context = {};

/* MENU_ABORT */

/* MENU_QUIT */
#define KEYBUFFER_SIZE (9)
typedef SDL_Scancode buffer_key_t;
static buffer_key_t buffer[KEYBUFFER_SIZE];
static int buffer_start = 0;
static int buffer_end   = 0;

bool buffer_isEmpty()
{
	return buffer_start == buffer_end;
}

buffer_key_t buffer_dequeue_nowait()
{
	if(buffer_start == buffer_end) return SDL_SCANCODE_UNKNOWN;
	buffer_key_t key = buffer[buffer_start];
	buffer_start++;
	if(buffer_start >= KEYBUFFER_SIZE) buffer_start = 0;
	return key;
}

void menu_send_event(SDL_Event * event)
{
	switch(event->type)
	{
	case SDL_KEYDOWN:
		/*
		event->key.keysym.scancode; //SDL_Scancode
		event->key.keysym.sym; //SDL_Keycode  - для ввода текста
		 */

		if(
				buffer_end + 1 == buffer_start ||
				(buffer_end == KEYBUFFER_SIZE-1 && buffer_start == 0)
		)
			printf("buffer is full!\n");
		else
		{
			buffer[buffer_end] = event->key.keysym.scancode;
			buffer_end++;
			if(buffer_end >= KEYBUFFER_SIZE) buffer_end = 0;
		}
		break;
	case SDL_KEYUP:

		break;
	default: break;
	}
//event->type
}



menu_key_t menu_key_get(int maxmenu)
{
	if(buffer_isEmpty())return NOTHING;
	buffer_key_t key = buffer_dequeue_nowait();
	switch(key)
	{
	case SDL_SCANCODE_RETURN  :
	case SDL_SCANCODE_RETURN2 : return ENTER;
	case SDL_SCANCODE_ESCAPE  : return LEAVE;
	case SDL_SCANCODE_UP      : return UP;
	case SDL_SCANCODE_DOWN    : return DOWN;
	case SDL_SCANCODE_LEFT    : return LEFT;
	case SDL_SCANCODE_RIGHT   : return RIGHT;
	case SDL_SCANCODE_SPACE   : return SPACE;
	default: break;
	}
	return NOTHING;
}

void menu_draw_conback()
{
	gr2D_setimage0(0, 0, game.m_i_conback);
}

/*
 * главное меню
 */
static void menu_main_draw(const void * ctx)
{
	const menu_main_context_t *__ctx = ctx;
	int menu = __ctx->menu;
	menu_draw_conback();
	gr2D_setimage0(277  ,159    ,game.m_i_logo);
	gr2D.color.current = 1;
	gr2Don_settext(1     ,183          , orient_horiz, c_strTITLE);
	gr2Don_settext(1     ,191          , orient_horiz, c_strCORP);
	gr2D_setimage0(120, 30+23*0 ,game.m_i_game);
	gr2D_setimage0(120, 30+23*1 ,game.m_i_case);
	gr2D_setimage0(120, 30+23*2 ,game.m_i_options);
	gr2D_setimage0(120, 30+23*3 ,game.m_i_about);
	if(game.created)
		gr2D_setimage0(120,30+23*4 , game.m_i_abort);
	gr2D_setimage0(120,30+23 * 5   , game.m_i_quit);
	gr2D_setimage0( 97,30+23 * menu, game.m_i_cur_0);
}
int menu_main(menu_key_t menukey, void * ctx)
{
	static menu_selector_t menus[] =
	{
			MENU_GAME,
			MENU_CUSTOM,
			MENU_OPTIONS,
			MENU_ABOUT,
			MENU_ABORT,
			MENU_QUIT
	};
	menu_main_context_t * __ctx  = ctx;
	switch(menukey)
	{
	case NOTHING: break;
	case UP     : if(__ctx->menu > 0  ) __ctx->menu--;break;
	case DOWN   : if(__ctx->menu < 6-1) __ctx->menu++;break;
	case LEFT   : break;
	case RIGHT  : break;
	case ENTER  :
		if(!game.created && __ctx->menu == 4)return MENU_MAIN;
		return menus[__ctx->menu];
	case LEAVE  :
		if(game.created)
		{
			game.ingame = true;
			return -1;
		}
		break;
	case SPACE: break;
	}
	return MENU_MAIN;
}

/*
 * меню "ИГРА"
 */
static void menu_game_draw(const void * ctx)
{
	const menu_game_context_t * __ctx = ctx;

	menu_draw_conback();
	gr2D_setimage0(120,30-23*1       ,game.m_i_game    );
	gr2D_setimage0(120,30+23*0       ,game.m_i_g_new_p1);
	gr2D_setimage0(120,30+23*1       ,game.m_i_g_new_p2);
	gr2D_setimage0(120,30+23*2       ,game.m_i_g_load  );
	gr2D_setimage0( 97,30+23* __ctx->menu, game.m_i_cur_0);
};
int menu_game(menu_key_t menukey, void * ctx)
{
	static menu_selector_t menus[] =
	{
			MENU_GAME_NEW1P,
			MENU_GAME_NEW2P,
			MENU_GAME_LOAD,
			MENU_GAME_SAVE
	};
	menu_game_context_t * __ctx = ctx;
	switch(menukey)
	{
	case NOTHING: break;
	case UP     : if(__ctx->menu > 0  ) __ctx->menu--;break;
	case DOWN   : if(__ctx->menu < 4-1) __ctx->menu++;break;
	case LEFT   : break;
	case RIGHT  : break;
	case ENTER  : return menus[__ctx->menu];
	case LEAVE  : return MENU_MAIN;
	case SPACE: break;
	}
	return MENU_GAME;
}


int menu_game_new1P(menu_key_t menukey, void * ctx)
{
	int ret;
	if(game.created) return MENU_MAIN;
	game.gamemap = mapList;
	ret = map_load(game.gamemap->map);
	if(ret)
	{
		game_msg_error(map_error_get());
		return MENU_ABORT;
	}
	game.flags = 0;
	ret = game_create();
	return MENU_PRELEVEL;
}

int menu_game_new2P(menu_key_t menukey, void * ctx)
{
	int ret;
	if(game.created) return MENU_MAIN;
	game.gamemap = mapList;
	ret = map_load(game.gamemap->map);
	if(ret)
	{
		game_msg_error(ret);
		return MENU_ABORT;
	}
	game.flags = c_g_f_2PLAYERS;
	ret = game_create();
	return MENU_PRELEVEL;
}


/*
 * меню "ЗАГРУЗКА"
 */
static void menu_game_load_draw(const void * ctx)
{
	const menu_game_load_context_t * __ctx = ctx;
	int iline;
	int ic;
	menu_draw_conback();
	gr2D_setimage0(120,30+23*(-1), game.m_i_g_load );
	for(iline = 0; iline < 8; iline++)
	{
		gr2D_setimage0(97+23, 30+iline*15, game.m_i_lineL);
		for(ic = 0; ic<=16;ic++)
			gr2D_setimage0(97+23+4+8*ic, 30+iline*15, game.m_i_lineM);
		gr2D_setimage0(97+23+4+8*(ic+1), 30+iline*15, game.m_i_lineR);
		gr2D.color.current = 7;
		if(game.saveslist[iline].Hname[0] == 0xFF)
			gr2Don_settext(97+23+4, 33+iline*15, orient_horiz, "---===EMPTY===---");
		else
		{
			gr2Don_settext(97+23+4, 33+iline*15, orient_horiz, game.saveslist[iline].Hname);
			//отображение статуса сохраненной игры
			gr2D_setimage0(98+23+4+8*(ic+1), 29+iline*15, game.m_i_flagRUS);
			if(game.saveslist[iline].flags & c_g_f_2PLAYERS)
			{
				gr2D_setimage0(110+23+4+8*(ic+1), 29+iline*15, game.m_i_flagRUS);
			};
		};
	};
	gr2D_setimage0(97, 30 + 15 * __ctx->menu + 2, game.m_i_cur_1);
}
int menu_game_load(menu_key_t menukey, void * ctx)
{
	menu_game_load_context_t * __ctx = ctx;
	int ret;
	int menu_cur = 0;
	game_record_getsaves();
	switch(menukey)
	{
	case NOTHING: break;
	case UP     : if(__ctx->menu > 0  ) __ctx->menu--;break;
	case DOWN   : if(__ctx->menu < 8-1) __ctx->menu++;break;
	case LEFT   :
	case RIGHT  :
	case ENTER  :
		if(game.created) return MENU_MAIN;
		ret = game_record_load(&game.saveslist[menu_cur]);
		if(!ret)
			return MENU_PRELEVEL;
		if(2<=ret)
		{
			game_msg_error(ret+10);
			return MENU_ABORT;
		}
		return MENU_MAIN;
	case LEAVE  : return MENU_GAME;
	case SPACE: break;
	}
	return MENU_GAME_LOAD;
}
/*
 * меню "СОХРАНЕНИЕ"
 */
static void menu_game_save_draw(const void * ctx)
{
	const menu_game_save_context_t * __ctx = (menu_game_save_context_t *) ctx;


	int menu_cur = __ctx->menu_cur;
	bool f_input = __ctx->f_input;

	int ic;
	int iline;

	menu_draw_conback();
	gr2D_setimage0(120, 30+23*(-1), game.m_i_g_save );
	if(!f_input)
		gr2D_setimage0(97, 30+15*menu_cur+2, game.m_i_cur_1);
	for(iline = 0; iline < 8; iline++)
	{
		gr2D_setimage0(97+23          ,30+iline*15,game.m_i_lineL);
		for(ic = 0; ic<=16; ic++)
			gr2D_setimage0(97+23+4+8*ic    ,30+iline*15,game.m_i_lineM);
		gr2D_setimage0(97+23+4+8*(ic+1),30+iline*15,game.m_i_lineR);
		gr2D.color.current = 7;
		if(game.saveslist[iline].Hname[0]==0xFF)
			gr2Don_settext(97+23+4,33+iline*15, orient_horiz, "---===EMPTY===---");
		else
		{
			gr2Don_settext(97+23+4,33+iline*15, orient_horiz, game.saveslist[iline].Hname);
			//отображение статуса сохраненной игры
			gr2D_setimage0( 98+23+4+8*(ic+1),29+iline*15,game.m_i_flagRUS);
			if(game.saveslist[iline].flags & c_g_f_2PLAYERS)
				gr2D_setimage0(110+23+4+8*(ic+1),29+iline*15,game.m_i_flagRUS);
		}
	}
}

int menu_game_save(menu_key_t menukey, void * ctx)
{
	int menu_cur = 0;
	gamesave_t tmprec;
	bool f_input = false;
	char ch;
	long l;
	bool onInit = false;
	if(onInit)
		game_record_getsaves();
	if(f_input)
	{
		if(menukey>=0x80) ;
		else
		{
			if( menukey==KP0_ENTER_1 || menukey == KP0_ESCAPE_1) ;
			else
			{
				ch = (char) menukey;
				l = strlen(game.saveslist[menu_cur].Hname);
				if(ch == 0xEE)
				{
					if(0<l) game.saveslist[menu_cur].Hname[l-1] = 0;
				}
				else
				{
					if(ch<0x80 && l<=16) str_addch(game.saveslist[menu_cur].Hname, ch);
				}
			}
		}
	}
	if(!f_input)
	{

	}
	else
	{
		if(menukey == ENTER)
		{
			if(f_input)
			{
				game_record_save(&game.saveslist[menu_cur]);
				f_input = false;
			}
			else
			{
				tmprec = game.saveslist[menu_cur];
				game.saveslist[menu_cur].flags = game.flags;
				if(game.saveslist[menu_cur].Hname[0]== 0xFF) game.saveslist[menu_cur].Hname[0] = 0x00;
				do{ }while(!( (menukey==KP0_ENTER_0) ));
				f_input = true;
			};
		};
		if(menukey == LEAVE)
		{
			if(f_input)
			{
				game.saveslist[menu_cur] = tmprec;
				f_input = false;
			}
			else
			{
				if(game.created)
				{
					game.ingame = false;
					game_time_reset();
				}
			}
		}
	}
	return MENU_GAME_SAVE;
}
/*
 * меню "ВЫБОР"
 */
static void menu_custom_draw(const void * ctx)
{
	const menu_custom_context_t * __ctx = ctx;
	menu_draw_conback();
	gr2D_setimage0(120,30-23*1       , game.m_i_case    );
	gr2D_setimage0(120,30+23*0       , game.m_i_arrowL  );
	gr2D_setimage0(260,30+23*0       , game.m_i_arrowR  );
	gr2D_setimage0(120,30+23*1       , game.m_i_g_new_p1);
	gr2D_setimage0(120,30+23*2       , game.m_i_g_new_p2);
	gr2D_setimage0( 97,30+23 * __ctx->menu, game.m_i_cur_0);
	gr2D.color.current = 25;
	gr2Don_settext(133, 33+23*0, orient_horiz, game.custommap->map);
	gr2Don_settext(133, 41+23*0, orient_horiz, game.custommap->name);
}

int menu_custom(menu_key_t menukey, void * ctx)
{
	static menu_selector_t menus[] =
	{
			MENU_CUSTOM,
			MENU_CUSTOM_NEWP1,
			MENU_CUSTOM_NEWP2,
	};

	menu_custom_context_t * __ctx = ctx;
	switch(menukey)
	{
	case NOTHING: break;
	case UP     : if(__ctx->menu > 0  ) __ctx->menu--;break;
	case DOWN   : if(__ctx->menu < 3-1) __ctx->menu++;break;
	case LEFT   :
	case RIGHT  :
		if(__ctx->menu == 0)
		{
			if(menukey == LEFT)
				if(game.custommap->prev) game.custommap = game.custommap->prev;
			if(menukey == RIGHT)
				if(game.custommap->next) game.custommap = game.custommap->next;
		}
		break;
	case ENTER  : return menus[__ctx->menu];
	case LEAVE  : return MENU_MAIN;
	case SPACE: break;
	}

	return MENU_CUSTOM;
}

int menu_custom_new1P(menu_key_t menukey, void * ctx)
{
	int ret;
	if(game.created) return MENU_MAIN;
	ret = map_load(game.custommap->map);
	if(ret)
	{
		game_msg_error(ret);
		return MENU_ABORT;
	}
	game.flags = c_g_f_CASE;
	ret = game_create();
	return MENU_GAME;
}

int menu_custom_new2P(menu_key_t menukey, void * ctx)
{
	int ret;
	if(game.created) return MENU_MAIN;
	ret = map_load(game.custommap->map);
	if(ret)
	{
		game_msg_error(ret);
		return MENU_ABORT;
	}
	game.flags = c_g_f_2PLAYERS | c_g_f_CASE;
	ret = game_create();
	return MENU_GAME;
}


/*
 * меню "НАСТРОЙКИ"
 */
static void menu_options_draw(const void * ctx)
{
	const menu_options_context_t * __ctx = ctx;

	int cur_pl      = __ctx->cur_pl;
	bool wait_a_key = __ctx->wait_a_key;

	menu_draw_conback();
	gr2D_setimage0(120,30+23*(-1)     ,game.m_i_options);
	if(!wait_a_key)
		gr2D_setimage0(58+cur_pl*131,30+12+12*__ctx->menu+1,game.m_i_cur_1);
	gr2D.color.current = 25;
	gr2Don_settext( 58+131*0, 30+9*0, 0, "ИГРОК1");
	gr2Don_settext( 58+131*1, 30+9*0, 0, "ИГРОК2");
	gr2Don_settext( 9,32+12*1, 0, "Вперед");
	gr2Don_settext( 9,32+12*2, 0, "Назад");
	gr2Don_settext( 9,32+12*3, 0, "Влево");
	gr2Don_settext( 9,32+12*4, 0, "Вправо");
	gr2Don_settext( 9,32+12*5, 0, "Пульки");
	gr2Don_settext( 9,32+12*6, 0, "Ракета");
	gr2Don_settext( 9,32+12*7, 0, "Мина");
	gr2Don_settext(82+131*0,32+12*1, orient_horiz, "%d", game.controlP0[ 0]);
	gr2Don_settext(82+131*0,32+12*2, orient_horiz, "%d", game.controlP0[ 2]);
	gr2Don_settext(82+131*0,32+12*3, orient_horiz, "%d", game.controlP0[ 4]);
	gr2Don_settext(82+131*0,32+12*4, orient_horiz, "%d", game.controlP0[ 6]);
	gr2Don_settext(82+131*0,32+12*5, orient_horiz, "%d", game.controlP0[ 7]);
	gr2Don_settext(82+131*0,32+12*6, orient_horiz, "%d", game.controlP0[10]);
	gr2Don_settext(82+131*0,32+12*7, orient_horiz, "%d", game.controlP0[12]);
	gr2Don_settext(82+131*1,32+12*1, orient_horiz, "%d", game.controlP1[ 0]);
	gr2Don_settext(82+131*1,32+12*2, orient_horiz, "%d", game.controlP1[ 2]);
	gr2Don_settext(82+131*1,32+12*3, orient_horiz, "%d", game.controlP1[ 4]);
	gr2Don_settext(82+131*1,32+12*4, orient_horiz, "%d", game.controlP1[ 6]);
	gr2Don_settext(82+131*1,32+12*5, orient_horiz, "%d", game.controlP1[ 8]);
	gr2Don_settext(82+131*1,32+12*6, orient_horiz, "%d", game.controlP1[10]);
	gr2Don_settext(82+131*1,32+12*7, orient_horiz, "%d", game.controlP1[12]);
}

int menu_options(menu_key_t menukey, void * ctx)
{
	menu_options_context_t * __ctx = ctx;
	int cur_pl = 0;
	bool wait_a_key = false;
	bool onDone = false;

	if(wait_a_key)
	{
		//"забивание" клавиш
		do{ }while(!( (menukey != KP0_ENTER_0) ));
		if(cur_pl==0)
		{
			//настройка игрока 0
			game.controlP0[__ctx->menu * 2 + 0] = menukey;
			do{ }while(!( (menukey != game.controlP0[__ctx->menu*2+0])));
			game.controlP0[__ctx->menu * 2 + 1] = menukey;
		}
		else
		{
			//настройка игрока 1
			game.controlP1[__ctx->menu * 2 + 0] = menukey;
			do{ }while(!( (menukey != game.controlP1[__ctx->menu*2+0]) ));
			game.controlP1[__ctx->menu * 2 + 1] = menukey;
		};
		wait_a_key = false;
	};
	menukey = -1000;
	if(menukey == ENTER) wait_a_key = true;       //нажали ENTER
	if(!wait_a_key)
	{
		if(menukey == LEAVE) return MENU_MAIN;
		if(menukey == LEFT) cur_pl = 0;
		if(menukey == RIGHT) cur_pl = 1;
	};
	if(onDone)
		game_cfg_save();
	return MENU_OPTIONS;
};
/*
 * меню "О ИГРЕ"
 */
static void menu_about_draw(const void * ctx)
{
	gr2D_setimage0( 0, 0, game.m_i_interlv);
	gr2D.color.current = 13;
	gr2Don_settext( 8,10* 1, orient_horiz, c_strTITLE);
	gr2Don_settext(56,10* 2, orient_horiz, c_strCORP);
	gr2Don_settext( 8,10*15, orient_horiz, c_about[12]);
	gr2Don_settext( 8,10*16, orient_horiz, c_about[13]);
	gr2Don_settext( 8,10*17, orient_horiz, c_about[14]);
	gr2Don_settext( 8,10*18, orient_horiz, c_about[15]);

	gr2D.color.current = 7;
	gr2Don_settext( 8,10* 3, orient_horiz, c_about[0]);
	gr2Don_settext( 8,10* 5, orient_horiz, c_about[2]);
	gr2Don_settext( 8,10* 8, orient_horiz, c_about[5]);
	gr2Don_settext( 8,10*10, orient_horiz, c_about[7]);
	gr2Don_settext( 8,10*12, orient_horiz, c_about[9]);
	gr2D.color.current = 15;
	gr2Don_settext( 8,10* 4, orient_horiz, c_about[1]);
	gr2Don_settext( 8,10* 6, orient_horiz, c_about[3]);
	gr2Don_settext( 8,10* 7, orient_horiz, c_about[4]);
	gr2Don_settext( 8,10* 9, orient_horiz, c_about[6]);
	gr2Don_settext( 8,10*11, orient_horiz, c_about[8]);
	gr2Don_settext( 8,10*13, orient_horiz, c_about[10]);
}
int menu_about(menu_key_t menukey, void * ctx)
{
	if(menukey == LEAVE) return MENU_MAIN;
	return MENU_ABOUT;
};
/*
 * информация об уровне
 */
static void menu_prelevel_draw(const void * ctx)
{
	gr2D_setimage0(0,0,game.m_i_interlv);
	gr2D.color.current = 15;
	gr2Don_settext(160-06*4 ,8*5, orient_horiz, "КАРТА:");
	gr2D.color.current = 15;
	gr2Don_settext(160-16*4, 8*7 , orient_horiz, map.name);
	gr2Don_settext(160-07*4, 8*10, orient_horiz, "ЗАДАЧА:");
	gr2Don_settext(108     , 191 , orient_horiz, "НАЖМИ ПРОБЕЛ");
	int i = 0;
	while(map.brief[i])
	{
		gr2Don_setchar(160 - 16 * 4 + ((i % 16) * 8), 8 * 12 + ((i / 16) * 10), map.brief[i]);
		i++;
	};
}
int menu_prelevel(menu_key_t menukey, void * ctx)
{
	//menu_prelevel_context_t * __ctx = ctx;
	switch(menukey)
	{
	case NOTHING: break;
	case UP     : break;
	case DOWN   : break;
	case LEFT   : break;
	case RIGHT  : break;
	case ENTER  : break;
	case LEAVE  : return MENU_MAIN;
	case SPACE  :
		game.ingame = true;
		return MENU_MAIN;
	}
	return MENU_PRELEVEL;
};
/*
 * заставка между уровнями
 */
static void menu_interlevel_draw(const void * ctx)
{
	gr2D_setimage0(0, 0, game.m_i_interlv);
	gr2D.color.current = 15;
	gr2Don_settext(108,191,0,"НАЖМИ ПРОБЕЛ");
	if(!game.P1)
	{
		//один игрок
		gr2D_setimage1(26,92,
				game.P0->Ibase,
				0,0,c_p_MDL_box,c_p_MDL_box
		);
		gr2D.color.current = 15;
		gr2Don_settext(48+8*00,84      , orient_horiz, "ОЧКИ      ФРАГИ      ВСЕГО ФРАГОВ");
		gr2Don_settext(48+8*00,84+4+8*1, orient_horiz, "%ld", game.P0->charact.scores);
		gr2Don_settext(48+8*10,84+4+8*1, orient_horiz, "%ld", game.P0->charact.frags );
		gr2Don_settext(48+8*21,84+4+8*1, orient_horiz, "%ld", game.P0->charact.fragstotal);
	}
	else
	{
		//два игрока
		gr2D_setimage1(26,84+18*0-1,
				game.P0->Ibase,
				0,0,c_p_MDL_box,c_p_MDL_box
		);
		gr2D_setimage1(26,84+18*1+1,
				game.P1->Ibase,
				0,0,c_p_MDL_box,c_p_MDL_box
		);
		gr2D.color.current = 15;
		gr2Don_settext(48+8*00,76      -1, orient_horiz, "ОЧКИ      ФРАГИ      ВСЕГО ФРАГОВ");
		gr2Don_settext(48+8*00,76+4+8*1-1, orient_horiz, "%ld", game.P0->charact.scores     );
		gr2Don_settext(48+8*10,76+4+8*1-1, orient_horiz, "%ld", game.P0->charact.frags      );
		gr2Don_settext(48+8*21,76+4+8*1-1, orient_horiz, "%ld", game.P0->charact.fragstotal );
		gr2Don_settext(48+8*00,76+4+8*3+1, orient_horiz, "%ld", game.P1->charact.scores     );
		gr2Don_settext(48+8*10,76+4+8*3+1, orient_horiz, "%ld", game.P1->charact.frags      );
		gr2Don_settext(48+8*21,76+4+8*3+1, orient_horiz, "%ld", game.P1->charact.fragstotal );
	}
}

int menu_interlevel(menu_key_t menukey, void * ctx)
{
	switch(menukey)
	{
	case NOTHING: break;
	case UP     : break;
	case DOWN   : break;
	case LEFT   : break;
	case RIGHT  : break;
	case ENTER  : return MENU_MAIN;
	case LEAVE  : return MENU_MAIN;
	case SPACE  : return MENU_MAIN;
	}
	return MENU_INTERLEVEL;
}

int menu_abort(menu_key_t menukey, void * ctx)
{
	game_abort();
	return MENU_MAIN;
}





menu_t menus[MENU_NUM] =
{
		{ 6, &menu_main_context    , menu_main        , menu_main_draw }, /* MENU_MAIN */
		{ 3, &menu_game_context    , menu_game        , menu_game_draw }, /* MENU_GAME */
		{ 0, NULL                  , menu_game_new1P  , NULL }, /* MENU_GAME_NEW1P */
		{ 0, NULL                  , menu_game_new2P  , NULL }, /* MENU_GAME_NEW2P */
		{ 8, &menu_load_context    , menu_game_load   , menu_game_load_draw   }, /* MENU_GAME_LOAD */
		{ 8, &menu_save_context    , menu_game_save   , menu_game_save_draw   }, /* MENU_GAME_SAVE */
		{ 3, &menu_custom_context  , menu_custom      , menu_custom_draw }, /* MENU_CUSTOM */
		{ 0, NULL                  , menu_custom_new1P, NULL }, /* MENU_CUSTOM_NEWP1 */
		{ 0, NULL                  , menu_custom_new2P, NULL }, /* MENU_CUSTOM_NEWP2 */
		{ 7, &menu_options_context , menu_options     , menu_options_draw}, /* MENU_OPTIONS */
		{ 0, &menu_about_context   , menu_about       , menu_about_draw  }, /* MENU_ABOUT */
		{ 0, &menu_interlevel_context, menu_interlevel, menu_interlevel_draw }, /* MENU_INTERLEVEL */
		{ 0, &menu_prelevel_context, menu_prelevel    , menu_prelevel_draw }, /* MENU_PRELEVEL */
		{ 0, NULL                  , menu_abort       , NULL }, /* MENU_ABORT */
		{ 0, NULL                  , NULL             , NULL }  /* MENU_QUIT */
};


int menu_handle(int imenu)
{
		if(0 <= imenu && imenu < MENU_NUM)
		{
			menu_t * menu = &menus[imenu];

			menu_key_t menukey = menu_key_get(menu->maxmenu);

			if(menu->handle) imenu = menu->handle(menukey, menu->context);
		}
		return imenu;
}

void menu_draw(int imenu)
{
	if(0 <= imenu && imenu < MENU_NUM)
	{
		menu_t * menu = &menus[imenu];
		if(menu->draw) menu->draw(menu->context);
	}
}
