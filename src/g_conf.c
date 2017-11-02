/*
 * g_conf.c
 *
 * game configuration file
 *
 *  Created on: 4 янв. 2017 г.
 *      Author: mastersan
 */

#include "types.h"
#include "cl_input.h"
#include "g_conf.h"
#include "game.h"

#include <SDL2/SDL.h>

static const keybind_t keybinds_permanent[] =
{
        { 0, SDL_SCANCODE_ESCAPE, "menu" },
        { 0, SDL_SCANCODE_X     , "+sfactor" },
        { 0, SDL_SCANCODE_C     , "+dfactor" },
};


static const keybind_t keybinds_default[] =
{
        { 0, SDL_SCANCODE_ESCAPE, "menu" },
        { 0, SDL_SCANCODE_UP    , "+move_north" },
        { 0, SDL_SCANCODE_DOWN  , "+move_south" },
        { 0, SDL_SCANCODE_LEFT  , "+move_west"  },
        { 0, SDL_SCANCODE_RIGHT , "+move_east"  },
        { 0, SDL_SCANCODE_SLASH , "+attack_artillery" },
        { 0, SDL_SCANCODE_PERIOD, "+attack_missile"   },
        { 0, SDL_SCANCODE_COMMA , "+attack_mine"      },
        { 1, SDL_SCANCODE_R   , "+move_north" },
        { 1, SDL_SCANCODE_F   , "+move_south" },
        { 1, SDL_SCANCODE_D   , "+move_west"  },
        { 1, SDL_SCANCODE_G   , "+move_east"  },
        { 1, SDL_SCANCODE_W   , "+attack_artillery" },
        { 1, SDL_SCANCODE_Q   , "+attack_missile"   },
        { 1, SDL_SCANCODE_TAB , "+attack_mine"      },
        { 0, SDL_SCANCODE_Z   , "win"               },
};

static void gconf_rebind_permanent()
{
    size_t i;
    for(i = 0; i < ARRAYSIZE(keybinds_permanent); i++)
    {
        client_key_unbind(keybinds_permanent[i].key);
        client_key_bind(
            keybinds_permanent[i].playerId,
            keybinds_permanent[i].key,
            keybinds_permanent[i].action);
    }

}

void gconf_newbinds()
{
    size_t i;
    for( i = 0; i < ARRAYSIZE(keybinds_default); i++)
    {
        const keybind_t * keybind = &keybinds_default[i];
        client_key_bind(
            keybind->playerId,
            keybind->key,
            keybind->action
        );
    }
}

/*
 * новый конфиг
 */
static int gconf_new()
{
    gconf_newbinds();
    return gconf_save();
}

static int conf_line_save(const client_keybind_t * keybind, void * userdata)
{
    static char *list[] =
    {
            "bind",
            "bind2"
    };

    FILE *f = userdata;

    char * cmd;
    switch(keybind->playerId)
    {
        case 0: cmd = list[1]; break;
        case 1: cmd = list[2]; break;
        default:
            game_halt("gconf_save(): invalid playerId %d.", keybind->playerId);
    }
    if(fprintf(f, "%s %d %s\n", cmd, keybind->key, keybind->action_press) <= 0 )
        return -1;
    return 0;
}
/*
 * запись конфига
 */
int gconf_save()
{
    gconf_rebind_permanent();

    int ret = 0;

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

    ret = client_foreachkey(conf_line_save, f);

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
 * чтение конфига
 */
int gconf_load()
{
    FILE * f;

    char * path = Z_malloc(strlen(game_dir_conf) + strlen(FILENAME_CONFIG) + 1);
    strcpy(path, game_dir_conf);
    strcat(path, FILENAME_CONFIG);
    f = fopen(path, "r");
    Z_free(path);
    if(!f)
        return gconf_new();

    char * line;
    size_t line_size = 0;
    ssize_t value;

    static const char delims[] = " \t\n";

    char * ptrptr;

    for(;;)
    {
        line = NULL;
        value = getline(&line, &line_size, f);
        if(value <= 0)
        {
            free(line);
            break;
        }
        char * tok = strtok_r(line, delims, &ptrptr);

        int player = -1;

        if(!strcmp(tok, "bind"))
        {
            player = 0;
        }
        else if(!strcmp(tok, "bind2"))
        {
            player = 1;
        }

        tok = strtok_r(NULL, delims, &ptrptr);
        if(!tok)
        {
            free(line);
            fclose(f);
            return gconf_new();
        }
        int key = atoi(tok);
        if(!tok)
        {
            free(line);
            fclose(f);
            return gconf_new();
        }
        tok = strtok_r(NULL, delims, &ptrptr);
        char * action = tok;

        client_key_bind(player, key, action);

        free(line);
    };
    fclose(f);
    gconf_rebind_permanent();
    return 0;
}
