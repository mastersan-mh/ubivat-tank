/*
 * @file progs_main.c
 *
 * @brief Progs init file
 *
 *  Created on: 27 нояб. 2017 г.
 *      Author: mastersan
 */

#include "progs_main.h"

#include "entities_includes.h"

#include "helpers.h"

#define SPAWN_INFO(xclassname, xmodels_num) \
    { \
        .classname = (xclassname), \
        .models_num = (xmodels_num), \
    }

const game_imports_t * gi = NULL;

static var_descr_t entity_vars_descr[] =
{
        ENTITY_VARS_COMMON,
        VAR_DESCR( VARTYPE_STRING, entity_t, text ),
        VAR_DESCR( VARTYPE_INTEGER, entity_t, amount ),
        VAR_DESCR( VARTYPE_INTEGER, entity_t, item_scores ),
        VAR_DESCR( VARTYPE_INTEGER, entity_t, item_health ),
        VAR_DESCR( VARTYPE_INTEGER, entity_t, item_armor  ),
        VAR_DESCR( VARTYPE_INTEGER, entity_t, item_ammo_artillery ),
        VAR_DESCR( VARTYPE_INTEGER, entity_t, item_ammo_missile ),
        VAR_DESCR( VARTYPE_INTEGER, entity_t, item_ammo_mine    ),
        VAR_DESCR( VARTYPE_INTEGER, entity_t, fragstotal), /* фрагов за пройденые карты */
        VAR_DESCR( VARTYPE_INTEGER, entity_t, frags      ), /* фрагов за карту */
        VAR_DESCR( VARTYPE_INTEGER, entity_t, scores     ), /* набрано очков */
        VAR_DESCR( VARTYPE_INTEGER, entity_t, level      ), /* уровень игрока */
        VAR_DESCR( VARTYPE_FLOAT  , entity_t, stat_traveled_distance)
};

typedef struct
{
    char * classname;
    entity_t * (*spawn) (entity_t * parent, const char * info);
} spawninfo_t;

static const spawninfo_t sinfos[] = {
        {"bull_artillery", bull_artillery_spawn},
        {"bull_missile"  , bull_missile_spawn},
        {"bull_mine"     , bull_mine_spawn},
        {"explode_artillery", explode_artillery_spawn},
        {"explode_missile"  , explode_missile_spawn},
        {"explode_mine"     , explode_mine_spawn},
        {"message", message_spawn},
        {"exit"   , exit_spawn},
        {"item_scores", item_scores_spawn},
        {"item_health", item_health_spawn},
        {"item_armor" , item_armor_spawn},
        {"item_ammo_missile", item_ammo_missile_spawn},
        {"item_ammo_mine"   , item_ammo_mine_spawn},
        {"spawner_player", spawner_player_spawn},
        {"spawner_enemy" , spawner_enemy_spawn },
        {"spawner_boss"  , spawner_boss_spawn  },
        {"player", player_spawn},
        {"enemy" , enemy_spawn },
        {"boss"  , boss_spawn  },
};

entity_t * spawn_entity_by_class(const char * classname, const char * info, entity_t * parent)
{
    size_t i;
    for(i = 0; i < ARRAYSIZE(sinfos); i++)
    {
        const spawninfo_t * sinfo = &sinfos[i];
        if(strcmp(classname, sinfo->classname) != 0)
            continue;
        if(sinfo->spawn)
        {
            return sinfo->spawn(parent, info);
        }
    }
    return NULL;
}

static ENTITY_FUNCTION_ACTION(player_action_move_north_on ) { player_action_move((entity_t *)self, DIR_UP   , true ); }
static ENTITY_FUNCTION_ACTION(player_action_move_north_off) { player_action_move((entity_t *)self, DIR_UP   , false); }
static ENTITY_FUNCTION_ACTION(player_action_move_south_on ) { player_action_move((entity_t *)self, DIR_DOWN , true ); }
static ENTITY_FUNCTION_ACTION(player_action_move_south_off) { player_action_move((entity_t *)self, DIR_DOWN , false); }
static ENTITY_FUNCTION_ACTION(player_action_move_west_on  ) { player_action_move((entity_t *)self, DIR_LEFT , true ); }
static ENTITY_FUNCTION_ACTION(player_action_move_west_off ) { player_action_move((entity_t *)self, DIR_LEFT , false); }
static ENTITY_FUNCTION_ACTION(player_action_move_east_on  ) { player_action_move((entity_t *)self, DIR_RIGHT, true ); }
static ENTITY_FUNCTION_ACTION(player_action_move_east_off ) { player_action_move((entity_t *)self, DIR_RIGHT, false); }

static ENTITY_FUNCTION_ACTION(player_attack_weapon1_on ) { player_action_attack((entity_t *)self, true , WEAP_ARTILLERY); }
static ENTITY_FUNCTION_ACTION(player_attack_weapon1_off) { player_action_attack((entity_t *)self, false, WEAP_ARTILLERY); }
static ENTITY_FUNCTION_ACTION(player_attack_weapon2_on ) { player_action_attack((entity_t *)self, true , WEAP_MISSILE); }
static ENTITY_FUNCTION_ACTION(player_attack_weapon2_off) { player_action_attack((entity_t *)self, false, WEAP_MISSILE); }
static ENTITY_FUNCTION_ACTION(player_attack_weapon3_on ) { player_action_attack((entity_t *)self, true , WEAP_MINE); }
static ENTITY_FUNCTION_ACTION(player_attack_weapon3_off) { player_action_attack((entity_t *)self, false, WEAP_MINE); }
static ENTITY_FUNCTION_ACTION(player_win) { gi->sv_game_win(); }

static entity_action_t player_actions[] =
{
        {"+move_north", player_action_move_north_on },
        {"-move_north", player_action_move_north_off},
        {"+move_south", player_action_move_south_on },
        {"-move_south", player_action_move_south_off},
        {"+move_west" , player_action_move_west_on  },
        {"-move_west" , player_action_move_west_off },
        {"+move_east" , player_action_move_east_on  },
        {"-move_east" , player_action_move_east_off },
        {"+attack_artillery", player_attack_weapon1_on },
        {"-attack_artillery", player_attack_weapon1_off},
        {"+attack_missile"  , player_attack_weapon2_on },
        {"-attack_missile"  , player_attack_weapon2_off},
        {"+attack_mine"     , player_attack_weapon3_on },
        {"-attack_mine"     , player_attack_weapon3_off},
        {"win"              , player_win               },
};


size_t g_entities_num = 0;
entity_t * g_entities;
static void g_world_create(size_t entities_num)
{
    size_t size = entities_num * sizeof(g_entities);
    g_entities_num = entities_num;
    g_entities = Z_malloc(size);
    memset(g_entities, 0, size);
}
static void g_world_destroy(void)
{
    Z_free(g_entities);
}




static void g_world_handle(void)
{
    /*
// touchs
static void P_entity_touchs(body_t * self)
{
    entity_common_t * self_vars = self->entity_;
    if(
            self->erase ||
            self->freezed ||
            !self->spawned ||
            !self_vars->alive
    )
        return;

    body_t * other;

    for( other = CIRCLEQ_NEXT(self, list); !CIRCLEQ_END(other, &bodies); other = CIRCLEQ_NEXT(other, list) )
    {
        bool self_touch = (self->touch != NULL);
        bool other_touch = (other->touch != NULL);

        if( !(self_touch || other_touch) )
            continue;

        if( !entities_in_contact(self, other) )
            continue;

        entity_common_t * other_vars = other->entity_;
        if(
                other->erase ||
                other->freezed ||
                !other->spawned ||
                !other_vars->alive
        )
            continue;

        if(self_touch)
            self->touch((entity_t *)self, (entity_t *)other);
        if(other_touch)
            other->touch((entity_t *)other, (entity_t *)self);

    }
}

     */

    size_t i;
    entity_t * entity;
    for(i = 0; i < g_entities_num; i++)
    {
        entity = &g_entities[i];

    }
}

static void entity_on_read(const char *classname, const char * info)
{

    /*

    entity_model_t * models; // TODO: deprecated, move to entity_t *

    if(info->models_num == 0)
    {
        body->models = NULL;
    }
    else
    {
        body->models = Z_malloc(info->models_num * sizeof(entity_model_t));
        for( i = 0; i < info->models_num; i++)
        {
            body->models[i].name = NULL;
            body->models[i].player.frame = 0.0f;
            body->models[i].player.fseq = NULL;
            body->models[i].player.play_frames_seq = NULL;
        }
    }
*/

    VARS_INFO_DUMP(info, "==== Entity \"%s\" spawn ====", classname);
    entity_t *entity = spawn_entity_by_class(classname, info, NULL);
    gi->link(entity);
}

game_local_t glocal = {};

void init(game_inits_t * init)
{
    glocal.maxclients = init->clients_max;
    glocal.clients = Z_malloc(init->clients_max * sizeof(game_client_t));
    glocal.maxentities = init->entities_max;
}

void done(void)
{
    Z_free(glocal.clients);
}


game_exports_t * progs_init(const game_imports_t * import)
{
    gi = import;

    static game_exports_t game_exports = {
            GAME_EXPORTS_ACTIONS(player_actions),
            GAME_EXPORTS_VARS_DESCR(entity_t, entity_vars_descr),
            .init = init,
            .world_create = g_world_create,
            .world_destroy = g_world_destroy,
            .world_handle = g_world_handle,
            .entity_on_read = entity_on_read,
            .client_player_connect = NULL,
            .client_player_spawn = client_player_spawn,
            .client_player_disconnect = NULL,
    };

    ui_register(player_ui_draw);

    return &game_exports;
}
