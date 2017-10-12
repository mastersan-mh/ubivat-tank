/*
 *        Ubivat Tank
 *        обработка меню
 * by Master San
 */

#include "menu.h"
#include "g_conf.h"
#include "img.h"
#include "_gr2D.h"
#include "fonts.h"
#include "types.h"
#include "video.h"
#include "sound.h"
#include "entity.h"
#include "client.h"
#include "cl_input.h"
#include "cl_game.h"

#define menu_draw_logo() \
	gr2D_setimage0(277, 159, image_get(IMG_MENU_LOGO))

#define menu_draw_header(image) \
		gr2D_setimage00(120, 30 - 23, image_get(image), 0, 22)

#define menu_draw_entry(row, image) \
		gr2D_setimage00(120, 30 + 23 * (row), image_get(image), 0, 22)

#define menu_draw_cursor(row) \
		gr2D_setimage00(120 - 23, 30 + 23 * (row), image_get(IMG_MENU_CUR_0), 0, 22);

#define menu_draw_string_indicator_small(row, nchars) \
		do { \
			gr2D_setimage0(120, 30 + (row) * 15, image_get(IMG_MENU_LINEL)); \
			int icol; \
			for(icol = 0; icol < (nchars); icol++) \
			{ \
				gr2D_setimage0(120 + 4 + icol * 8, 30 + (row) * 15, image_get(IMG_MENU_LINEM)); \
			} \
			gr2D_setimage0(120 + 4 + icol * 8, 30 + (row) * 15, image_get(IMG_MENU_LINER)); \
		} while (0)

typedef enum
{
	NOTHING,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	ENTER,
	LEAVE,
	SPACE
} menu_key_t;

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

/* MENU_CUSTOM_CONNECT */

/* MENU_OPTIONS */
menu_options_ctx_t menu_options_ctx = {};
/* MENU_ABOUT */
menu_about_ctx_t menu_about_ctx = {};
/* MENU_ABORT */

/* MENU_QUIT */

static void _menu_dec(const int menu_amount, int * menu)
{
	sound_play_start(NULL, 0, SOUND_MENU_MOVE, 1);
	if((*menu) <= 0)
	{
		*menu = menu_amount - 1;
		return;
	}
	(*menu)--;
}

static void _menu_inc(const int menu_amount, int * menu)
{
	sound_play_start(NULL, 0, SOUND_MENU_MOVE, 1);
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

buffer_key_t buffer_dequeue_nowait(void)
{
	if(buffer_start == buffer_end) return SDL_SCANCODE_UNKNOWN;
	buffer_key_t key = buffer[buffer_start];
	buffer_start++;
	if(buffer_start >= KEYBUFFER_SIZE) buffer_start = 0;
	return key;
}

static void menu_send_event(SDL_Event * event)
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
	case SDL_QUIT:
		break;
	default:
		break;
	}
}

void menu_events_pump(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		menu_send_event(&event);
	}
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
	gr2D_setimage0(0, 0, image_get(IMG_MENU_CONBACK));
}

/*
 * главное меню
 */
static int menu_main(void * ctx)
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
		sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
		if(cl_state.state == GAMESTATE_NOGAME && __ctx->menu == 4)
			return MENU_MAIN;
		return menus[__ctx->menu];
	case LEAVE  :
		sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
		if(cl_state.state != GAMESTATE_NOGAME)
			cl_state.show_menu = false;
		break;
	case SPACE: break;
	}
	return MENU_MAIN;
}

static void menu_main_draw(const void * ctx_)
{
	const menu_main_ctx_t *ctx = ctx_;
	int imenu = ctx->menu;
	menu_draw_conback();
	menu_draw_logo();
	font_color_set3i(COLOR_1);
	video_printf(1     ,183          , c_strTITLE);
	video_printf(1     ,191          , c_strCORP);

	static image_index_t list[] =
	{
			IMG_MENU_GAME,
			IMG_MENU_CASE,
			IMG_MENU_OPTIONS,
			IMG_MENU_ABOUT,
			IMG_MENU_ABORT,
			IMG_MENU_QUIT
	};
	for(int i = 0; i < ARRAYSIZE(list); i++)
	{
		if(i != 4 || cl_state.state != GAMESTATE_NOGAME)
			menu_draw_entry(i, list[i]);
	}
	menu_draw_cursor(imenu);
}
/*
 * меню "ИГРА"
 */
static int menu_game(void * ctx)
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
		sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
		return menus[__ctx->menu];
	case LEAVE  :
		sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
		return MENU_MAIN;
	case SPACE: break;
	}
	return MENU_GAME;
}

static void menu_game_draw(const void * ctx_)
{
	const menu_game_ctx_t * ctx = ctx_;
	menu_draw_conback();
	menu_draw_header(IMG_MENU_GAME);
	menu_draw_entry(0, IMG_MENU_G_NEW_P1);
	menu_draw_entry(1, IMG_MENU_G_NEW_P2);
	menu_draw_entry(2, IMG_MENU_G_LOAD);
	menu_draw_cursor(ctx->menu);
};

static int menu_game_new1P(void * ctx)
{
	int ret;
	if(cl_state.state != GAMESTATE_NOGAME)
		return MENU_MAIN;
	cl_state.gamemap = mapList;
	ret = cl_game_create(0);
	if(ret)
	{
		game_halt("Error: Can not create game.");
		return MENU_ABORT;
	}

	client_connect();
	clients_initcams();

	client_event_setgamemap_send(cl_state.gamemap->map);

	return MENU_MAIN;
}

static int menu_game_new2P(void * ctx)
{
	int ret;
	if(cl_state.state != GAMESTATE_NOGAME)
		return MENU_MAIN;
	cl_state.gamemap = mapList;
	ret = cl_game_create(GAMEFLAG_2PLAYERS);
	if(ret)
	{
		game_halt("Error: Can not create game.");
		return MENU_ABORT;
	}

	client_connect();
	client_connect();
	clients_initcams();

	client_event_setgamemap_send(cl_state.gamemap->map);

	return MENU_MAIN;
}


/*
 * меню "ЗАГРУЗКА"
 */
static int menu_game_load(void * ctx)
{
	menu_game_load_ctx_t * __ctx = ctx;
	menu_key_t menukey = menu_key_get();
	switch(__ctx->state)
	{
	case MENU_GAME_LOAD_INIT:
		g_gamesave_cacheinfos();
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
			sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
			__ctx->state = MENU_GAME_LOAD_INIT;
			if(cl_state.state != GAMESTATE_NOGAME)
				return MENU_MAIN;
			if(!gamesaves[__ctx->menu].exist)
				break;

			{

				int ret;
				ret = cl_game_create(0);
				if(ret)
				{
					game_halt("Error: Can not create game.");
					return MENU_ABORT;
				}

				client_connect();
				clients_initcams();

				client_event_gamesave_load_send(__ctx->menu);

			}
			/*
			 TODO: move this to server
			if(!ret)
				return MENU_MAIN;
			if(2<=ret)
			{
				game_msg_error(ret+10);
				return MENU_ABORT;
			}
			*/
			return MENU_MAIN;
		case LEAVE  :
			sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
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
	menu_draw_conback();
	menu_draw_header(IMG_MENU_G_LOAD);
	for(irow = 0; irow < 8; irow++)
	{
		menu_draw_string_indicator_small(irow, 16);
		font_color_set3i(COLOR_7);
		if(!gamesaves[irow].exist)
		{
			video_printf(97+23+4, 33+irow*15, "---===EMPTY===---");
			continue;
		}
		video_printf(97+23+4, 33+irow*15, gamesaves[irow].name);
		//отображение статуса сохраненной игры
		int icol = 16;
		gr2D_setimage0(98+23+4+8*(icol+1), 29+irow*15, image_get(IMG_FLAG_RUS));
		if(gamesaves[irow].flags & GAMEFLAG_2PLAYERS)
		{
			gr2D_setimage0(110+23+4+8*(icol+1), 29+irow*15, image_get(IMG_FLAG_RUS));
		};
	};
	gr2D_setimage0(97, 30 + 15 * __ctx->menu + 2, image_get(IMG_MENU_CUR_1));
}
static int menu_game_save(void * ctx)
{
	menu_game_save_ctx_t * __ctx = ctx;
	size_t l;
	char ch;
	switch(__ctx->state)
	{
	case MENU_GAME_SAVE_INIT:
		g_gamesave_cacheinfos();
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
			sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
			__ctx->rec = gamesaves[__ctx->menu];
			gamesaves[__ctx->menu].exist = true;
			__ctx->state = MENU_GAME_SAVE_INPUT;
			break;
		case LEAVE  :
			sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
			__ctx->state = MENU_GAME_SAVE_INIT;
			cl_state.show_menu = false;
			client_event_nextgamestate_send();
			return MENU_MAIN;
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
			gamesaves[__ctx->menu] = __ctx->rec;
			__ctx->state = MENU_GAME_SAVE_SELECT;
			break;
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_RETURN2:
			__ctx->state = MENU_GAME_SAVE_SAVE;
			break;
		case SDL_SCANCODE_BACKSPACE:
			l = strlen(gamesaves[__ctx->menu].name);
			if(0 < l) gamesaves[__ctx->menu].name[l-1] = 0;
			break;
		default :
			ch = SDL_GetKeyFromScancode(scancode);
			if(ch == 0)break;
			l = strlen(gamesaves[__ctx->menu].name);
			if(ch < 0x80 && l <= 16)
				str_addch(gamesaves[__ctx->menu].name, ch);
		}
		break;
	case MENU_GAME_SAVE_SAVE:
		client_event_gamesave_save_send(__ctx->menu);
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
	menu_draw_header(IMG_MENU_G_SAVE);
	if(__ctx->state == MENU_GAME_SAVE_SELECT)
		gr2D_setimage0(97, 30+15*menu+2, image_get(IMG_MENU_CUR_1));
	for(irow = 0; irow < 8; irow++)
	{
		gr2D_setimage0(97+23, 30+irow*15, image_get(IMG_MENU_LINEL));
		for(icol = 0; icol < 16; icol++)
			gr2D_setimage0(97+23+4+8*icol, 30+irow*15, image_get(IMG_MENU_LINEM));
		gr2D_setimage0(97+23+4+8*icol, 30+irow*15, image_get(IMG_MENU_LINER));
		font_color_set3i(COLOR_7);
		if(!gamesaves[irow].exist)
		{
			video_printf(97+23+4,33+irow*15, "---===EMPTY===---");
			continue;
		}
		video_printf(97+23+4,33+irow*15, gamesaves[irow].name);
		//отображение статуса сохраненной игры
		gr2D_setimage0(98+23+4+8*(icol+1), 29+irow*15, image_get(IMG_FLAG_RUS));
		if(gamesaves[irow].flags & GAMEFLAG_2PLAYERS)
			gr2D_setimage0(110+23+4+8*(icol+1), 29+irow*15, image_get(IMG_FLAG_RUS));
	}
}

/*
 * меню "ВЫБОР"
 */
static int menu_custom(void * ctx)
{
	static menu_selector_t menus[] =
	{
			MENU_CUSTOM,
			MENU_CUSTOM_NEWP1,
			MENU_CUSTOM_NEWP2,
			MENU_CUSTOM_CONNECT,
	};

	menu_custom_ctx_t * __ctx = ctx;
	menu_key_t menukey = menu_key_get();
	switch(menukey)
	{
	case NOTHING: break;
	case UP     : _menu_dec(ARRAYSIZE(menus), &__ctx->menu);break;
	case DOWN   : _menu_inc(ARRAYSIZE(menus), &__ctx->menu);break;
	case LEFT   :
	case RIGHT  :
		if(__ctx->menu == 0)
		{
			if(menukey == LEFT)
				if(cl_state.custommap->prev) cl_state.custommap = cl_state.custommap->prev;
			if(menukey == RIGHT)
				if(cl_state.custommap->next) cl_state.custommap = cl_state.custommap->next;
		}
		break;
	case ENTER  :
		sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
		return menus[__ctx->menu];
	case LEAVE  :
		sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
		return MENU_MAIN;
	case SPACE: break;
	}

	return MENU_CUSTOM;
}


static void menu_custom_draw(const void * ctx)
{
	const menu_custom_ctx_t * __ctx = ctx;
	menu_draw_conback();
	menu_draw_header(IMG_MENU_CASE);
	gr2D_setimage0(120,30+23*0       , image_get(IMG_MENU_ARROWL));
	gr2D_setimage0(260,30+23*0       , image_get(IMG_MENU_ARROWR));
	menu_draw_entry(1, IMG_MENU_G_NEW_P1);
	menu_draw_entry(2, IMG_MENU_G_NEW_P2);
	menu_draw_entry(3, IMG_MENU_CASE_SERVERCONNECT);
	menu_draw_cursor(__ctx->menu);
	font_color_set3i(COLOR_25);
	video_printf(133, 33+23*0, cl_state.custommap->map);
	video_printf(133, 41+23*0, cl_state.custommap->name);
}

static int menu_custom_connect(void * ctx_)
{
	static menu_selector_t menus[] =
	{
			MENU_CUSTOM,
			MENU_CUSTOM_NEWP1,
			MENU_CUSTOM_NEWP2,
			MENU_CUSTOM_CONNECT,
	};

	menu_custom_ctx_t * ctx = ctx_;
	menu_key_t menukey = menu_key_get();
	switch(menukey)
	{
	case NOTHING: break;
	case UP     : _menu_dec(ARRAYSIZE(menus), &ctx->menu);break;
	case DOWN   : _menu_inc(ARRAYSIZE(menus), &ctx->menu);break;
	case LEFT   :
	case RIGHT  :
		if(ctx->menu == 0)
		{
			if(menukey == LEFT)
				if(cl_state.custommap->prev) cl_state.custommap = cl_state.custommap->prev;
			if(menukey == RIGHT)
				if(cl_state.custommap->next) cl_state.custommap = cl_state.custommap->next;
		}
		break;
	case ENTER  :
		sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
		return menus[ctx->menu];
	case LEAVE  :
		sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
		return MENU_MAIN;
	case SPACE: break;
	}

	return MENU_CUSTOM;
}

static void menu_custom_connect_draw(const void * ctx_)
{
	const menu_custom_ctx_t * ctx = ctx_;
	menu_draw_conback();
	menu_draw_header(IMG_MENU_CASE_SERVERCONNECT);
/*
	gr2D_setimage0(120,30+23*0       , image_get(IMG_MENU_ARROWL));
	gr2D_setimage0(260,30+23*0       , image_get(IMG_MENU_ARROWR));
	menu_drawentry(1, IMG_MENU_G_NEW_P1);
	menu_drawentry(2, IMG_MENU_G_NEW_P2);
	menu_drawentry(3, IMG_MENU_CASE_SERVERCONNECT);
	menu_drawcursor(ctx->menu);
	font_color_set3i(COLOR_25);
	video_printf(133, 33+23*0, cl_state.custommap->map);
	video_printf(133, 41+23*0, cl_state.custommap->name);
	*/
}



static int menu_custom_new1P(void * ctx)
{

	int ret;
	if(cl_state.state != GAMESTATE_NOGAME)
		return MENU_MAIN;
	ret = cl_game_create(GAMEFLAG_CUSTOMGAME);
	if(ret)
	{
		game_halt("Error: Can not create game.");
		return MENU_ABORT;
	}

	client_connect();
	clients_initcams();

	client_event_setgamemap_send(cl_state.custommap->map);

	return MENU_MAIN;
}

static int menu_custom_new2P(void * ctx)
{
	int ret;
	if(cl_state.state != GAMESTATE_NOGAME)
		return MENU_MAIN;
	ret = cl_game_create(GAMEFLAG_2PLAYERS | GAMEFLAG_CUSTOMGAME);
	if(ret)
	{
		game_halt("Error: Can not create game.");
		return MENU_ABORT;
	}

	client_connect();
	client_connect();
	clients_initcams();

	client_event_setgamemap_send(cl_state.custommap->map);

	return MENU_MAIN;
}

enum
{
		ACTION_PLAYER_MOVE_NORTH,
		ACTION_PLAYER_MOVE_SOUTH,
		ACTION_PLAYER_MOVE_WEST,
		ACTION_PLAYER_MOVE_EAST,
		ACTION_PLAYER_ATTACK_WEAPON1,
		ACTION_PLAYER_ATTACK_WEAPON2,
		ACTION_PLAYER_ATTACK_WEAPON3,
		ACTION_PLAYER_NUM,
};

static const char *player_actions[ACTION_PLAYER_NUM] =
{
		"+move_north",
		"+move_south",
		"+move_west" ,
		"+move_east" ,
		"+attack_artillery",
		"+attack_missile",
		"+attack_mine"

};

static int menu_options(void * ctx)
{
	menu_options_ctx_t * __ctx = ctx;
#define MENU_ROWS 7
	switch(__ctx->state)
	{
	case MENU_OPTIONS_SELECT:
		switch(menu_key_get())
		{
		case NOTHING: break;
		case UP     : _menu_dec(MENU_ROWS, &__ctx->menu);break;
		case DOWN   : _menu_inc(MENU_ROWS, &__ctx->menu);break;
		case LEFT   : _menu_dec(2, &__ctx->column);break;
		case RIGHT  : _menu_inc(2, &__ctx->column);break;
		case ENTER  :
			sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
			__ctx->state = MENU_OPTIONS_WAIT_KEY;break;
		case LEAVE  :
			sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
			gconf_rebind_all();
			gconf_save();
			__ctx->state = MENU_OPTIONS_SELECT;
			return MENU_MAIN;
		case SPACE  : break;
		}
		break;
	case MENU_OPTIONS_WAIT_KEY:


		if(buffer_isEmpty())
			break;
		buffer_key_t scancode = buffer_dequeue_nowait();
		switch(scancode)
		{
		case SDL_SCANCODE_UNKNOWN: break;
		case SDL_SCANCODE_ESCAPE:
			__ctx->state = MENU_OPTIONS_SELECT;
			break;
		default:
			input_key_unbind(scancode);
			input_action_unbind(__ctx->column, player_actions[__ctx->menu]);
			input_key_bind_act(__ctx->column, scancode, player_actions[__ctx->menu]);
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
	menu_draw_header(IMG_MENU_OPTIONS);
	if(__ctx->state == MENU_OPTIONS_SELECT)
		gr2D_setimage0(58 + __ctx->column * 131, 30 + 12 + 12 * __ctx->menu + 1, image_get(IMG_MENU_CUR_1));
	font_color_set3i(COLOR_25);
	video_printf( 58+131*0, 30+9*0, "[ИГРОК 1]");
	video_printf( 58+131*1, 30+9*0, "[ИГРОК 2]");
	video_printf( 9,32+12*1, "Вперед");
	video_printf( 9,32+12*2, "Назад");
	video_printf( 9,32+12*3, "Влево");
	video_printf( 9,32+12*4, "Вправо");
	video_printf( 9,32+12*5, "Пульки");
	video_printf( 9,32+12*6, "Ракета");
	video_printf( 9,32+12*7, "Мина");

	size_t i;
	for(i = 0; i < ACTION_PLAYER_NUM; i++)
	{
		video_printf(82 + 131 * 0, 32 + 12 + 12 * i, "%d", input_key_get(0, player_actions[i]));
		video_printf(82 + 131 * 1, 32 + 12 + 12 * i, "%d", input_key_get(1, player_actions[i]));
	}
}



/*
 * меню "О ИГРЕ"
 */
static int menu_about(void * ctx)
{
	if(menu_key_get() == LEAVE)
	{
		sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
		return MENU_MAIN;
	}
	return MENU_ABOUT;
};

static void menu_about_draw(const void * ctx)
{
	gr2D_setimage0( 0, 0, image_get(IMG_MENU_I_INTERLV));
	int i = 0;
	coloredtext_t * text;

	font_color_set3i(COLOR_13);
	video_printf( 8,10 * 1, c_strTITLE);
	video_printf(56,10 * 2, c_strCORP);

	for(;;)
	{
		text = &c_about[i];
		if(!text->text)break;
		font_color_sets(&text->color);
		video_printf( 8, 10 * (i + 3), text->text);
		i++;
	}
}


static int menu_abort(void * ctx)
{
	cl_game_abort();
	return MENU_MAIN;
}

static int menu_quit(void * ctx)
{
	cl_game_quit_set();
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
		{ NULL                , menu_custom_connect, menu_custom_connect_draw}, /* MENU_CUSTOM_CONNECT */
		{ &menu_options_ctx   , menu_options     , menu_options_draw}, /* MENU_OPTIONS */
		{ &menu_about_ctx     , menu_about       , menu_about_draw  }, /* MENU_ABOUT */
		{ NULL                , menu_abort       , NULL }, /* MENU_ABORT */
		{ NULL                , menu_quit        , NULL }  /* MENU_QUIT */
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
		if(menu->draw)
			menu->draw(menu->context);
	}
}
