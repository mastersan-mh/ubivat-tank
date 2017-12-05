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



static var_descr_t bull_vars[] =
{
        ENTITY_VARS_COMMON,
};

static var_descr_t explode_vars[] =
{
        ENTITY_VARS_COMMON,
};

static var_descr_t message_vars[] =
{
        ENTITY_VARS_COMMON,
        VAR_DESCR( VARTYPE_STRING, message_vars_t, text )
};

static var_descr_t exit_vars[] =
{
        ENTITY_VARS_COMMON,
        VAR_DESCR( VARTYPE_STRING, exit_vars_t, text)
};

var_descr_t item_vars_descr[] =
{
        ENTITY_VARS_COMMON,
        VAR_DESCR( VARTYPE_INTEGER, item_vars_t, amount )
};

static var_descr_t spawner_player_vars[] =
{
        ENTITY_VARS_COMMON,
        VAR_DESCR( VARTYPE_INTEGER, spawn_vars_t, item_scores ),
        VAR_DESCR( VARTYPE_INTEGER, spawn_vars_t, item_health ),
        VAR_DESCR( VARTYPE_INTEGER, spawn_vars_t, item_armor  ),
        VAR_DESCR( VARTYPE_INTEGER, spawn_vars_t, item_ammo_missile ),
        VAR_DESCR( VARTYPE_INTEGER, spawn_vars_t, item_ammo_mine    ),
};

static var_descr_t player_vars[] =
{
        ENTITY_VARS_COMMON,
        VAR_DESCR( VARTYPE_INTEGER, player_vars_t, fragstotal), /* фрагов за пройденые карты */
        VAR_DESCR( VARTYPE_INTEGER, player_vars_t, frags      ), /* фрагов за карту */
        VAR_DESCR( VARTYPE_INTEGER, player_vars_t, scores     ), /* набрано очков */
        VAR_DESCR( VARTYPE_INTEGER, player_vars_t, level      ), /* уровень игрока */

        VAR_DESCR( VARTYPE_INTEGER, player_vars_t, item_health         ),
        VAR_DESCR( VARTYPE_INTEGER, player_vars_t, item_armor          ),
        VAR_DESCR( VARTYPE_INTEGER, player_vars_t, item_ammo_artillery ),
        VAR_DESCR( VARTYPE_INTEGER, player_vars_t, item_ammo_missile   ),
        VAR_DESCR( VARTYPE_INTEGER, player_vars_t, item_ammo_mine      ),
};

static const game_exports_entityinfo_t ge_entities_info[] = {
        SPAWN_INFO("bull_artillery", bull_vars_t, bull_vars, 1),
        SPAWN_INFO("bull_missile"  , bull_vars_t, bull_vars, 1),
        SPAWN_INFO("bull_mine"     , bull_vars_t, bull_vars, 1),
        SPAWN_INFO("explode_artillery", explode_vars_t, explode_vars, 1),
        SPAWN_INFO("explode_missile"  , explode_vars_t, explode_vars, 1),
        SPAWN_INFO("explode_mine"     , explode_vars_t, explode_vars, 1),
        SPAWN_INFO("message", message_vars_t, message_vars, 0),
        SPAWN_INFO("exit"   , exit_vars_t   , exit_vars   , 1),
        SPAWN_INFO("item_scores", item_vars_t, item_vars_descr, 1),
        SPAWN_INFO("item_health", item_vars_t, item_vars_descr, 1),
        SPAWN_INFO("item_armor" , item_vars_t, item_vars_descr, 1),
        SPAWN_INFO("item_ammo_missile", item_vars_t, item_vars_descr, 1),
        SPAWN_INFO("item_ammo_mine"   , item_vars_t, item_vars_descr, 1),
        SPAWN_INFO("spawner_player", spawn_vars_t, spawner_player_vars, 0),
        SPAWN_INFO("spawner_enemy" , spawn_vars_t, spawner_player_vars, 0),
        SPAWN_INFO("spawner_boss"  , spawn_vars_t, spawner_player_vars, 0),
        SPAWN_INFO("player", player_vars_t, player_vars, 2),
        SPAWN_INFO("enemy" , player_vars_t, player_vars, 2),
        SPAWN_INFO("boss"  , player_vars_t, player_vars, 2),
};

typedef struct
{
    char * classname;
    ENTITY (*spawn) (ENTITY parent, const char * info);
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

ENTITY spawn_entity_by_class(const char * classname, const char * info, ENTITY parent)
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

static ENTITY_FUNCTION_ACTION(player_action_move_north_on ) { player_action_move(entity_vars(self), DIR_UP   , true ); }
static ENTITY_FUNCTION_ACTION(player_action_move_north_off) { player_action_move(entity_vars(self), DIR_UP   , false); }
static ENTITY_FUNCTION_ACTION(player_action_move_south_on ) { player_action_move(entity_vars(self), DIR_DOWN , true ); }
static ENTITY_FUNCTION_ACTION(player_action_move_south_off) { player_action_move(entity_vars(self), DIR_DOWN , false); }
static ENTITY_FUNCTION_ACTION(player_action_move_west_on  ) { player_action_move(entity_vars(self), DIR_LEFT , true ); }
static ENTITY_FUNCTION_ACTION(player_action_move_west_off ) { player_action_move(entity_vars(self), DIR_LEFT , false); }
static ENTITY_FUNCTION_ACTION(player_action_move_east_on  ) { player_action_move(entity_vars(self), DIR_RIGHT, true ); }
static ENTITY_FUNCTION_ACTION(player_action_move_east_off ) { player_action_move(entity_vars(self), DIR_RIGHT, false); }

static ENTITY_FUNCTION_ACTION(player_attack_weapon1_on ) { player_action_attack(entity_vars(self), true , WEAP_ARTILLERY); }
static ENTITY_FUNCTION_ACTION(player_attack_weapon1_off) { player_action_attack(entity_vars(self), false, WEAP_ARTILLERY); }
static ENTITY_FUNCTION_ACTION(player_attack_weapon2_on ) { player_action_attack(entity_vars(self), true , WEAP_MISSILE); }
static ENTITY_FUNCTION_ACTION(player_attack_weapon2_off) { player_action_attack(entity_vars(self), false, WEAP_MISSILE); }
static ENTITY_FUNCTION_ACTION(player_attack_weapon3_on ) { player_action_attack(entity_vars(self), true , WEAP_MINE); }
static ENTITY_FUNCTION_ACTION(player_attack_weapon3_off) { player_action_attack(entity_vars(self), false, WEAP_MINE); }
static ENTITY_FUNCTION_ACTION(player_win) { sv_game_win(); }

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


void entity_on_read(const char *classname, const char * info)
{
    VARS_INFO_DUMP(info, "==== Entity \"%s\" spawn ====", classname);
    spawn_entity_by_class(classname, info, NULL);
}

game_exports_t * progs_init(void)
{
    static game_exports_t game_exports = {
            GAME_EXPORTS_ENTITIES_INFO(ge_entities_info),
            GAME_EXPORTS_ACTIONS(player_actions),
            .client_player_connect = NULL,
            .client_player_spawn = client_player_spawn,
            .client_player_disconnect = NULL,
            .entity_on_read = entity_on_read

    };

    ui_register(player_ui_draw);

    return &game_exports;
}
