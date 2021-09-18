/*
 * ang_actions.c
 *
 *  Created on: 25 сент. 2021 г.
 *      Author: mastersan
 */

#include "eng_actions.h"

#include "system/tree.h"
#include "game.h"

#include <SDL2/SDL.h>

struct eng_actions_ctx
{
    struct eng_action * registered;
    RB_HEAD(eng_actions, eng_action) sorted;
};

static struct eng_actions_ctx eng_actions_ctx;

/**
 * @brief Game action and it's name
 */
struct eng_action
{
    RB_ENTRY(eng_action) node;
    const struct eng_game_action * action;
};

static int P_action_cmp(void * a_, void * b_)
{
    struct eng_action * a = a_;
    struct eng_action * b = b_;
    return strncmp(a->action->name, b->action->name, ENG_ACTION_NAME_SIZE);
}

RB_GENERATE_FIND(eng_actions, eng_action, node, P_action_cmp, static);
RB_GENERATE_INSERT_COLOR(eng_actions, eng_action, node, static);
RB_GENERATE_INSERT(eng_actions, eng_action, node, P_action_cmp, static);


int eng_actions_register(
        const struct eng_game_action * game_actions,
        size_t game_actions_num
)
{

    eng_actions_ctx.registered = Z_calloc(game_actions_num, sizeof(struct eng_action));
    if(eng_actions_ctx.registered == NULL)
    {
        game_halt("Out of memory");
        return -1;
    }

    size_t i;
    for(i = 0; i < game_actions_num; ++i)
    {
        const struct eng_game_action * game_action = &game_actions[i];
        struct eng_action * eng_action = &eng_actions_ctx.registered[i];
        eng_action->action = game_action;

        struct eng_action * ent_dup = RB_INSERT(eng_actions, &eng_actions_ctx.sorted, eng_action);
        if(ent_dup != NULL)
        {
            game_halt("Action `%s` already registered", game_action->name);
            return -1;
        }
    }
    return 0;
}

const struct eng_game_action * eng_action_get(
        const char * action_name
)
{
    struct eng_action tmp_action;
    struct eng_game_action game_action;

    game_action.name = action_name;
    tmp_action.action = &game_action;

    struct eng_action * ent_action =
            RB_FIND(eng_actions, &eng_actions_ctx.sorted, &tmp_action);
    if(ent_action == NULL)
    {
        return NULL;
    }
    return ent_action->action;
}

void eng_action_exec(const char * action_name)
{
    const struct eng_game_action * game_action = eng_action_get(action_name);
    if(game_action == NULL)
    {
        game_warn("Unknown action `%s`", action_name);
        return;
    }

    if(game_action->func != NULL)
    {
        game_action->func();
    }
}

void eng_actions_init(void)
{
    eng_actions_ctx.registered = NULL;
    RB_INIT(&eng_actions_ctx.sorted);
}

void eng_actions_done(void)
{
    Z_free(eng_actions_ctx.registered);
}
