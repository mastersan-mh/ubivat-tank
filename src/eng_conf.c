/*
 * g_conf.c
 *
 * game configuration file
 *
 *  Created on: 4 янв. 2017 г.
 *      Author: mastersan
 */

#include "types.h"
#include "eng_conf.h"
#include "game.h"

#include "system/tree.h"

#include <SDL2/SDL.h>

/**
 * @brief Bindidng of scancode and action
 */
struct eng_bind
{
    RB_ENTRY(eng_bind) node;
    SDL_Scancode scancode;
    char action_name[ENG_ACTION_NAME_SIZE];
};

struct eng_conf
{
    RB_HEAD(eng_binds, eng_bind) binds;
    char * path;
};

static struct eng_conf conf;


static int P_bind_cmp(void * a_, void * b_)
{
    struct eng_bind * a = a_;
    struct eng_bind * b = b_;
    return (long)(a->scancode) - (long)(b->scancode);
}

RB_GENERATE_FIND(eng_binds, eng_bind, node, P_bind_cmp, static);
RB_GENERATE_INSERT_COLOR(eng_binds, eng_bind, node, static);
RB_GENERATE_INSERT(eng_binds, eng_bind, node, P_bind_cmp, static);
RB_GENERATE_REMOVE_COLOR(eng_binds, eng_bind, node, static);
RB_GENERATE_REMOVE(eng_binds, eng_bind, node, static);
RB_GENERATE_MINMAX(eng_binds, eng_bind, node, static);
RB_GENERATE_NEXT(eng_binds, eng_bind, node, static);

const char * eng_scancode_name_get(SDL_Scancode scancode)
{
    return SDL_GetScancodeName(scancode);
}

int eng_conf_key_bind(
        SDL_Scancode scancode,
        const char * action
)
{
    struct eng_bind * ent_bind = Z_malloc(sizeof(struct eng_bind));
    if(ent_bind == NULL)
    {
        game_halt("Out of memory");
        return -1;
    }
    ent_bind->scancode = scancode;
    strncpy(ent_bind->action_name, action, ENG_ACTION_NAME_SIZE);

    struct eng_bind * ent_bind_dup = RB_INSERT(eng_binds, &conf.binds, ent_bind);
    if(ent_bind_dup != NULL)
    {
        Z_free(ent_bind);
        game_warn("Key `%s` already binded, unbind first", eng_scancode_name_get(scancode));
        return -1;
    }

    return 0;
}

const char * eng_conf_key_action_get(
        SDL_Scancode scancode
)
{
    struct eng_bind tmp_bind;
    tmp_bind.scancode = scancode;

    struct eng_bind * ent_bind =
            RB_FIND(eng_binds, &conf.binds, &tmp_bind);
    if(ent_bind == NULL)
    {
        return NULL;
    }

    return ent_bind->action_name;
}

const SDL_Scancode * eng_conf_action_key_get(const char * action_name)
{
    struct eng_bind * ent_bind;

    RB_FOREACH(ent_bind, eng_binds, &conf.binds)
    {
        if(strcmp(ent_bind->action_name, action_name) == 0)
        {
            return &ent_bind->scancode;
        }
    }
    return NULL;
}

void eng_conf_key_unbind(
        SDL_Scancode scancode
)
{
    struct eng_bind tmp_bind;
    tmp_bind.scancode = scancode;

    struct eng_bind * ent_bind =
            RB_FIND(eng_binds, &conf.binds, &tmp_bind);
    if(ent_bind == NULL)
    {
        return;
    }

    RB_REMOVE(eng_binds, &conf.binds, ent_bind);
    Z_free(ent_bind);
}

void eng_conf_key_unbindall(void)
{
    while(!RB_EMPTY(&conf.binds))
    {
        struct eng_bind * ent_bind =
                RB_ROOT(&conf.binds);
        RB_REMOVE(eng_binds, &conf.binds, ent_bind);
        Z_free(ent_bind);
    }
}

static int P_conf_line_save(const struct eng_bind * keybind, FILE *f)
{
    int res;
    res = fprintf(f, "bind %d %s\n",
            keybind->scancode,
            keybind->action_name
    );
    if(res <= 0)
    {
        game_error("Config file I/O error");
        return -1;
    }
    return 0;
}
/*
 * запись конфига
 */
int eng_conf_save(void)
{
    FILE * f = fopen(conf.path, "w");
    if(f == NULL)
    {
        game_error("Unable to open config file `%s`", conf.path);
        return -1;
    }

    struct eng_bind * ent_bind;
    RB_FOREACH(ent_bind, eng_binds, &conf.binds)
    {
        P_conf_line_save(ent_bind, f);
    }

    fclose(f);
    return 0;
};

/*
 * чтение конфига
 */
int eng_conf_load()
{
    FILE * f;

    f = fopen(conf.path, "r");
    if(f == NULL)
    {
        game_warn("Unable to load configuration file `%s`", conf.path);
        return -1;
    }

#define LINE_SIZE_MAX 4096
    char line[LINE_SIZE_MAX];
    char * value;

    static const char delims[] = " \t\n";

    char * ptrptr;

    size_t iline = 0;
    for(;;)
    {
        iline++;

        value = fgets(line, LINE_SIZE_MAX, f);
        if(value == NULL)
        {
            break;
        }
        char * tok = strtok_r(line, delims, &ptrptr);
        if(tok == NULL)
        {
            game_warn("Invalid config line %u", iline);
            continue;
        }

        if(strcmp(tok, "bind") != 0)
        {
            game_warn("Invalid config line %u", iline);
            continue;
        }

        tok = strtok_r(NULL, delims, &ptrptr);
        if(!tok)
        {
            game_warn("Invalid config line %u", iline);
            continue;
        }
        int key = atoi(tok);
        if(!tok)
        {
            game_warn("Invalid config line %u", iline);
            continue;
        }
        tok = strtok_r(NULL, delims, &ptrptr);
        char * action = tok;

        eng_conf_key_bind(key, action);
    };
    fclose(f);
    return 0;
}


int eng_conf_init(void)
{
    RB_INIT(&conf.binds);

    conf.path = Z_malloc(strlen(game_dir_conf) + strlen(FILENAME_CONFIG) + 1);
    strcpy(conf.path, game_dir_conf);
    strcat(conf.path, FILENAME_CONFIG);

    return 0;
}

void eng_conf_done(void)
{

}
