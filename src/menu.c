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

#define menu_draw_bkground(image) \
        video_image_draw(0, 0, (image))

#define menu_draw_conback() \
        menu_draw_bkground(IMG_MENU_CONBACK)

#define menu_draw_logo() \
        video_image_draw(277, 159, IMG_MENU_LOGO)

#define menu_draw_header(image) \
        gr2D_setimage00(120, 30 - 23, image_get(image), 0, 22)

#define menu_draw_entry(row, image) \
        gr2D_setimage00(120, 30 + 23 * (row), image_get(image), 0, 22)

#define menu_draw_cursor(row) \
        gr2D_setimage00(120 - 23, 30 + 23 * (row), image_get(IMG_MENU_CUR_0), 0, 22);

#define menu_draw_spinbox_horisontal(row, width) \
        do { \
            video_image_draw(120          , 30 + 23 * (row), IMG_MENU_ARROWL); \
            video_image_draw(120 + (width), 30 + 23 * (row), IMG_MENU_ARROWR); \
        } while (0)

#define menu_draw_string_indicator_small(row, nchars) \
        do { \
            video_image_draw(120, 30 + (row) * 15, IMG_MENU_LINEL); \
            int icol; \
            for(icol = 0; icol < (nchars); icol++) \
            { \
                video_image_draw(120 + 4 + icol * 8, 30 + (row) * 15, IMG_MENU_LINEM); \
            } \
            video_image_draw(120 + 4 + icol * 8, 30 + (row) * 15, IMG_MENU_LINER); \
        } while (0)

#define	menu_draw_cursor_small(row) \
        video_image_draw(97, 30 + 2 + (row) * 15, IMG_MENU_CUR_1)

#define menu_draw_cursor_small_columned(col, row, col_width_pixels) \
        video_image_draw(58 + (col) * (col_width_pixels), 30 + 1 + 12 * (row), IMG_MENU_CUR_1);

#define menu_draw_icon_small(col, row, image) \
        video_image_draw(120 + 1 + 4 + 15 * (col), 29 + (row) * 15, (image));

/* MENU_MAIN */
static menu_main_ctx_t menu_main_ctx = {};

/* MENU_GAME */
static menu_game_ctx_t menu_game_ctx = {};

/* MENU_GAME_NEW1P */

/* MENU_GAME_NEW2P */

/* MENU_GAME_LOAD */
static menu_game_load_ctx_t menu_load_ctx = {};

/* MENU_GAME_SAVE */
static menu_game_save_ctx_t menu_save_ctx = {};

/* MENU_CUSTOM */
static menu_custom_ctx_t menu_custom_ctx = {};

/* MENU_CUSTOM_CONNECT */

static menu_custom_connect_ctx_t menu_custom_connect_ctx = {};

/* MENU_CUSTOM_NEWP1 */

/* MENU_CUSTOM_NEWP2 */

/* MENU_CUSTOM_CONNECT */

/* MENU_OPTIONS */
static menu_options_ctx_t menu_options_ctx = {};
/* MENU_ABOUT */
static menu_about_ctx_t menu_about_ctx = {};
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

static buffer_key_t P_menu_scancode_get(void)
{
    return buffer_dequeue_nowait();
}

static menu_action_t P_menu_scancode2action(buffer_key_t scancode)
{
    switch(scancode)
    {
    case SDL_SCANCODE_RETURN  :
    case SDL_SCANCODE_RETURN2 : return MENU_ACTION_ENTER;
    case SDL_SCANCODE_ESCAPE  : return MENU_ACTION_LEAVE;
    case SDL_SCANCODE_UP      : return MENU_ACTION_UP;
    case SDL_SCANCODE_DOWN    : return MENU_ACTION_DOWN;
    case SDL_SCANCODE_LEFT    : return MENU_ACTION_LEFT;
    case SDL_SCANCODE_RIGHT   : return MENU_ACTION_RIGHT;
    case SDL_SCANCODE_SPACE   : return MENU_ACTION_SPACE;
    case SDL_SCANCODE_UNKNOWN:
    default: return MENU_ACTION_NOTHING;
    }
}

/*
 * главное меню
 */
static int menu_main(buffer_key_t scancode, menu_action_t action, void * ctx_)
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
    menu_main_ctx_t * ctx = ctx_;
    switch(action)
    {
    case MENU_ACTION_NOTHING: break;
    case MENU_ACTION_FIRSTENTRY: break;
    case MENU_ACTION_UP     : _menu_dec(ARRAYSIZE(menus), &ctx->menu);break;
    case MENU_ACTION_DOWN   : _menu_inc(ARRAYSIZE(menus), &ctx->menu);break;
    case MENU_ACTION_LEFT   : break;
    case MENU_ACTION_RIGHT  : break;
    case MENU_ACTION_ENTER  :
        sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
        if(client.gamestate.state == GAMESTATE_NOGAME && ctx->menu == 4)
            return MENU_MAIN;
        return menus[ctx->menu];
    case MENU_ACTION_LEAVE  :
        sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
        if(client.gamestate.state != GAMESTATE_NOGAME)
            client.gamestate.show_menu = false;
        break;
    case MENU_ACTION_SPACE: break;
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
        if(i != 4 || client.gamestate.state != GAMESTATE_NOGAME)
            menu_draw_entry(i, list[i]);
    }
    menu_draw_cursor(imenu);
}
/*
 * меню "ИГРА"
 */
static int menu_game(buffer_key_t scancode, menu_action_t action, void * ctx_)
{
    static menu_selector_t menus[] =
    {
            MENU_GAME_NEW1P,
            MENU_GAME_NEW2P,
            MENU_GAME_LOAD
    };
    menu_game_ctx_t * ctx = ctx_;
    switch(action)
    {
    case MENU_ACTION_NOTHING: break;
    case MENU_ACTION_FIRSTENTRY: break;
    case MENU_ACTION_UP     : _menu_dec(ARRAYSIZE(menus), &ctx->menu);break;
    case MENU_ACTION_DOWN   : _menu_inc(ARRAYSIZE(menus), &ctx->menu);break;
    case MENU_ACTION_LEFT   : break;
    case MENU_ACTION_RIGHT  : break;
    case MENU_ACTION_ENTER  :
        sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
        return menus[ctx->menu];
    case MENU_ACTION_LEAVE  :
        sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
        return MENU_MAIN;
    case MENU_ACTION_SPACE: break;
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

static int menu_game_new1P(buffer_key_t scancode, menu_action_t action, void * ctx)
{
    int ret;
    if(client.gamestate.state != GAMESTATE_NOGAME)
        return MENU_MAIN;
    client.gamestate.gamemap = mapList;
    ret = cl_game_create(0);
    if(ret)
    {
        game_halt("Error: Can not create game.");
        return MENU_ABORT;
    }

    client_connect();
    client_initcams();

    client_req_setgamemap_send(client.gamestate.gamemap->map);

    return MENU_MAIN;
}

static int menu_game_new2P(buffer_key_t scancode, menu_action_t action, void * ctx_)
{
    int ret;
    if(client.gamestate.state != GAMESTATE_NOGAME)
        return MENU_MAIN;
    client.gamestate.gamemap = mapList;
    ret = cl_game_create(GAMEFLAG_2PLAYERS);
    if(ret)
    {
        game_halt("Error: Can not create game.");
        return MENU_ABORT;
    }

    client_connect();
    client_initcams();

    client_req_setgamemap_send(client.gamestate.gamemap->map);

    return MENU_MAIN;
}


/*
 * меню "ЗАГРУЗКА"
 */
static int menu_game_load(buffer_key_t scancode, menu_action_t action, void * ctx_)
{
    menu_game_load_ctx_t * ctx = ctx_;
    switch(action)
    {
    case MENU_ACTION_NOTHING: break;
    case MENU_ACTION_FIRSTENTRY:
        g_gamesave_cacheinfos();
        break;
    case MENU_ACTION_UP     : _menu_dec(8, &ctx->menu);break;
    case MENU_ACTION_DOWN   : _menu_inc(8, &ctx->menu);break;
    case MENU_ACTION_LEFT   : break;
    case MENU_ACTION_RIGHT  : break;
    case MENU_ACTION_ENTER  :
        sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
        if(client.gamestate.state != GAMESTATE_NOGAME)
            return MENU_MAIN;
        if(!gamesaves[ctx->menu].exist)
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
            client_initcams();

            client_req_gamesave_load_send(ctx->menu);

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
    case MENU_ACTION_LEAVE  :
        sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
        return MENU_GAME;
    case MENU_ACTION_SPACE  : break;
    }
    return MENU_GAME_LOAD;

}
static void menu_game_load_draw(const void * ctx_)
{
    const menu_game_load_ctx_t * ctx = ctx_;
    int irow;
    menu_draw_conback();
    menu_draw_header(IMG_MENU_G_LOAD);
    for(irow = 0; irow < 8; irow++)
    {
        char *s;
        bool showstat;
        if((showstat = gamesaves[irow].exist))
            s = gamesaves[irow].name;
        else
            s = "---===EMPTY===---";

        menu_draw_string_indicator_small(irow, 16);
        font_color_set3i(COLOR_7);
        video_printf(97+23+4, 33+irow*15, s);
        //отображение статуса сохраненной игры
        if(showstat)
        {
            menu_draw_icon_small(9, irow, IMG_FLAG_RUS);
            if(gamesaves[irow].flags & GAMEFLAG_2PLAYERS)
            {
                menu_draw_icon_small(10, irow, IMG_FLAG_RUS);
            }
        }
    }

    menu_draw_cursor_small(ctx->menu);
}
static int menu_game_save(buffer_key_t scancode, menu_action_t action, void * ctx_)
{
    menu_game_save_ctx_t * ctx = ctx_;
    size_t l;
    char ch;

    if(action == MENU_ACTION_FIRSTENTRY)
    {
        g_gamesave_cacheinfos();
        ctx->state = MENU_GAME_SAVE_SELECT;
    }

    switch(ctx->state)
    {
    case MENU_GAME_SAVE_SELECT:
        switch(action)
        {
        case MENU_ACTION_NOTHING: break;
        case MENU_ACTION_FIRSTENTRY: break;
        case MENU_ACTION_UP     : _menu_dec(8, &ctx->menu);break;
        case MENU_ACTION_DOWN   : _menu_inc(8, &ctx->menu);break;
        case MENU_ACTION_LEFT   :
        case MENU_ACTION_RIGHT  :
        case MENU_ACTION_ENTER  :
            sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
            ctx->rec = gamesaves[ctx->menu];
            gamesaves[ctx->menu].exist = true;
            ctx->state = MENU_GAME_SAVE_INPUT;
            break;
        case MENU_ACTION_LEAVE  :
            sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
            client.gamestate.show_menu = false;
            client_req_nextgamestate_send();
            return MENU_MAIN;
        case MENU_ACTION_SPACE  :break;
        }
        break;
        case MENU_GAME_SAVE_INPUT:
            switch(scancode)
            {
            case SDL_SCANCODE_UNKNOWN: break;
            case SDL_SCANCODE_ESCAPE:
                gamesaves[ctx->menu] = ctx->rec;
                ctx->state = MENU_GAME_SAVE_SELECT;
                break;
            case SDL_SCANCODE_RETURN:
            case SDL_SCANCODE_RETURN2:
                ctx->state = MENU_GAME_SAVE_SAVE;
                break;
            case SDL_SCANCODE_BACKSPACE:
                l = strlen(gamesaves[ctx->menu].name);
                if(0 < l) gamesaves[ctx->menu].name[l-1] = 0;
                break;
            default :
                ch = SDL_GetKeyFromScancode(scancode);
                if(ch == 0)break;
                l = strlen(gamesaves[ctx->menu].name);
                if(ch < 0x80 && l <= 16)
                    str_addch(gamesaves[ctx->menu].name, ch);
            }
            break;
            case MENU_GAME_SAVE_SAVE:
                client_req_gamesave_save_send(ctx->menu);
                ctx->state = MENU_GAME_SAVE_SELECT;
                break;
    }
    return MENU_GAME_SAVE;
}
/*
 * меню "СОХРАНЕНИЕ"
 */
static void menu_game_save_draw(const void * ctx_)
{
    const menu_game_save_ctx_t * ctx = ctx_;

    int menu = ctx->menu;

    int irow;

    menu_draw_conback();
    menu_draw_header(IMG_MENU_G_SAVE);
    if(ctx->state == MENU_GAME_SAVE_SELECT)
        menu_draw_cursor_small(menu);
    for(irow = 0; irow < 8; irow++)
    {
        char *s;
        bool showstat;
        if((showstat = gamesaves[irow].exist))
            s = gamesaves[irow].name;
        else
            s = "---===EMPTY===---";

        menu_draw_string_indicator_small(irow, 16);
        font_color_set3i(COLOR_7);
        video_printf(97+23+4,33+irow*15, s);

        if(showstat)
        {
            //отображение статуса сохраненной игры
            menu_draw_icon_small(9, irow, IMG_FLAG_RUS);
            if(gamesaves[irow].flags & GAMEFLAG_2PLAYERS)
            {
                menu_draw_icon_small(10, irow, IMG_FLAG_RUS);
            }
        }
    }
}

/*
 * меню "ВЫБОР"
 */
static int menu_custom(buffer_key_t scancode, menu_action_t action, void * ctx_)
{
    static menu_selector_t menus[] =
    {
            MENU_CUSTOM,
            MENU_CUSTOM_NEWP1,
            MENU_CUSTOM_NEWP2,
            MENU_CUSTOM_CONNECT,
    };

    menu_custom_ctx_t * ctx = ctx_;
    switch(action)
    {
    case MENU_ACTION_NOTHING: break;
    case MENU_ACTION_FIRSTENTRY: break;
    case MENU_ACTION_UP     : _menu_dec(ARRAYSIZE(menus), &ctx->menu);break;
    case MENU_ACTION_DOWN   : _menu_inc(ARRAYSIZE(menus), &ctx->menu);break;
    case MENU_ACTION_LEFT   :
    case MENU_ACTION_RIGHT  :
        if(ctx->menu == 0)
        {
            if(action == MENU_ACTION_LEFT)
                if(client.gamestate.custommap->prev) client.gamestate.custommap = client.gamestate.custommap->prev;
            if(action == MENU_ACTION_RIGHT)
                if(client.gamestate.custommap->next) client.gamestate.custommap = client.gamestate.custommap->next;
        }
        break;
    case MENU_ACTION_ENTER  :
        sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
        return menus[ctx->menu];
    case MENU_ACTION_LEAVE  :
        sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
        return MENU_MAIN;
    case MENU_ACTION_SPACE: break;
    }

    return MENU_CUSTOM;
}


static void menu_custom_draw(const void * ctx_)
{
    const menu_custom_ctx_t * ctx = ctx_;
    menu_draw_conback();
    menu_draw_header(IMG_MENU_CASE);

    menu_draw_spinbox_horisontal(0, 140);
    font_color_set3i(COLOR_25);
    video_printf(120 + 13, 33 +     23*0, client.gamestate.custommap->map);
    video_printf(120 + 13, 33 + 8 + 23*0, client.gamestate.custommap->name);

    menu_draw_entry(1, IMG_MENU_G_NEW_P1);
    menu_draw_entry(2, IMG_MENU_G_NEW_P2);
    menu_draw_entry(3, IMG_MENU_CASE_SERVERCONNECT);
    menu_draw_cursor(ctx->menu);
}

static int menu_custom_connect(buffer_key_t scancode, menu_action_t action, void * ctx_)
{
    menu_custom_connect_ctx_t * ctx = ctx_;
    switch(action)
    {
    case MENU_ACTION_NOTHING: break;
    case MENU_ACTION_FIRSTENTRY:

        break;
    case MENU_ACTION_UP     : /* _menu_dec(, &ctx->menu);*/ break;
    case MENU_ACTION_DOWN   : /* _menu_inc(, &ctx->menu);*/ break;
    case MENU_ACTION_LEFT   :
    case MENU_ACTION_RIGHT  : break;
    case MENU_ACTION_ENTER  :
        sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
        //return menus[ctx->menu];
        break;
    case MENU_ACTION_LEAVE  :
        sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
        return MENU_CUSTOM;
    case MENU_ACTION_SPACE: break;
    }

    return MENU_CUSTOM_CONNECT;
}

static void menu_custom_connect_draw(const void * ctx_)
{
    //const menu_custom_ctx_t * ctx = ctx_;
    menu_draw_conback();
    menu_draw_header(IMG_MENU_CASE_SERVERCONNECT);
}



static int menu_custom_new1P(buffer_key_t scancode, menu_action_t action, void * ctx)
{

    int ret;
    if(client.gamestate.state != GAMESTATE_NOGAME)
        return MENU_MAIN;
    ret = cl_game_create(GAMEFLAG_CUSTOMGAME);
    if(ret)
    {
        game_halt("Error: Can not create game.");
        return MENU_ABORT;
    }

    client_connect();
    client_initcams();

    client_req_setgamemap_send(client.gamestate.custommap->map);

    return MENU_MAIN;
}

static int menu_custom_new2P(buffer_key_t scancode, menu_action_t action, void * ctx)
{
    int ret;
    if(client.gamestate.state != GAMESTATE_NOGAME)
        return MENU_MAIN;
    ret = cl_game_create(GAMEFLAG_2PLAYERS | GAMEFLAG_CUSTOMGAME);
    if(ret)
    {
        game_halt("Error: Can not create game.");
        return MENU_ABORT;
    }

    client_connect();
    client_initcams();

    client_req_setgamemap_send(client.gamestate.custommap->map);

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
    ACTION_PLAYER__NUM,
};

static const char *player_actions[ACTION_PLAYER__NUM] =
{
        "+move_north",
        "+move_south",
        "+move_west" ,
        "+move_east" ,
        "+attack_artillery",
        "+attack_missile",
        "+attack_mine",
};

static int menu_options(buffer_key_t scancode, menu_action_t action, void * ctx_)
{
    menu_options_ctx_t * ctx = ctx_;
#define MENU_ROWS 7
#define MENU_COLS 2
    switch(ctx->state)
    {
    case MENU_OPTIONS_SELECT:
        switch(action)
        {
        case MENU_ACTION_NOTHING: break;
        case MENU_ACTION_FIRSTENTRY: break;
        case MENU_ACTION_UP     : _menu_dec(MENU_ROWS, &ctx->menu);break;
        case MENU_ACTION_DOWN   : _menu_inc(MENU_ROWS, &ctx->menu);break;
        case MENU_ACTION_LEFT   : _menu_dec(MENU_COLS, &ctx->column);break;
        case MENU_ACTION_RIGHT  : _menu_inc(MENU_COLS, &ctx->column);break;
        case MENU_ACTION_ENTER  :
            sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
            ctx->state = MENU_OPTIONS_WAIT_KEY;break;
        case MENU_ACTION_LEAVE  :
            sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
            gconf_rebind_all();
            gconf_save();
            ctx->state = MENU_OPTIONS_SELECT;
            return MENU_MAIN;
        case MENU_ACTION_SPACE  : break;
        }
        break;
        case MENU_OPTIONS_WAIT_KEY:
            switch(scancode)
            {
            case SDL_SCANCODE_UNKNOWN: break;
            case SDL_SCANCODE_ESCAPE:
                ctx->state = MENU_OPTIONS_SELECT;
                break;
            default:
                input_key_unbind(scancode);
                input_action_unbind(ctx->column, player_actions[ctx->menu]);
                input_key_bind_act(ctx->column, scancode, player_actions[ctx->menu]);
                ctx->state = MENU_OPTIONS_SELECT;
            }
    }
    return MENU_OPTIONS;
};

/*
 * меню "НАСТРОЙКИ"
 */
static void menu_options_draw(const void * ctx_)
{
    const menu_options_ctx_t * ctx = ctx_;

#define MENU_OPTIONS_DRAW_COLUMN_WIDTH 131
    menu_draw_conback();
    menu_draw_header(IMG_MENU_OPTIONS);
    if(ctx->state == MENU_OPTIONS_SELECT)
        menu_draw_cursor_small_columned(ctx->column, ctx->menu + 1, MENU_OPTIONS_DRAW_COLUMN_WIDTH);

    font_color_set3i(COLOR_25);
    video_printf( 58 + MENU_OPTIONS_DRAW_COLUMN_WIDTH * 0, 30+9*0, "[ИГРОК 1]");
    video_printf( 58 + MENU_OPTIONS_DRAW_COLUMN_WIDTH * 1, 30+9*0, "[ИГРОК 2]");
    video_printf( 9,32+12*1, "Вперед");
    video_printf( 9,32+12*2, "Назад");
    video_printf( 9,32+12*3, "Влево");
    video_printf( 9,32+12*4, "Вправо");
    video_printf( 9,32+12*5, "Пульки");
    video_printf( 9,32+12*6, "Ракета");
    video_printf( 9,32+12*7, "Мина");

    size_t i;
    for(i = 0; i < ACTION_PLAYER__NUM; i++)
    {
        video_printf(82 + 131 * 0, 32 + 12 + 12 * i, "%d", input_key_get(0, player_actions[i]));
        video_printf(82 + 131 * 1, 32 + 12 + 12 * i, "%d", input_key_get(1, player_actions[i]));
    }
}



/*
 * меню "О ИГРЕ"
 */
static int menu_about(buffer_key_t scancode, menu_action_t action, void * ctx)
{
    if(action == MENU_ACTION_LEAVE)
    {
        sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
        return MENU_MAIN;
    }
    return MENU_ABOUT;
};

static void menu_about_draw(const void * ctx)
{
    menu_draw_bkground(IMG_MENU_I_INTERLV);

    font_color_set3i(COLOR_13);
    video_printf( 8,10 * 1, c_strTITLE);
    video_printf(56,10 * 2, c_strCORP);

    int i;
    coloredtext_t * text;
    for(i = 0; (text = &c_about[i])->text != NULL; i++)
    {
        font_color_sets(&text->color);
        video_printf( 8, 10 * (i + 3), text->text);
    }
}


static int menu_abort(buffer_key_t scancode, menu_action_t action, void * ctx)
{
    cl_game_abort();
    return MENU_MAIN;
}

static int menu_quit(buffer_key_t scancode, menu_action_t action, void * ctx)
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
        { &menu_custom_connect_ctx, menu_custom_connect, menu_custom_connect_draw}, /* MENU_CUSTOM_CONNECT */
        { &menu_options_ctx   , menu_options     , menu_options_draw}, /* MENU_OPTIONS */
        { &menu_about_ctx     , menu_about       , menu_about_draw  }, /* MENU_ABOUT */
        { NULL                , menu_abort       , NULL }, /* MENU_ABORT */
        { NULL                , menu_quit        , NULL }  /* MENU_QUIT */
};


int menu_handle(int imenu)
{
    static int imenu_old = -1;
    if(0 <= imenu && imenu < MENU_NUM)
    {
        buffer_key_t scancode;
        menu_action_t action;
        if(imenu_old != imenu)
        {
            scancode = SDL_SCANCODE_UNKNOWN;
            action = MENU_ACTION_FIRSTENTRY;
            imenu_old = imenu;
        }
        else
        {
            scancode = P_menu_scancode_get();
            action = P_menu_scancode2action(scancode);
        }
        menu_t * menu = &menus[imenu];
        if(menu->handle)
            imenu = menu->handle(scancode, action, menu->context);
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
