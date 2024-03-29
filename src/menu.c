/*
 *        Ubivat Tank
 *        обработка меню
 * by Master San
 */
#include "menu.h"
#include "img.h"
#include "_gr2D.h"
#include "fonts.h"
#include "types.h"
#include "video.h"
#include "sound.h"

/* MENU_MAIN */
menu_main_ctx_t menu_main_ctx = {};

/* MENU_GAME */
menu_game_ctx_t menu_game_ctx = {};

/* MENU_GAME_NEW1P */

/* MENU_GAME_NEW2P */

/* MENU_GAME_LOAD */
menu_game_load_ctx_t menu_load_ctx = {};

/* MENU_GAME_SAVE */
menu_game_save_ctx_t menu_save_ctx = {};

/* MENU_CUSTOM */
menu_custom_ctx_t menu_custom_ctx = {};

/* MENU_CUSTOM_NEWP1 */

/* MENU_CUSTOM_NEWP2 */

/* MENU_OPTIONS */
menu_options_ctx_t menu_options_ctx = {};
/* MENU_ABOUT */
menu_about_ctx_t menu_about_ctx = {};
/* MENU_INTERLEVEL */
menu_interlevel_ctx_t menu_interlevel_ctx = {};
/* MENU_PRELEVEL */
menu_prelevel_ctx_t menu_prelevel_ctx = {};

/* MENU_ABORT */

/* MENU_QUIT */



static void _menu_dec(const int menu_amount, int * menu)
{
	sound_play_start(SOUND_MENU_MOVE, 1);
	if((*menu) <= 0)
	{
		*menu = menu_amount - 1;
		return;
	}
	(*menu)--;
}

static void _menu_inc(const int menu_amount, int * menu)
{
	sound_play_start(SOUND_MENU_MOVE, 1);
	if((*menu) >= menu_amount - 1)
	{
		*menu = 0;
		return;
	}
	(*menu)++;
}

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
			buffer[buffer_end] = event->key.keysym.scancode;// use .sym istead
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



menu_key_t menu_key_get(void)
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

static void menu_draw_conback(void)
{
	gr2D_setimage0(0, 0, game.m_i_conback);
}

/*
 * главное меню
 */
int menu_main(void * ctx)
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
	menu_main_ctx_t * __ctx  = ctx;
	switch(menu_key_get())
	{
	case NOTHING: break;
	case UP     : _menu_dec(6, &__ctx->menu);break;
	case DOWN   : _menu_inc(6, &__ctx->menu);break;
	case LEFT   : break;
	case RIGHT  : break;
	case ENTER  :
		sound_play_start(SOUND_MENU_ENTER, 1);
		if(!game.created && __ctx->menu == 4)return MENU_MAIN;
		return menus[__ctx->menu];
	case LEAVE  :
		sound_play_start(SOUND_MENU_ENTER, 1);
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

static void menu_main_draw(const void * ctx)
{
	const menu_main_ctx_t *__ctx = ctx;
	int menu = __ctx->menu;
	menu_draw_conback();
	gr2D_setimage0(277  ,159    ,game.m_i_logo);
	font_color_set3i(COLOR_1);
	video_printf(1     ,183          , orient_horiz, c_strTITLE);
	video_printf(1     ,191          , orient_horiz, c_strCORP);

	static image_index_t list[] =
	{
			M_GAME,
			M_CASE,
			M_OPTIONS,
			M_ABOUT,
			M_ABORT,
			M_QUIT
	};
	for(int i = 0; i < 6; i++)
	{
		if(i != 4 || game.created)
			gr2D_setimage0(120, 30 + 23 * i, image_get(list[i]));
	}
	gr2D_setimage0( 97,30+23 * menu, game.m_i_cur_0);
}
/*
 * меню "ИГРА"
 */
int menu_game(void * ctx)
{
	static menu_selector_t menus[] =
	{
			MENU_GAME_NEW1P,
			MENU_GAME_NEW2P,
			MENU_GAME_LOAD,
			MENU_GAME_SAVE
	};
	menu_game_ctx_t * __ctx = ctx;
	switch(menu_key_get())
	{
	case NOTHING: break;
	case UP     : _menu_dec(3, &__ctx->menu);break;
	case DOWN   : _menu_inc(3, &__ctx->menu);break;
	case LEFT   : break;
	case RIGHT  : break;
	case ENTER  :
		sound_play_start(SOUND_MENU_ENTER, 1);
		return menus[__ctx->menu];
	case LEAVE  :
		sound_play_start(SOUND_MENU_ENTER, 1);
		return MENU_MAIN;
	case SPACE: break;
	}
	return MENU_GAME;
}


static void menu_game_draw(const void * ctx)
{
	const menu_game_ctx_t * __ctx = ctx;

	menu_draw_conback();
	gr2D_setimage0(120,30-23*1       ,game.m_i_game    );
	gr2D_setimage0(120,30+23*0       ,game.m_i_g_new_p1);
	gr2D_setimage0(120,30+23*1       ,game.m_i_g_new_p2);
	gr2D_setimage0(120,30+23*2       ,game.m_i_g_load  );
	gr2D_setimage0( 97,30+23* __ctx->menu, game.m_i_cur_0);
};
int menu_game_new1P(void * ctx)
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

int menu_game_new2P(void * ctx)
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
int menu_game_load(void * ctx)
{
	menu_game_load_ctx_t * __ctx = ctx;
	int ret;
	menu_key_t menukey = menu_key_get();
	switch(__ctx->state)
	{
	case MENU_GAME_LOAD_INIT:
		game_record_getsaves();
		__ctx->state = MENU_GAME_LOAD_SELECT;
		break;
	case MENU_GAME_LOAD_SELECT:
		switch(menukey)
		{
		case NOTHING: break;
		case UP     : _menu_dec(8, &__ctx->menu);break;
		case DOWN   : _menu_inc(8, &__ctx->menu);break;
		case LEFT   : break;
		case RIGHT  : break;
		case ENTER  :
			sound_play_start(SOUND_MENU_ENTER, 1);
			__ctx->state = MENU_GAME_LOAD_INIT;
			if(game.created) return MENU_MAIN;
			if(!game.saveslist[__ctx->menu].exist) break;
			ret = game_record_load(__ctx->menu);
			if(!ret)
				return MENU_PRELEVEL;
			if(2<=ret)
			{
				game_msg_error(ret+10);
				return MENU_ABORT;
			}
			return MENU_MAIN;
		case LEAVE  :
			sound_play_start(SOUND_MENU_ENTER, 1);
			return MENU_GAME;
		case SPACE  : break;
		}
		break;
	}
	return MENU_GAME_LOAD;

}
static void menu_game_load_draw(const void * ctx)
{
	const menu_game_load_ctx_t * __ctx = ctx;
	int irow;
	int icol;
	menu_draw_conback();
	gr2D_setimage0(120,30+23*(-1), game.m_i_g_load );
	for(irow = 0; irow < 8; irow++)
	{
		gr2D_setimage0(97+23, 30+irow*15, game.m_i_lineL);
		for(icol = 0; icol<16; icol++)
			gr2D_setimage0(97+23+4+8*icol, 30+irow*15, game.m_i_lineM);
		gr2D_setimage0(97+23+4+8*icol, 30+irow*15, game.m_i_lineR);
		font_color_set3i(COLOR_7);
		if(!game.saveslist[irow].exist)
		{
			video_printf(97+23+4, 33+irow*15, orient_horiz, "---===EMPTY===---");
			continue;
		}
		video_printf(97+23+4, 33+irow*15, orient_horiz, game.saveslist[irow].name);
		//отображение статуса сохраненной игры
		gr2D_setimage0(98+23+4+8*(icol+1), 29+irow*15, game.m_i_flagRUS);
		if(game.saveslist[irow].flags & c_g_f_2PLAYERS)
		{
			gr2D_setimage0(110+23+4+8*(icol+1), 29+irow*15, game.m_i_flagRUS);
		};
	};
	gr2D_setimage0(97, 30 + 15 * __ctx->menu + 2, game.m_i_cur_1);
}
int menu_game_save(void * ctx)
{
	menu_game_save_ctx_t * __ctx = ctx;
	size_t l;
	char ch;
	switch(__ctx->state)
	{
	case MENU_GAME_SAVE_INIT:
		game_record_getsaves();
		__ctx->state = MENU_GAME_SAVE_SELECT;
		break;
	case MENU_GAME_SAVE_SELECT:
		switch(menu_key_get())
		{
		case NOTHING: break;
		case UP     : _menu_dec(8, &__ctx->menu);break;
		case DOWN   : _menu_inc(8, &__ctx->menu);break;
		case LEFT   :
		case RIGHT  :
		case ENTER  :
			sound_play_start(SOUND_MENU_ENTER, 1);
			__ctx->rec = game.saveslist[__ctx->menu];
			game.saveslist[__ctx->menu].flags = game.flags;
			game.saveslist[__ctx->menu].exist = true;
			__ctx->state = MENU_GAME_SAVE_INPUT;
			break;
		case LEAVE  :
			sound_play_start(SOUND_MENU_ENTER, 1);
			__ctx->state = MENU_GAME_SAVE_INIT;
			return MENU_PRELEVEL;
		case SPACE  :break;
		}
		break;
	case MENU_GAME_SAVE_INPUT:
		if(buffer_isEmpty())break;
		buffer_key_t scancode = buffer_dequeue_nowait();
		switch(scancode)
		{
		case SDL_SCANCODE_UNKNOWN: break;
		case SDL_SCANCODE_ESCAPE:
			game.saveslist[__ctx->menu] = __ctx->rec;
			__ctx->state = MENU_GAME_SAVE_SELECT;
			break;
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_RETURN2:
			__ctx->state = MENU_GAME_SAVE_SAVE;
			break;
		case SDL_SCANCODE_BACKSPACE:
			l = strlen(game.saveslist[__ctx->menu].name);
			if(0 < l) game.saveslist[__ctx->menu].name[l-1] = 0;
			break;
		default :
			ch = SDL_GetKeyFromScancode(scancode);
			if(ch == 0)break;
			l = strlen(game.saveslist[__ctx->menu].name);
			if(ch < 0x80 && l <= 16) str_addch(game.saveslist[__ctx->menu].name, ch);
		}
		break;
	case MENU_GAME_SAVE_SAVE:
		game_record_save(__ctx->menu);
		__ctx->state = MENU_GAME_SAVE_SELECT;
		break;
	}
	return MENU_GAME_SAVE;
}
/*
 * меню "СОХРАНЕНИЕ"
 */
static void menu_game_save_draw(const void * ctx)
{
	const menu_game_save_ctx_t * __ctx = ctx;

	int menu = __ctx->menu;

	int irow;
	int icol;

	menu_draw_conback();
	gr2D_setimage0(120, 30+23*(-1), game.m_i_g_save );
	if(__ctx->state == MENU_GAME_SAVE_SELECT)
		gr2D_setimage0(97, 30+15*menu+2, game.m_i_cur_1);
	for(irow = 0; irow < 8; irow++)
	{
		gr2D_setimage0(97+23, 30+irow*15, game.m_i_lineL);
		for(icol = 0; icol < 16; icol++)
			gr2D_setimage0(97+23+4+8*icol, 30+irow*15, game.m_i_lineM);
		gr2D_setimage0(97+23+4+8*icol, 30+irow*15, game.m_i_lineR);
		font_color_set3i(COLOR_7);
		if(!game.saveslist[irow].exist)
		{
			video_printf(97+23+4,33+irow*15, orient_horiz, "---===EMPTY===---");
			continue;
		}
		video_printf(97+23+4,33+irow*15, orient_horiz, game.saveslist[irow].name);
		//отображение статуса сохраненной игры
		gr2D_setimage0(98+23+4+8*(icol+1), 29+irow*15, game.m_i_flagRUS);
		if(game.saveslist[irow].flags & c_g_f_2PLAYERS)
			gr2D_setimage0(110+23+4+8*(icol+1), 29+irow*15, game.m_i_flagRUS);
	}
}

/*
 * меню "ВЫБОР"
 */
int menu_custom(void * ctx)
{
	static menu_selector_t menus[] =
	{
			MENU_CUSTOM,
			MENU_CUSTOM_NEWP1,
			MENU_CUSTOM_NEWP2,
	};

	menu_custom_ctx_t * __ctx = ctx;
	menu_key_t menukey = menu_key_get();
	switch(menukey)
	{
	case NOTHING: break;
	case UP     : _menu_dec(3, &__ctx->menu);break;
	case DOWN   : _menu_inc(3, &__ctx->menu);break;
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
	case ENTER  :
		sound_play_start(SOUND_MENU_ENTER, 1);
		return menus[__ctx->menu];
	case LEAVE  :
		sound_play_start(SOUND_MENU_ENTER, 1);
		return MENU_MAIN;
	case SPACE: break;
	}

	return MENU_CUSTOM;
}

static void menu_custom_draw(const void * ctx)
{
	const menu_custom_ctx_t * __ctx = ctx;
	menu_draw_conback();
	gr2D_setimage0(120,30-23*1       , game.m_i_case    );
	gr2D_setimage0(120,30+23*0       , game.m_i_arrowL  );
	gr2D_setimage0(260,30+23*0       , game.m_i_arrowR  );
	gr2D_setimage0(120,30+23*1       , game.m_i_g_new_p1);
	gr2D_setimage0(120,30+23*2       , game.m_i_g_new_p2);
	gr2D_setimage0( 97,30+23 * __ctx->menu, game.m_i_cur_0);
	font_color_set3i(COLOR_25);
	video_printf(133, 33+23*0, orient_horiz, game.custommap->map);
	video_printf(133, 41+23*0, orient_horiz, game.custommap->name);
}

int menu_custom_new1P(void * ctx)
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
	return MENU_PRELEVEL;
}

int menu_custom_new2P(void * ctx)
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
	return MENU_PRELEVEL;
}

int menu_options(void * ctx)
{
	menu_options_ctx_t * __ctx = ctx;
	static actions_t menuactions[] =
	{
			ACTION_PLAYER_MOVE_UP,
			ACTION_PLAYER_MOVE_DOWN,
			ACTION_PLAYER_MOVE_LEFT,
			ACTION_PLAYER_MOVE_RIGHT,
			ACTION_PLAYER_ATTACK_WEAPON1,
			ACTION_PLAYER_ATTACK_WEAPON2,
			ACTION_PLAYER_ATTACK_WEAPON3,
			ACTION_PLAYER2_MOVE_UP,
			ACTION_PLAYER2_MOVE_DOWN,
			ACTION_PLAYER2_MOVE_LEFT,
			ACTION_PLAYER2_MOVE_RIGHT,
			ACTION_PLAYER2_ATTACK_WEAPON1,
			ACTION_PLAYER2_ATTACK_WEAPON2,
			ACTION_PLAYER2_ATTACK_WEAPON3
	};
#define MENU_ROWS 7
	switch(__ctx->state)
	{
	case MENU_OPTIONS_SELECT:
		switch(menu_key_get())
		{
		case NOTHING: break;
		case UP     : _menu_dec(MENU_ROWS, &__ctx->menu);break;
		case DOWN   : _menu_inc(MENU_ROWS, &__ctx->menu);break;
		case LEFT   : _menu_dec(1, &__ctx->column);break;
		case RIGHT  : _menu_inc(1, &__ctx->column);break;
		case ENTER  :
			sound_play_start(SOUND_MENU_ENTER, 1);
			__ctx->state = MENU_OPTIONS_WAIT_KEY;break;
		case LEAVE  :
			sound_play_start(SOUND_MENU_ENTER, 1);
			game_cfg_save();
			game_rebind_keys_all();
			__ctx->state = MENU_OPTIONS_SELECT;
			return MENU_MAIN;
		case SPACE  : break;
		}
		break;
	case MENU_OPTIONS_WAIT_KEY:


		if(buffer_isEmpty())break;
		buffer_key_t scancode = buffer_dequeue_nowait();
		switch(scancode)
		{
		case SDL_SCANCODE_UNKNOWN: break;
		case SDL_SCANCODE_ESCAPE:
			__ctx->state = MENU_OPTIONS_SELECT;
			break;
		default:
			game.controls[menuactions[__ctx->menu + __ctx->column * MENU_ROWS ]] = scancode;
			__ctx->state = MENU_OPTIONS_SELECT;
		}
	}
	return MENU_OPTIONS;
};

/*
 * меню "НАСТРОЙКИ"
 */
static void menu_options_draw(const void * ctx)
{
	const menu_options_ctx_t * __ctx = ctx;

	menu_draw_conback();
	gr2D_setimage0(120,30+23*(-1)     ,game.m_i_options);
	if(__ctx->state == MENU_OPTIONS_SELECT)
		gr2D_setimage0(58 + __ctx->column * 131, 30 + 12 + 12 * __ctx->menu + 1, game.m_i_cur_1);
	font_color_set3i(COLOR_25);
	video_printf( 58+131*0, 30+9*0, 0, "ИГРОК1");
	video_printf( 58+131*1, 30+9*0, 0, "ИГРОК2");
	video_printf( 9,32+12*1, 0, "Вперед");
	video_printf( 9,32+12*2, 0, "Назад");
	video_printf( 9,32+12*3, 0, "Влево");
	video_printf( 9,32+12*4, 0, "Вправо");
	video_printf( 9,32+12*5, 0, "Пульки");
	video_printf( 9,32+12*6, 0, "Ракета");
	video_printf( 9,32+12*7, 0, "Мина");
	video_printf(82+131*0,32+12*1, orient_horiz, "%d", game.controls[ACTION_PLAYER_MOVE_UP]);
	video_printf(82+131*0,32+12*2, orient_horiz, "%d", game.controls[ACTION_PLAYER_MOVE_DOWN]);
	video_printf(82+131*0,32+12*3, orient_horiz, "%d", game.controls[ACTION_PLAYER_MOVE_LEFT]);
	video_printf(82+131*0,32+12*4, orient_horiz, "%d", game.controls[ACTION_PLAYER_MOVE_RIGHT]);
	video_printf(82+131*0,32+12*5, orient_horiz, "%d", game.controls[ACTION_PLAYER_ATTACK_WEAPON1]);
	video_printf(82+131*0,32+12*6, orient_horiz, "%d", game.controls[ACTION_PLAYER_ATTACK_WEAPON2]);
	video_printf(82+131*0,32+12*7, orient_horiz, "%d", game.controls[ACTION_PLAYER_ATTACK_WEAPON3]);
	video_printf(82+131*1,32+12*1, orient_horiz, "%d", game.controls[ACTION_PLAYER2_MOVE_UP]);
	video_printf(82+131*1,32+12*2, orient_horiz, "%d", game.controls[ACTION_PLAYER2_MOVE_DOWN]);
	video_printf(82+131*1,32+12*3, orient_horiz, "%d", game.controls[ACTION_PLAYER2_MOVE_LEFT]);
	video_printf(82+131*1,32+12*4, orient_horiz, "%d", game.controls[ACTION_PLAYER2_MOVE_RIGHT]);
	video_printf(82+131*1,32+12*5, orient_horiz, "%d", game.controls[ACTION_PLAYER2_ATTACK_WEAPON1]);
	video_printf(82+131*1,32+12*6, orient_horiz, "%d", game.controls[ACTION_PLAYER2_ATTACK_WEAPON2]);
	video_printf(82+131*1,32+12*7, orient_horiz, "%d", game.controls[ACTION_PLAYER2_ATTACK_WEAPON3]);
}



/*
 * меню "О ИГРЕ"
 */
int menu_about(void * ctx)
{
	if(menu_key_get() == LEAVE)
	{
		sound_play_start(SOUND_MENU_ENTER, 1);
		return MENU_MAIN;
	}
	return MENU_ABOUT;
};

static void menu_about_draw(const void * ctx)
{
	gr2D_setimage0( 0, 0, game.m_i_interlv);
	int i = 0;
	coloredtext_t * text;

	font_color_set3i(COLOR_13);
	video_printf( 8,10 * 1, orient_horiz, c_strTITLE);
	video_printf(56,10 * 2, orient_horiz, c_strCORP);

	for(;;)
	{
		text = &c_about[i];
		if(!text->text)break;
		font_color_sets(&text->color);
		video_printf( 8, 10 * (i + 3), orient_horiz, text->text);
		i++;
	}
}

/*
 * информация об уровне
 */
int menu_prelevel(void * ctx)
{

	menu_prelevel_ctx_t * __ctx = ctx;

	if(!__ctx->sound_playId)
		__ctx->sound_playId = sound_play_start(SOUND_MUSIC1, -1);


	switch(menu_key_get())
	{
	case NOTHING: break;
	case UP     : break;
	case DOWN   : break;
	case LEFT   : break;
	case RIGHT  : break;
	case ENTER  :
	case LEAVE  :
	case SPACE  :
		sound_play_stop(__ctx->sound_playId);
		sound_play_start(SOUND_MENU_ENTER, 1);
		game.ingame = true;
		return MENU_MAIN;
	}
	return MENU_PRELEVEL;
};
static void menu_prelevel_draw(const void * ctx)
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
/*
 * заставка между уровнями
 */
int menu_interlevel(void * ctx)
{
	switch(menu_key_get())
	{
	case NOTHING: break;
	case UP     : break;
	case DOWN   : break;
	case LEFT   : break;
	case RIGHT  : break;
	case ENTER  : return
		sound_play_start(SOUND_MENU_ENTER, 1);
		return MENU_GAME_SAVE;
	case LEAVE  :
		sound_play_start(SOUND_MENU_ENTER, 1);
		return MENU_GAME_SAVE;
	case SPACE  : return MENU_GAME_SAVE;
	}
	return MENU_INTERLEVEL;
}

static void menu_interlevel_draw(const void * ctx)
{
	gr2D_setimage0(0, 0, game.m_i_interlv);
	font_color_set3i(COLOR_15);
	video_printf(108,191,0,"НАЖМИ ПРОБЕЛ");
	if(!game.P1)
	{
		//один игрок
		gr2D_setimage1(26, 92,
				game.P0->Ibase,
				0,0,c_p_MDL_box,c_p_MDL_box
		);
		font_color_set3i(COLOR_15);
		video_printf(48+8*00,84      , orient_horiz, "ОЧКИ      ФРАГИ      ВСЕГО ФРАГОВ");
		video_printf(48+8*00,84+4+8*1, orient_horiz, "%ld", game.P0->charact.scores);
		video_printf(48+8*10,84+4+8*1, orient_horiz, "%ld", game.P0->charact.frags );
		video_printf(48+8*21,84+4+8*1, orient_horiz, "%ld", game.P0->charact.fragstotal);
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
		font_color_set3i(COLOR_15);
		video_printf(48+8*00,76      -1, orient_horiz, "ОЧКИ      ФРАГИ      ВСЕГО ФРАГОВ");
		video_printf(48+8*00,76+4+8*1-1, orient_horiz, "%ld", game.P0->charact.scores     );
		video_printf(48+8*10,76+4+8*1-1, orient_horiz, "%ld", game.P0->charact.frags      );
		video_printf(48+8*21,76+4+8*1-1, orient_horiz, "%ld", game.P0->charact.fragstotal );
		video_printf(48+8*00,76+4+8*3+1, orient_horiz, "%ld", game.P1->charact.scores     );
		video_printf(48+8*10,76+4+8*3+1, orient_horiz, "%ld", game.P1->charact.frags      );
		video_printf(48+8*21,76+4+8*3+1, orient_horiz, "%ld", game.P1->charact.fragstotal );
	}
}

int menu_abort(void * ctx)
{
	game_abort();
	return MENU_MAIN;
}





menu_t menus[MENU_NUM] =
{
		{ &menu_main_ctx      , menu_main        , menu_main_draw }, /* MENU_MAIN */
		{ &menu_game_ctx      , menu_game        , menu_game_draw }, /* MENU_GAME */
		{ NULL                , menu_game_new1P  , NULL }, /* MENU_GAME_NEW1P */
		{ NULL                , menu_game_new2P  , NULL }, /* MENU_GAME_NEW2P */
		{ &menu_load_ctx      , menu_game_load   , menu_game_load_draw   }, /* MENU_GAME_LOAD */
		{ &menu_save_ctx      , menu_game_save   , menu_game_save_draw   }, /* MENU_GAME_SAVE */
		{ &menu_custom_ctx    , menu_custom      , menu_custom_draw }, /* MENU_CUSTOM */
		{ NULL                , menu_custom_new1P, NULL }, /* MENU_CUSTOM_NEWP1 */
		{ NULL                , menu_custom_new2P, NULL }, /* MENU_CUSTOM_NEWP2 */
		{ &menu_options_ctx   , menu_options     , menu_options_draw}, /* MENU_OPTIONS */
		{ &menu_about_ctx     , menu_about       , menu_about_draw  }, /* MENU_ABOUT */
		{ &menu_interlevel_ctx, menu_interlevel  , menu_interlevel_draw }, /* MENU_INTERLEVEL */
		{ &menu_prelevel_ctx  , menu_prelevel    , menu_prelevel_draw }, /* MENU_PRELEVEL */
		{ NULL                , menu_abort       , NULL }, /* MENU_ABORT */
		{ NULL                , NULL             , NULL }  /* MENU_QUIT */
};


int menu_handle(int imenu)
{
		if(0 <= imenu && imenu < MENU_NUM)
		{
			menu_t * menu = &menus[imenu];
			if(menu->handle) imenu = menu->handle(menu->context);
		}
		return imenu;
}

void menu_draw(int imenu)
{
	video_viewport_set(
		0.0f,
		0.0f,
		VIDEO_SCREEN_W,
		VIDEO_SCREEN_H
	);
	if(0 <= imenu && imenu < MENU_NUM)
	{
		menu_t * menu = &menus[imenu];
		if(menu->draw) menu->draw(menu->context);
	}
}
