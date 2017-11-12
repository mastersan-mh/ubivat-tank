/*
 *        Ubivat Tank
 *        обработка игрока
 *
 * by Master San
 */

#include "ent_player.h"
#include "ent_player_think.h"
#include "ent_spawn.h"
#include "ent_message.h"
#include "ent_exit.h"
#include "ent_items.h"
#include "ent_bull.h"
#include "ent_explode.h"
#include "ent_message.h"
#include "ent_weap.h"

#include "ui.h"
#include "system.h"
#include "types.h"
#include "entity.h"
#include "entity_helpers.h"
#include "img.h"
#include "map.h"
#include "server.h"
#include "sv_game.h"
#include "game.h"
#include "sound.h"

void coerce_value_int(int * val, int min, int max)
{
    if(*val > max) *val = max;
    else if(*val < min) *val = min;
}

player_invitemtype_t player_entity_to_itemtype(const entity_t * entity)
{
    static const char *entitynames[] =
    {
            "item_scores",
            "item_health",
            "item_armor" ,
            "item_ammo_artillery",
            "item_ammo_missile",
            "item_ammo_mine"
    };
    size_t i;
    for(i = 0; i < ARRAYSIZE(entitynames); i++)
    {
        if(ENTITY_IS(entity, entitynames[i]))
            return i;
    }
    game_halt("player_entity_to_itemtype(): invalid item = %s\n", entity->info->name);
    return -1;
}

/*
 * инициализируем игрока при спавне
 */
void player_spawn_init(void * player , void * spawn)
{
    if(!spawn)
        game_halt("Error: Player spawn is NULL, can not create player.");

    player_vars_t * pvars = entity_vars(player);

    spawn_vars_t * sp = entity_vars(spawn);

    VEC2_COPY(pvars->origin, sp->origin);

    var_int_t spawn_scores = sp->item_scores;
    if(0 <= spawn_scores && spawn_scores <= PLAYER_SCORES_MAX)
        pvars->scores = spawn_scores;

    player_class_init(player, pvars);


    playerinfo_t * playerinfo = &playerinfo_table[pvars->level];

    void inventory_item_init(void * player, INTEGER * playervar, INTEGER infovar, INTEGER spawn_value)
    {
        if( spawn_value > infovar )
            spawn_value = infovar;
        else if(spawn_value <=0)
        {
            if( !ENTITY_IS(player, "player") ) /* враг/босс */
                spawn_value = infovar;
            else
            {
                /* игрок-человек */
                if(sv_game_flag_localgame())
                    spawn_value = infovar;
                else
                {
                    //по уровням
                    if(sv_game_is_first_map()) // первая карта
                        spawn_value = infovar;
                    else // не первая карта
                        spawn_value = *playervar;
                }
            }
        }
        *playervar = spawn_value;
    }

    var_int_t player_health = pvars->item_health;
    if(player_health < 0)
    {
        player_health = sp->item_health;
        if(player_health < 0)
            player_health = playerinfo->items[ITEM_SCORES];
        pvars->item_health = player_health;
    }

    /* инициализация инвентаря */
    inventory_item_init(player, &pvars->item_health      , playerinfo->items[ITEM_HEALTH]      , sp->item_health);
    inventory_item_init(player, &pvars->item_armor       , playerinfo->items[ITEM_ARMOR]       , sp->item_armor );
    inventory_item_init(player, &pvars->item_ammo_missile, playerinfo->items[ITEM_AMMO_MISSILE], sp->item_ammo_missile);
    inventory_item_init(player, &pvars->item_ammo_mine   , playerinfo->items[ITEM_AMMO_MINE]   , sp->item_ammo_mine);

    pvars->frags = 0;
    pvars->bull         = NULL;
    pvars->move.speed   = 0;
    pvars->move.go      = false;
    pvars->attack       = false;
    pvars->reloadtime_d = 0;
};


playerinfo_t playerinfo_table[__PLAYER_LEVEL_NUM] =
{
        /* SCORES                 , HEALTH, ARMOR,  AMMO_ARTILLERY,   AMMO_MISSILE,     AMMO_MINE,     speed,            modelname */
        { { PLAYER_SCOREPERCLASS * 1,    100,     0, PLAYER_ITEM_AMOUNT_INF, PLAYER_ITEM_AMOUNT_NA, PLAYER_ITEM_AMOUNT_NA }, 40/2 * SPEEDSCALE, "tank1"},
        { { PLAYER_SCOREPERCLASS * 2,    100,    50, PLAYER_ITEM_AMOUNT_INF, PLAYER_ITEM_AMOUNT_NA, PLAYER_ITEM_AMOUNT_NA }, 50/2 * SPEEDSCALE, "tank2"},
        { { PLAYER_SCOREPERCLASS * 3,    100,   100, PLAYER_ITEM_AMOUNT_INF, 50            , PLAYER_ITEM_AMOUNT_NA }, 60/2 * SPEEDSCALE, "tank3"},
        { { PLAYER_SCOREPERCLASS * 4,    200,   150, PLAYER_ITEM_AMOUNT_INF, 50            , PLAYER_ITEM_AMOUNT_NA }, 70/2 * SPEEDSCALE, "tank4"},
        { { PLAYER_SCOREPERCLASS * 5,    200,   200, PLAYER_ITEM_AMOUNT_INF, 50            , 50             }, 90/2 * SPEEDSCALE, "tank5"},
        { { PLAYER_SCOREPERCLASS * 6,   5000,  5000, PLAYER_ITEM_AMOUNT_INF, 50            , 50             }, 90/2 * SPEEDSCALE, "tank5"}  /* BOSS */
};

static void tank_common_modelaction_endframef(void * entity, unsigned int imodel, char * actionname)
{
    entity_model_play_start(entity, imodel, actionname);
}
static const ent_modelaction_t tank_modelactions[] =
{
        {
                .name = "run",
                .startframe = 0,
                .endframe = 3,
                .endframef = tank_common_modelaction_endframef
        }
};

static entitymodel_t tank_player_models[] =
{
        {
                .modelname = "tank1",
                .modelscale = 16.0f / 2.0f,
                .translation = { 0.0, 0.0 },
                .actions_num = 1,
                .actions = tank_modelactions
        },
        {
                .modelname = "flag_player",
                .modelscale = 16.0f / 2.0f,
                .translation = { 0.0, 0.0 },
                .actions_num = 0,
                .actions = NULL
        }
};

static entitymodel_t tank_enemy_models[] =
{
        {
                .modelname = "tank1",
                .modelscale = 16.0f / 2.0f,
                .translation = { 0.0, 0.0 },
                .actions_num = 1,
                .actions = tank_modelactions
        },
        {
                .modelname = "flag_enemy",
                .modelscale = 16.0f / 2.0f,
                .translation = { 0.0, 0.0 },
                .actions_num = 0,
                .actions = NULL
        }
};

static entitymodel_t tank_boss_models[] =
{
        {
                .modelname = "tank1",
                .modelscale = 16.0f / 2.0f,
                .translation = { 0.0, 0.0 },
                .actions_num = 1,
                .actions = tank_modelactions
        },
        {
                .modelname = "flag_boss",
                .modelscale = 16.0f / 2.0f,
                .translation = { 0.0, 0.0 },
                .actions_num = 0,
                .actions = NULL
        }
};

static ENTITY_FUNCTION_INIT(player_init);
static ENTITY_FUNCTION_DONE(player_done);

static ENTITY_FUNCTION_SPAWN(player_spawn)
{
    entity_t * spawn = entity_get_random("spawn_player");
    player_spawn_init(this, spawn);
}

static ENTITY_FUNCTION_HANDLE(player_handle);

static ENTITY_FUNCTION_INIT(enemy_init);
static ENTITY_FUNCTION_DONE(enemy_done);
static ENTITY_FUNCTION_HANDLE(enemy_handle);

static ENTITY_FUNCTION_INIT(boss_init);
static ENTITY_FUNCTION_DONE(boss_done);
static ENTITY_FUNCTION_HANDLE(boss_handle);


/*
 * touchs
 */
static void player_touch_message(entity_t * actor, entity_t * exposed)
{
    entity_message_t * msg = exposed->common;
    //отправим сообщение игроку
    sv_game_message_send(msg->text);
}

static void player_touch_exit(entity_t * actor, entity_t * exposed)
{
    entity_exit_t * msg = exposed->common;
    //отправим сообщение игроку
    sv_game_message_send(msg->text);
    sv_game_win();
}

static void player_touch_item_scores(entity_t * player, entity_t * item)
{
    player_vars_t * pl = player->common;
    var_int_t level = pl->level;
    playerinfo_t *playerinfo = &playerinfo_table[level];

    ENTITY_UNSPAWN(item);

    entity_item_t * it = item->common;

    var_int_t scores = ( pl->scores += it->amount );

    player_class_init(player, player->common);
    if(scores / PLAYER_SCOREPERCLASS >= 5)
    {
        if(pl->item_health < playerinfo->items[ITEM_HEALTH])
            pl->item_health = playerinfo->items[ITEM_HEALTH];
        if(pl->item_armor < playerinfo->items[ITEM_ARMOR])
            pl->item_armor = playerinfo->items[ITEM_ARMOR];
    }
}


static void player_touch_item(entity_t * player, entity_t * item)
{
    player_invitemtype_t invitemtype = player_entity_to_itemtype(item);

    player_vars_t * pl = player->common;
    entity_item_t * it = item->common;

    var_int_t level = pl->level;
    playerinfo_t *playerinfo = &playerinfo_table[level];

    int itemamount = it->amount;

    void aaaa(playerinfo_t *playerinfo, entity_t * player, INTEGER * amount)
    {
        int player_itemamount;
        if(
                playerinfo->items[invitemtype] > 0 &&
                (
                        ( (player_itemamount = *amount) ) < playerinfo->items[invitemtype] ||
                        itemamount < 0
                )
        )
        {
            ENTITY_UNSPAWN(item);
            player_itemamount += itemamount;
            coerce_value_int(&player_itemamount, 0, playerinfo->items[invitemtype]);
            *amount = player_itemamount;
        }
    }

    switch(invitemtype)
    {
        case ITEM_SCORES: aaaa(playerinfo, player, &pl->scores); break;
        case ITEM_HEALTH: aaaa(playerinfo, player, &pl->item_health); break;
        case ITEM_ARMOR : aaaa(playerinfo, player, &pl->item_armor); break;
        case ITEM_AMMO_ARTILLERY: aaaa(playerinfo, player, &pl->item_ammo_artillery); break;
        case ITEM_AMMO_MISSILE  : aaaa(playerinfo, player, &pl->item_ammo_missile); break;
        case ITEM_AMMO_MINE     : aaaa(playerinfo, player, &pl->item_ammo_mine); break;
        default: ;
    }

}

static void player_action_move(player_vars_t * pl, direction_t dir, bool go)
{
    if(go)
        pl->dir = dir;
    else
    {
        if(pl->dir != dir)
            return;
    }
    pl->move.go = go;
}

static void player_action_attack(player_vars_t * pl, bool attack, weapontype_t weap)
{
    if(attack)
        pl->weap = weap;
    pl->attack = attack;
}


ENTITY_FUNCTION_ACTION(player_action_move_north_on ) { player_action_move(thisdata, DIR_UP   , true ); }
ENTITY_FUNCTION_ACTION(player_action_move_north_off) { player_action_move(thisdata, DIR_UP   , false); }
ENTITY_FUNCTION_ACTION(player_action_move_south_on ) { player_action_move(thisdata, DIR_DOWN , true ); }
ENTITY_FUNCTION_ACTION(player_action_move_south_off) { player_action_move(thisdata, DIR_DOWN , false); }
ENTITY_FUNCTION_ACTION(player_action_move_west_on  ) { player_action_move(thisdata, DIR_LEFT , true ); }
ENTITY_FUNCTION_ACTION(player_action_move_west_off ) { player_action_move(thisdata, DIR_LEFT , false); }
ENTITY_FUNCTION_ACTION(player_action_move_east_on  ) { player_action_move(thisdata, DIR_RIGHT, true ); }
ENTITY_FUNCTION_ACTION(player_action_move_east_off ) { player_action_move(thisdata, DIR_RIGHT, false); }

ENTITY_FUNCTION_ACTION(player_attack_weapon1_on ) { player_action_attack(thisdata, true , WEAP_ARTILLERY); }
ENTITY_FUNCTION_ACTION(player_attack_weapon1_off) { player_action_attack(thisdata, false, WEAP_ARTILLERY); }
ENTITY_FUNCTION_ACTION(player_attack_weapon2_on ) { player_action_attack(thisdata, true , WEAP_MISSILE); }
ENTITY_FUNCTION_ACTION(player_attack_weapon2_off) { player_action_attack(thisdata, false, WEAP_MISSILE); }
ENTITY_FUNCTION_ACTION(player_attack_weapon3_on ) { player_action_attack(thisdata, true , WEAP_MINE); }
ENTITY_FUNCTION_ACTION(player_attack_weapon3_off) { player_action_attack(thisdata, false, WEAP_MINE); }
ENTITY_FUNCTION_ACTION(player_win) { sv_game_win(); }


static var_descr_t player_vars[] =
{
        ENTITY_COMMON_VARS,
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

static entitytouch_t player_touchs[] =
{
        { "item_scores", player_touch_item_scores },
        { "item_health", player_touch_item        },
        { "item_armor"       , player_touch_item },
        { "item_ammo_missile", player_touch_item },
        { "item_ammo_mine"   , player_touch_item },
        { "message", player_touch_message },
        { "exit"   , player_touch_exit    },
};

static entityaction_t player_actions[] =
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

static void player_handle_common(void * player, player_vars_t * pl);

ENTITY_FUNCTION_INIT(player_init)
{
    player_vars_t * pl = thisdata;

#if defined(_DEBUG_PLAYERMAXLEVEL)
    pl->scores      = 9000;
    pl->item_health = 100;
    pl->item_armor  = 100;
    pl->item_ammo_missile = 100;
    pl->item_ammo_mine    = 100;
#endif

    player_spawn_init(this, parent);// parent = spawn



}

ENTITY_FUNCTION_DONE(player_done)
{
    player_vars_t * pl = thisdata;
    sound_play_stop(this, -1);
    ctrl_AI_done(&(pl->brain));
}

ENTITY_FUNCTION_HANDLE(player_handle)
{
    player_handle_common(this, thisdata);
}

ENTITY_FUNCTION_INIT(enemy_init)
{
    player_spawn_init(this, parent);
    player_vars_t * pl = thisdata;
    ctrl_AI_init(&pl->brain);
}

ENTITY_FUNCTION_DONE(enemy_done)
{
    player_done(this, thisdata);
}

ENTITY_FUNCTION_HANDLE(enemy_handle)
{
    think_enemy(this);
    player_handle_common(this, thisdata);
}

ENTITY_FUNCTION_INIT(boss_init)
{
    player_spawn_init(this, parent);
    player_vars_t * pl = thisdata;
    ctrl_AI_init(&pl->brain);
}
ENTITY_FUNCTION_DONE(boss_done)
{
    player_done(this, thisdata);
}
ENTITY_FUNCTION_HANDLE(boss_handle)
{
    think_enemy(this);
    player_handle_common(this, thisdata);
}

/*
 * набор и проверка кодов (добавление 10.05.2006)
 */
void player_checkcode(void)
{
    /*

	static char codebuf[17];
	static int codecount = 0;
	static long codetimer = 0;

	bool allow;
	char key;

	if(codetimer<10)
	{
		codetimer += game.P0->time.delta;
	}
	else
	{
		if(!keypressed())
		{
			if(codetimer<100) codetimer += game.P0->time.delta;
			else
			{
				codebuf[0] = 0;
				codecount = 0;
			}
		}
		else
		{
			key = readkey();
			if(key == 0) readkey();
			else {
				allow = checkchar(key);
				if(allow) {
					codetimer = 0;
					if(codecount<16) {
						codebuf[codecount] = strZ_UPcheng(key);
						codecount++;
						codebuf[codecount] = 0;

						if(strcmp(codebuf, code_levelup)==0) {
							game.P0->charact.scores += 200;
							if(c_score_max<game.P0->charact.scores) game.P0->charact.scores = c_score_max;
							player_class_init(game.P0);
							if(game.P1) {
								game.P1->charact.scores += 200;
								if(c_score_max<game.P1->charact.scores) game.P1->charact.scores = c_score_max;
								player_class_init(game.P1);
							};
						};

						if(strcmp(codebuf,code_health) == 0) {
							game.P0->charact.health = game.P0->charact.healthmax;
							if(game.P1) game.P1->charact.health = game.P1->charact.healthmax;
						};

						if(strcmp(codebuf,code_armor) == 0) {
							game.P0->charact.armor = game.P0->charact.armormax;
							if(game.P1) game.P1->charact.armor = game.P1->charact.armormax;
						};

						if(strcmp(codebuf,code_ammo) == 0) {
							if(game.P0->w.ammo[1] != c_p_WEAP_notused)
								game.P0->w.ammo[1] = wtable[1].ammo;
							if(game.P0->w.ammo[2] != c_p_WEAP_notused)
								game.P0->w.ammo[2] = wtable[2].ammo;
							if(game.P1) {
								if(game.P1->w.ammo[1] != c_p_WEAP_notused)
									game.P1->w.ammo[1] = wtable[1].ammo;
								if(game.P1->w.ammo[2] != c_p_WEAP_notused)
									game.P1->w.ammo[2] = wtable[2].ammo;
							};
						};

						if(strcmp(codebuf,code_all) == 0) {
							game.P0->charact.health = game.P0->charact.healthmax;
							game.P0->charact.armor = game.P0->charact.armormax;
							if(game.P0->w.ammo[1] != c_p_WEAP_notused)
								game.P0->w.ammo[1] = wtable[1].ammo;
							if(game.P0->w.ammo[2] != c_p_WEAP_notused)
								game.P0->w.ammo[2] = wtable[2].ammo;
							if(game.P1) {
								game.P1->charact.health = game.P1->charact.healthmax;
								game.P1->charact.armor = game.P1->charact.armormax;
								if(game.P1->w.ammo[1] != c_p_WEAP_notused)
									game.P1->w.ammo[1] = wtable[1].ammo;
								if(game.P1->w.ammo[2] != c_p_WEAP_notused)
									game.P1->w.ammo[2] = wtable[2].ammo;
							};
						};

					}
					else {
						codebuf[0] = 0;
						codecount = 0;
					}
				}
			}
		}
	}
     */
}

/*
 * обработка игрока
 */
static void player_handle_common(void * player, player_vars_t * pl)
{
#define PLAYER_SOUND_MOVE 0
#define PLAYER_SOUND_ATTACK 1

    void player_handle_common_(entity_t * player, int info_item, INTEGER * item)
    {
        if(
                (info_item == PLAYER_ITEM_AMOUNT_INF)||
                (*item > 0)
        )
        {
            // пули не кончились
            pl->reloadtime_d = c_p_WEAP_reloadtime;
            //создаем пулю
            weaponinfo_t * weaponinfo = &weaponinfo_table[pl->weap];
            direction_t dir;
            if(pl->weap != WEAP_MINE)
                dir = pl->dir;
            else
                dir = entity_direction_invert(pl->dir);

            var_value_t vars_values[] =
            {
                    VAR_VALUE_VECTOR2("origin", pl->origin[0], pl->origin[1]),
                    VAR_VALUE_DIRECTION("dir", dir),
            };

            entity_new(weaponinfo->entityname, player, vars_values, ARRAYSIZE(vars_values));

            sound_play_start(player, PLAYER_SOUND_ATTACK, weaponinfo->sound_index, 1);

            if(
                    info_item > 0 && //если пули у оружия не бесконечны и
                    ENTITY_IS(player, "player")    // игрок не монстр(у монстров пули не кончаются)
            )
                (*item)--;
        }
    }

    enum
    {
        STATE_IDLE,
        STATE_RUN_BEGIN,
        STATE_RUN_END,
        STATE_RUN,
        STATE_DEAD
    };
    int state = STATE_IDLE;
    vec2_t Sorig;
    vec_t L,R,U,D;

    var_int_t level = pl->level;
    playerinfo_t *playerinfo = &playerinfo_table[level];

    pl->alive = ( pl->item_health > 0 );

    if(!pl->alive)
    {
        state = STATE_DEAD;
    }
    else
    {
        if(pl->move.go)
        {
            if(!pl->move.prev_go)
                state = STATE_RUN_BEGIN;
            else
                state = STATE_RUN;
        }
        else
        {
            if(pl->move.prev_go)
                state = STATE_RUN_END;
        }
        pl->move.prev_go = pl->move.go;
    }

    switch(state)
    {
    case STATE_IDLE: break;
    case STATE_RUN_BEGIN: entity_model_play_start(player, 0, "run"); break;
    case STATE_RUN_END  : entity_model_play_pause(player, 0); break;
    case STATE_RUN: break;
    case STATE_DEAD:

        sound_play_stop(player, -1);

        //если игрок мертв
        if(ENTITY_IS_SPAWNED(player))
        {
            var_value_t vars_values[] =
            {
                    VAR_VALUE_VECTOR2("origin", pl->origin[0], pl->origin[1]),
                    VAR_VALUE_DIRECTION("dir", pl->dir),
            };

            entity_new("explode_missile", player, vars_values, ARRAYSIZE(vars_values));
            ENTITY_UNSPAWN(player);

            if(!ENTITY_IS(player, "player"))
                ENTITY_HIDE(player);
            pl->item_armor = 0;
            pl->item_ammo_missile = 0;
            pl->item_ammo_mine = 0;
        };
        if(ENTITY_IS(player, "boss"))
            sv_game_win();
        break;
    }


    if(pl->alive)
    {
        //игрок жив
        if(pl->bull)
        {
            bull_vars_t * bull_vars = entity_vars(pl->bull);
            bull_vars->dir = pl->dir;
            pl->move.go = false;
        };
        if(pl->move.go)
        {
            //игрок едет
            pl->move.speed += PLAYER_ACCEL * dtime;
            if(pl->move.speed > playerinfo->speed) pl->move.speed = playerinfo->speed;
            if(!sound_started(player, PLAYER_SOUND_MOVE))
            {
                sound_play_start(player, PLAYER_SOUND_MOVE, SOUND_PLAYER_TANKMOVE, -1);
            }

        }
        else
        {
            //игрок останавливается
            pl->move.speed -= PLAYER_DECEL * dtime;
        };
        if(pl->move.speed < 0)
        {
            sound_play_stop(player, PLAYER_SOUND_MOVE);
            pl->move.speed = 0;
        }
        entity_move(player, pl->dir, pl->move.speed, true);

        vec_t speed_s = playerinfo->speed / 4;

        vec_t halfbox = ENTITY_HALFBODYBOX(player);
        vec_t quarterbox = halfbox * 0.5f;
        //стрейф
        switch(pl->dir)
        {
        case DIR_UP:
        case DIR_DOWN:
            VEC2_COPY(Sorig, pl->origin);
            Sorig[0] -= quarterbox;
            map_clip_find_near(Sorig, halfbox, pl->dir, MAP_WALL_CLIP, halfbox + 2, &L);
            VEC2_COPY(Sorig, pl->origin);
            Sorig[0] += quarterbox;
            map_clip_find_near(Sorig, halfbox, pl->dir, MAP_WALL_CLIP, halfbox + 2, &R);
            if((halfbox<L) && (R-1<=halfbox)) entity_move(player, DIR_LEFT, speed_s, true);//strafe left
            if((halfbox<R) && (L-1<=halfbox)) entity_move(player, DIR_RIGHT, speed_s, true);//strafe right
            break;
        case DIR_LEFT:
        case DIR_RIGHT:
            VEC2_COPY(Sorig, pl->origin);
            Sorig[1] -= quarterbox;
            map_clip_find_near(Sorig, halfbox, pl->dir, MAP_WALL_CLIP, halfbox + 2, &D);
            VEC2_COPY(Sorig, pl->origin);
            Sorig[1] += quarterbox;
            map_clip_find_near(Sorig, halfbox, pl->dir, MAP_WALL_CLIP, halfbox + 2, &U);
            if((halfbox < U)&&(D-1 <= halfbox)) entity_move(player, DIR_UP  , speed_s, true);//strafe up
            if((halfbox < D)&&(U-1 <= halfbox)) entity_move(player, DIR_DOWN, speed_s, true);//strafe down
            break;
        }
    }
    //стрельба
    if(!pl->attack)
    {
        //игрок не атакует
        if(pl->reloadtime_d>0) pl->reloadtime_d -= dtime;//учитываем время на перезарядку
    }
    else
    {
        //игрок атакует
        if(pl->alive)
        {
            if(pl->reloadtime_d > 0) pl->reloadtime_d -= dtime;//учитываем время на перезарядку
            else
            {
                if(!pl->bull)
                {

                    switch(pl->weap)
                    {
                    case WEAP_ARTILLERY:
                        player_handle_common_(player, playerinfo->items[ITEM_AMMO_ARTILLERY], &pl->item_ammo_artillery);
                        break;
                    case WEAP_MISSILE  :
                        player_handle_common_(player, playerinfo->items[ITEM_AMMO_MISSILE], &pl->item_ammo_missile  );
                        break;
                    case WEAP_MINE     :
                        player_handle_common_(player, playerinfo->items[ITEM_AMMO_MINE], &pl->item_ammo_mine);
                        break;
                    default: ;
                    }

                }
            }
        }
    }
    if(pl->reloadtime_d < 0) pl->reloadtime_d = 0;
}

/*
 * инициализация класса танка
 */
void player_class_init(void * player, player_vars_t * pl)
{
    void player_class_init_(void * player, INTEGER * dest, INTEGER src)
    {
        if(
                src == PLAYER_ITEM_AMOUNT_NA ||
                src == PLAYER_ITEM_AMOUNT_INF
        )
        {
            *dest = src;
            return;
        }
        if( src < 0 )
            *dest = 0;
        if( !ENTITY_IS(player, "player") )
            *dest = src;
    }

    int level = pl->scores / PLAYER_SCOREPERCLASS;
    if(level > PLAYER_LEVEL5) level = PLAYER_LEVEL5;
    if( ENTITY_IS(player, "boss") )
        level = PLAYER_LEVEL_BOSS;

    pl->level = level;

    playerinfo_t * playerinfo = &playerinfo_table[level];

    player_class_init_(player, &pl->scores     , playerinfo->items[ITEM_SCORES]);
    player_class_init_(player, &pl->item_health, playerinfo->items[ITEM_HEALTH]);
    player_class_init_(player, &pl->item_armor , playerinfo->items[ITEM_ARMOR]);
    player_class_init_(player, &pl->item_ammo_artillery, playerinfo->items[ITEM_AMMO_ARTILLERY]);
    player_class_init_(player, &pl->item_ammo_missile  , playerinfo->items[ITEM_AMMO_MISSILE]);
    player_class_init_(player, &pl->item_ammo_mine     , playerinfo->items[ITEM_AMMO_MINE]);

    entity_model_set(player, 0, playerinfo->modelname);

}

/*
 * вычисление повреждения наносимого игроку
 */
void player_getdamage(void * player, void * explode, bool self, vec_t distance, const explodeinfo_t * explodeinfo)
{
    player_vars_t * pl = ((entity_t*)player)->common;
    int damage_full;
    int armor;

    vec_t radius = ((entity_t*)explode)->info->bodybox * 0.5f;

    //weapon_info_t * weapinfo = &wtable[explode->explode.type];

    if( pl->item_health > 0 && distance <= radius )
    {
        if(self)
            damage_full = explodeinfo->selfdamage * (1-distance/radius);
        else
            damage_full = explodeinfo->damage     * (1-distance/radius);

        int damage_armor = damage_full*2/3;
        int damage_health = damage_full - damage_armor;

        armor = pl->item_armor - damage_armor;
        pl->item_health -= damage_health;
        if(armor < 0)
        {
            pl->item_health += armor;
            pl->item_armor = 0;
        }
        else
            pl->item_armor = armor;
        if(pl->item_health <= 0)
        {
            entity_t * killer = entity_parent(explode);
            if(killer)
            {
                player_vars_t * pl = killer->common;
                if(!self)
                {
                    //атакующему добавим очки
                    pl->fragstotal++;
                    pl->frags++;
                    pl->scores += PLAYER_SCORES_PER_ENEMY;
                }
                else
                {
                    //атакующий умер от своей пули
                    pl->fragstotal--;
                    pl->frags--;
                    pl->scores = 0;
                }
                player_class_init(killer, pl);
            }
        }
    }
}


/****************/

#include "_gr2D.h"
#include "video.h"

/*
 * вывод информации об игроке
 */
static void player_ui_draw(camera_t * cam, entity_t * player)
{
    player_vars_t * pl = player->common;
    static image_index_t list[] =
    {
            IMG_HUD_ICON_TANK0,
            IMG_HUD_ICON_TANK1,
            IMG_HUD_ICON_TANK2,
            IMG_HUD_ICON_TANK3,
            IMG_HUD_ICON_TANK4,
    };

    var_int_t level = pl->level;
    playerinfo_t *playerinfo = &playerinfo_table[level];

    int ref_y = VIDEO_SCREEN_H - 16 * 2;

    font_color_set3i(COLOR_1);
    // gr2D_settext(cam->x,cam_y,0,'('+realtostr(player->move.orig.x,8,4)+';'+realtostr(player->move.orig.y,8,4)+')');
    // gr2D_settext(cam->x,cam_y,0,"PING %d", player->time.delta);

    ui_drawimage(cam, 16 * 0     , ref_y, IMG_ITEM_HEALTH);
    ui_drawimage(cam, 16 * 6     , ref_y, IMG_ITEM_ARMOR);

    ui_printf(cam, 16 * 0 + 16, ref_y, "%ld", pl->item_health);
    ui_printf(cam, 16 * 6 + 16, ref_y, "%ld", pl->item_armor);
    ui_printf(cam, 16 * 0 + 16, ref_y + 8, "%d", playerinfo->items[ITEM_HEALTH]);
    ui_printf(cam, 16 * 6 + 16, ref_y + 8, "%d", playerinfo->items[ITEM_ARMOR]);

    /* вторая строка */
    ref_y += 16;
    ui_drawimage(cam, 16 * 0, ref_y, list[level]);
    ui_drawimage(cam, 16 * 4, ref_y, weaponinfo_table[0].icon);
    ui_drawimage(cam, 16 * 6, ref_y, weaponinfo_table[1].icon);
    ui_drawimage(cam, 16 * 8, ref_y, weaponinfo_table[2].icon);

    ui_printf(cam, 16 * 4 + 16, ref_y + 4, "@"); // player->items[ITEM_AMMO_ARTILLERY]
    if(pl->item_ammo_missile >= 0)
        ui_printf(cam, 16 * 6 + 16, ref_y + 4, "%ld", pl->item_ammo_missile);
    if(pl->item_ammo_mine >= 0)
        ui_printf(cam, 16 * 8 + 16, ref_y + 4, "%ld", pl->item_ammo_mine);
    ui_printf(cam, 16 * 0 + 16, ref_y +  4, "%ld", pl->scores);

}



static const entityinfo_t player_reginfo = {
        .name = "player",
        .flags = ENTITYFLAG_SOLIDWALL,
        .bodybox = 16,
        ENTITYINFO_VARS(player_vars_t, player_vars),
        ENTITYINFO_ENTMODELS(tank_player_models),
        .init = player_init,
        .done = player_done,
        .spawn = player_spawn,
        .handle = player_handle,
        ENTITYINFO_TOUCHS(player_touchs),
        ENTITYINFO_ACTIONS(player_actions),
};

static const entityinfo_t enemy_reginfo = {
        .name = "enemy",
        .flags = ENTITYFLAG_SOLIDWALL,
        .bodybox = 16,
        ENTITYINFO_VARS(player_vars_t, player_vars),
        ENTITYINFO_ENTMODELS(tank_enemy_models),
        .init = enemy_init,
        .done = enemy_done,
        .handle = enemy_handle,
};

static const entityinfo_t boss_reginfo = {
        .name = "boss",
        .flags = ENTITYFLAG_SOLIDWALL,
        .bodybox = 16,
        ENTITYINFO_VARS(player_vars_t, player_vars),
        ENTITYINFO_ENTMODELS(tank_boss_models),
        .init = boss_init,
        .done = boss_done,
        .handle = boss_handle,
};

void entity_player_init(void)
{
    entity_register(&player_reginfo);
    entity_register(&enemy_reginfo);
    entity_register(&boss_reginfo);
    ui_register(player_ui_draw);
}
