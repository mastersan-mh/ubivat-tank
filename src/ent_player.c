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
void player_spawn_init(entity_t * player, player_t * pl, const entity_t * spawn)
{
	if(!spawn)
		game_halt("Error: Player spawn is NULL, can not create player.");

	VEC2_COPY(spawn->origin, player->origin);

	var_int_t spawn_scores = ENTITY_VARIABLE_INTEGER(spawn, "item_scores");
	if(0 <= spawn_scores && spawn_scores <= PLAYER_SCORES_MAX)
		ENTITY_VARIABLE_INTEGER(player, "scores") = spawn_scores;

	player_class_init(player, pl);

	var_int_t level = ENTITY_VARIABLE_INTEGER(player, "level");
	playerinfo_t * playerinfo = &playerinfo_table[level];

	static const struct {
		char * spawnvarname;
		char * playervarname;
		player_invitemtype_t item;
	}
	spawn_itemvars[] =
	{
			{ "item_health"        , "item_health"      , ITEM_HEALTH},
			{ "item_armor"         , "item_armor"       , ITEM_ARMOR},
			{ "item_ammo_missile"  , "item_ammo_missile", ITEM_AMMO_MISSILE},
			{ "item_ammo_mine"     , "item_ammo_mine"   , ITEM_AMMO_MINE},
	};

#define SPAWNVAR(i) ENTITY_VARIABLE_INTEGER(spawn, spawn_itemvars[i].spawnvarname)
#define PLAYERVAR(i) ENTITY_VARIABLE_INTEGER(player, spawn_itemvars[i].playervarname)
#define PLAYERINFOVAR(i) playerinfo->items[spawn_itemvars[i].item]

	var_int_t player_health = ENTITY_VARIABLE_INTEGER(player, "item_health");
	if(player_health < 0)
	{
		player_health = ENTITY_VARIABLE_INTEGER(spawn, "item_health");
		if(player_health < 0)
			player_health = PLAYERINFOVAR(0);
		ENTITY_VARIABLE_INTEGER(player, "item_health") = player_health;
	}

/* инициализация инвентаря */
	size_t i;
	for(i = 0; i < ARRAYSIZE(spawn_itemvars); i++)
	{
		var_int_t spawn_value = SPAWNVAR(i);
		if( spawn_value > PLAYERINFOVAR(i) )
			spawn_value = PLAYERINFOVAR(i);
		else if(spawn_value <=0)
		{
			if( !ENTITY_IS(player, "player") ) /* враг/босс */
				spawn_value = PLAYERINFOVAR(i);
			else
			{
				/* игрок-человек */
				if(sv_game_is_custom_game())
					spawn_value = PLAYERINFOVAR(i);
				else
				{
					//по уровням
					if(sv_game_is_first_map()) // первая карта
						spawn_value = PLAYERINFOVAR(i);
					else // не первая карта
						spawn_value = PLAYERVAR(i);
				}
			}
		}
		PLAYERVAR(i) = spawn_value;
	}

	ENTITY_VARIABLE_INTEGER(player, "frags") = 0;
	pl->bull         = NULL;
	pl->move.speed   = 0;
	pl->move.go      = false;
	pl->attack       = false;
	pl->reloadtime_d = 0;
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

#define tank_common_modelaction_endframef entity_model_play_start

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
	player_spawn_init(this, thisdata, spawn);
}

static ENTITY_FUNCTION_HANDLE(player_handle);

static void player_restore(entity_t * this, void * thisdata, const void * userstoredata)
{
	player_class_init(this, thisdata);
}


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
	//отправим сообщение игроку
	sv_game_message_send(ENTITY_VARIABLE_STRING(exposed, "text"));
}

static void player_touch_exit(entity_t * actor, entity_t * exposed)
{
	//отправим сообщение игроку
	sv_game_message_send(ENTITY_VARIABLE_STRING(exposed, "text"));
	sv_game_win();
}

static void player_touch_item_scores(entity_t * player, entity_t * item)
{
	var_int_t level = ENTITY_VARIABLE_INTEGER(player, "level");
	playerinfo_t *playerinfo = &playerinfo_table[level];

	ENTITY_UNSPAWN(item);

	var_int_t scores = ( ENTITY_VARIABLE_INTEGER(player, "scores") += ENTITY_VARIABLE_INTEGER(item, "amount") );

	player_class_init(player, player->data);
	if(scores / PLAYER_SCOREPERCLASS >= 5)
	{
		if(ENTITY_VARIABLE_INTEGER(player, "item_health") < playerinfo->items[ITEM_HEALTH])
			ENTITY_VARIABLE_INTEGER(player, "item_health") = playerinfo->items[ITEM_HEALTH];
		if(ENTITY_VARIABLE_INTEGER(player, "item_armor") < playerinfo->items[ITEM_ARMOR])
			ENTITY_VARIABLE_INTEGER(player, "item_armor") = playerinfo->items[ITEM_ARMOR];
	}
}


static void player_touch_item(entity_t * player, entity_t * item)
{
	static const char *list[] =
	{
			"scores",
			"item_health",
			"item_armor",
			"item_ammo_artillery",
			"item_ammo_missile",
			"item_ammo_mine",
	};

	player_invitemtype_t invitemtype = player_entity_to_itemtype(item);

	var_int_t level = ENTITY_VARIABLE_INTEGER(player, "level");
	playerinfo_t *playerinfo = &playerinfo_table[level];

	int itemamount = ENTITY_VARIABLE_INTEGER(item, "amount");
	int player_itemamount;

	if(
			playerinfo->items[invitemtype] > 0 &&
			(
					( (player_itemamount = ENTITY_VARIABLE_INTEGER(player, list[invitemtype]) ) < playerinfo->items[invitemtype] ) ||
					itemamount < 0
			)
	)
	{
		ENTITY_UNSPAWN(item);
		player_itemamount += itemamount;
		coerce_value_int(&player_itemamount, 0, playerinfo->items[invitemtype]);
		ENTITY_VARIABLE_INTEGER(player, list[invitemtype]) = player_itemamount;
	}
}

static void player_action_move(entity_t * this, player_t * pl, direction_t dir, bool go)
{
	if(go)
		this->dir = dir;
	else
	{
		if(this->dir != dir)
			return;
	}
	pl->move.go = go;
}

static void player_action_attack(entity_t * this, player_t * pl, bool attack, weapontype_t weap)
{
	if(attack)
		pl->weap = weap;
	pl->attack = attack;
}


ENTITY_FUNCTION_ACTION(player_action_move_north_on ) { player_action_move(this, thisdata, DIR_UP   , true ); }
ENTITY_FUNCTION_ACTION(player_action_move_north_off) { player_action_move(this, thisdata, DIR_UP   , false); }
ENTITY_FUNCTION_ACTION(player_action_move_south_on ) { player_action_move(this, thisdata, DIR_DOWN , true ); }
ENTITY_FUNCTION_ACTION(player_action_move_south_off) { player_action_move(this, thisdata, DIR_DOWN , false); }
ENTITY_FUNCTION_ACTION(player_action_move_west_on  ) { player_action_move(this, thisdata, DIR_LEFT , true ); }
ENTITY_FUNCTION_ACTION(player_action_move_west_off ) { player_action_move(this, thisdata, DIR_LEFT , false); }
ENTITY_FUNCTION_ACTION(player_action_move_east_on  ) { player_action_move(this, thisdata, DIR_RIGHT, true ); }
ENTITY_FUNCTION_ACTION(player_action_move_east_off ) { player_action_move(this, thisdata, DIR_RIGHT, false); }

ENTITY_FUNCTION_ACTION(player_attack_weapon1_on ) { player_action_attack(this, thisdata, true , WEAP_ARTILLERY); }
ENTITY_FUNCTION_ACTION(player_attack_weapon1_off) { player_action_attack(this, thisdata, false, WEAP_ARTILLERY); }
ENTITY_FUNCTION_ACTION(player_attack_weapon2_on ) { player_action_attack(this, thisdata, true , WEAP_MISSILE); }
ENTITY_FUNCTION_ACTION(player_attack_weapon2_off) { player_action_attack(this, thisdata, false, WEAP_MISSILE); }
ENTITY_FUNCTION_ACTION(player_attack_weapon3_on ) { player_action_attack(this, thisdata, true , WEAP_MINE); }
ENTITY_FUNCTION_ACTION(player_attack_weapon3_off) { player_action_attack(this, thisdata, false, WEAP_MINE); }
ENTITY_FUNCTION_ACTION(player_win) { sv_game_win(); }


static entityvarinfo_t player_vars[] =
{
		{ "fragstotal", VARTYPE_INTEGER }, /* фрагов за пройденые карты */
		{ "frags"     , VARTYPE_INTEGER }, /* фрагов за карту */
		{ "scores"    , VARTYPE_INTEGER }, /* набрано очков */
		{ "level"     , VARTYPE_INTEGER }, /* уровень игрока */

		{ "item_health"        , VARTYPE_INTEGER },
		{ "item_armor"         , VARTYPE_INTEGER },
		{ "item_ammo_artillery", VARTYPE_INTEGER },
		{ "item_ammo_missile"  , VARTYPE_INTEGER },
		{ "item_ammo_mine"     , VARTYPE_INTEGER },
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

static void player_handle_common(entity_t * player, player_t * pl);

ENTITY_FUNCTION_INIT(player_init)
{
	player_t * pl = thisdata;

#if defined(_DEBUG_PLAYERMAXLEVEL)
	ENTITY_VARIABLE_INTEGER(this, "scores") = 9000;
	ENTITY_VARIABLE_INTEGER(this, "item_health") = 100;
	ENTITY_VARIABLE_INTEGER(this, "item_armor")  = 100;
	ENTITY_VARIABLE_INTEGER(this, "item_ammo_missile") = 100;
	ENTITY_VARIABLE_INTEGER(this, "item_ammo_mine")    = 100;
#endif

	player_spawn_init(this, pl, parent);// parent = spawn

}

ENTITY_FUNCTION_DONE(player_done)
{
	player_t * pl = thisdata;
	sound_play_stop(this, -1);
	ctrl_AI_done(&(pl->brain));
}

ENTITY_FUNCTION_HANDLE(player_handle)
{
	player_handle_common(this, thisdata);
}

ENTITY_FUNCTION_INIT(enemy_init)
{
	player_t * pl = thisdata;
	player_spawn_init(this, pl, parent);
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
	player_t * pl = thisdata;
	player_spawn_init(this, pl, parent);
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
static void player_handle_common(entity_t * player, player_t * pl)
{
#define PLAYER_SOUND_MOVE 0
#define PLAYER_SOUND_ATTACK 1
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

	var_int_t level = ENTITY_VARIABLE_INTEGER(player, "level");
	playerinfo_t *playerinfo = &playerinfo_table[level];

	player->alive = ( ENTITY_VARIABLE_INTEGER(player, "item_health") > 0 );

	if(!player->alive)
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
				entity_new(
					"explode_missile",
					player->origin[0],
					player->origin[1],
					player->dir,
					player
				);
				ENTITY_UNSPAWN(player);

				if(!ENTITY_IS(player, "player"))
					ENTITY_ALLOW_DRAW_SET(player, false);
				ENTITY_VARIABLE_INTEGER(player, "item_armor") = 0;
				ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") = 0;
				ENTITY_VARIABLE_INTEGER(player, "item_ammo_mine") = 0;
			};
			if(ENTITY_IS(player, "boss"))
				sv_game_win();
			break;
	}


	if(player->alive)
	{
		//игрок жив
		if(pl->bull)
		{
			pl->bull->dir = player->dir;
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
		entity_move(player, player->dir, pl->move.speed, true);

		vec_t speed_s = playerinfo->speed / 4;

		vec_t halfbox = ENTITY_HALFBODYBOX(player);
		vec_t quarterbox = halfbox * 0.5f;
		//стрейф
		switch(player->dir)
		{
		case DIR_UP:
		case DIR_DOWN:
			VEC2_COPY(player->origin, Sorig);
			Sorig[0] -= quarterbox;
			map_clip_find_near(Sorig, halfbox, player->dir, MAP_WALL_CLIP, halfbox + 2, &L);
			VEC2_COPY(player->origin, Sorig);
			Sorig[0] += quarterbox;
			map_clip_find_near(Sorig, halfbox, player->dir, MAP_WALL_CLIP, halfbox + 2, &R);
			if((halfbox<L) && (R-1<=halfbox)) entity_move(player, DIR_LEFT, speed_s, true);//strafe left
			if((halfbox<R) && (L-1<=halfbox)) entity_move(player, DIR_RIGHT, speed_s, true);//strafe right
			break;
		case DIR_LEFT:
		case DIR_RIGHT:
			VEC2_COPY(player->origin, Sorig);
			Sorig[1] -= quarterbox;
			map_clip_find_near(Sorig, halfbox, player->dir, MAP_WALL_CLIP, halfbox + 2, &D);
			VEC2_COPY(player->origin, Sorig);
			Sorig[1] += quarterbox;
			map_clip_find_near(Sorig, halfbox, player->dir, MAP_WALL_CLIP, halfbox + 2, &U);
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
		if(player->alive)
		{
			if(pl->reloadtime_d > 0) pl->reloadtime_d -= dtime;//учитываем время на перезарядку
			else
			{
				if(!pl->bull)
				{
					static const char *list[] =
					{
							"scores",
							"item_health",
							"item_armor",
							"item_ammo_artillery",
							"item_ammo_missile",
							"item_ammo_mine",
					};

					size_t item;
					switch(pl->weap)
					{
					case WEAP_ARTILLERY: item = ITEM_AMMO_ARTILLERY; break;
					case WEAP_MISSILE  : item = ITEM_AMMO_MISSILE; break;
					case WEAP_MINE     : item = ITEM_AMMO_MINE; break;
					default: item = ITEM_AMMO_ARTILLERY;
					}

					if(
							(playerinfo->items[item] == PLAYER_ITEM_AMOUNT_INF)||            //если пуль бесконечно много
							(ENTITY_VARIABLE_INTEGER(player, list[item]) > 0)
					)
					{
						// пули не кончились
						pl->reloadtime_d = c_p_WEAP_reloadtime;
						//создаем пулю
						weaponinfo_t * weaponinfo = &weaponinfo_table[pl->weap];
						direction_t dir;
						if(pl->weap != WEAP_MINE)
							dir = player->dir;
						else
							dir = entity_direction_invert(player->dir);
						entity_new(
							weaponinfo->entityname,
							player->origin[0],
							player->origin[1],
							dir,
							player
						);
						sound_play_start(player, PLAYER_SOUND_ATTACK, weaponinfo->sound_index, 1);

						if(
								playerinfo->items[item] > 0 && //если пули у оружия не бесконечны и
								ENTITY_IS(player, "player")    // игрок не монстр(у монстров пули не кончаются)
						)
							ENTITY_VARIABLE_INTEGER(player, list[item])--;
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
void player_class_init(entity_t * player, player_t * pl)
{
	int level = ENTITY_VARIABLE_INTEGER(player, "scores") / PLAYER_SCOREPERCLASS;
	if(level > PLAYER_LEVEL5) level = PLAYER_LEVEL5;
	if( ENTITY_IS(player, "boss") )
		level = PLAYER_LEVEL_BOSS;

	ENTITY_VARIABLE_INTEGER(player, "level") = level;

	playerinfo_t * playerinfo = &playerinfo_table[level];

	size_t i;
	for(i = 0; i < ITEM_NUM; i++)
	{
		static const char *list[] =
		{
				"scores",
				"item_health",
				"item_armor",
				"item_ammo_artillery",
				"item_ammo_missile",
				"item_ammo_mine",
		};

		if(
				playerinfo->items[i] == PLAYER_ITEM_AMOUNT_NA ||
				playerinfo->items[i] == PLAYER_ITEM_AMOUNT_INF
		)
			ENTITY_VARIABLE_INTEGER(player, list[i]) = playerinfo->items[i];
		else
		{
			if( playerinfo->items[i] < 0 )
				ENTITY_VARIABLE_INTEGER(player, list[i]) = 0;
			if( !ENTITY_IS(player, "player") )
			{
				ENTITY_VARIABLE_INTEGER(player, list[i]) = playerinfo->items[i];
			}
		}
	}

	entity_model_set(player, 0, playerinfo->modelname);

}

/*
 * вычисление повреждения наносимого игроку
 */
void player_getdamage(entity_t * player, entity_t * explode, bool self, vec_t distance, const explodeinfo_t * explodeinfo)
{
	int damage_full;
	int armor;

	vec_t radius = explode->info->bodybox * 0.5f;

	//weapon_info_t * weapinfo = &wtable[explode->explode.type];

	if( ENTITY_VARIABLE_INTEGER(player, "item_health") > 0 && distance <= radius )
	{
		if(self)
			damage_full = explodeinfo->selfdamage * (1-distance/radius);
		else
			damage_full = explodeinfo->damage     * (1-distance/radius);

		int damage_armor = damage_full*2/3;
		int damage_health = damage_full - damage_armor;

		armor = ENTITY_VARIABLE_INTEGER(player, "item_armor") - damage_armor;
		ENTITY_VARIABLE_INTEGER(player, "item_health") -= damage_health;
		if(armor < 0)
		{
			ENTITY_VARIABLE_INTEGER(player, "item_health") += armor;
			ENTITY_VARIABLE_INTEGER(player, "item_armor") = 0;
		}
		else
			ENTITY_VARIABLE_INTEGER(player, "item_armor") = armor;
		if(ENTITY_VARIABLE_INTEGER(player, "item_health") <= 0)
		{
			entity_t * killer = explode->parent;
			if(killer)
			{
				player_t * pl = killer->data;
				if(!self)
				{
					//атакующему добавим очки
					ENTITY_VARIABLE_INTEGER(killer, "fragstotal")++;
					ENTITY_VARIABLE_INTEGER(killer, "frags")++;
					ENTITY_VARIABLE_INTEGER(killer, "scores") += PLAYER_SCORES_PER_ENEMY;
				}
				else
				{
					//атакующий умер от своей пули
					ENTITY_VARIABLE_INTEGER(killer, "fragstotal")--;
					ENTITY_VARIABLE_INTEGER(killer, "frags")--;
					ENTITY_VARIABLE_INTEGER(killer, "scores") = 0;
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
	static image_index_t list[] =
	{
			IMG_HUD_ICON_TANK0,
			IMG_HUD_ICON_TANK1,
			IMG_HUD_ICON_TANK2,
			IMG_HUD_ICON_TANK3,
			IMG_HUD_ICON_TANK4,
	};

	var_int_t level = ENTITY_VARIABLE_INTEGER(player, "level");
	playerinfo_t *playerinfo = &playerinfo_table[level];

	int ref_y = VIDEO_SCREEN_H - 16 * 2;

	font_color_set3i(COLOR_1);
	// gr2D_settext(cam->x,cam_y,0,'('+realtostr(player->move.orig.x,8,4)+';'+realtostr(player->move.orig.y,8,4)+')');
	// gr2D_settext(cam->x,cam_y,0,"PING %d", player->time.delta);

	ui_drawimage(cam, 16 * 0     , ref_y, image_get(IMG_ITEM_HEALTH));
	ui_drawimage(cam, 16 * 6     , ref_y, image_get(IMG_ITEM_ARMOR));

	ui_printf(cam, 16 * 0 + 16, ref_y, "%ld", ENTITY_VARIABLE_INTEGER(player, "item_health"));
	ui_printf(cam, 16 * 6 + 16, ref_y, "%ld", ENTITY_VARIABLE_INTEGER(player, "item_armor"));
	ui_printf(cam, 16 * 0 + 16, ref_y + 8, "%d", playerinfo->items[ITEM_HEALTH]);
	ui_printf(cam, 16 * 6 + 16, ref_y + 8, "%d", playerinfo->items[ITEM_ARMOR]);

	/* вторая строка */
	ref_y += 16;
	ui_drawimage(cam, 16 * 0, ref_y, image_get( list[ level ] ));
	ui_drawimage(cam, 16 * 4, ref_y, image_get( weaponinfo_table[0].icon ));
	ui_drawimage(cam, 16 * 6, ref_y, image_get( weaponinfo_table[1].icon ));
	ui_drawimage(cam, 16 * 8, ref_y, image_get( weaponinfo_table[2].icon ));

	ui_printf(cam, 16 * 4 + 16, ref_y + 4, "@"); // player->items[ITEM_AMMO_ARTILLERY]
	if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") >= 0)
		ui_printf(cam, 16 * 6 + 16, ref_y + 4, "%ld", ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile"));
	if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_mine") >= 0)
		ui_printf(cam, 16 * 8 + 16, ref_y + 4, "%ld", ENTITY_VARIABLE_INTEGER(player, "item_ammo_mine"));
	ui_printf(cam, 16 * 0 + 16, ref_y +  4, "%ld", ENTITY_VARIABLE_INTEGER(player, "scores"));

}



static const entityinfo_t player_reginfo = {
		.name = "player",
		.datasize = sizeof(player_t),
		.flags = ENTITYFLAG_SOLIDWALL,
		.bodybox = 16,
		ENTITYINFO_VARS(player_vars),
		ENTITYINFO_ENTMODELS(tank_player_models),
		.init = player_init,
		.done = player_done,
		.spawn = player_spawn,
		.handle = player_handle,
		.client_store = NULL,
		.client_restore = player_restore,
		ENTITYINFO_TOUCHS(player_touchs),
		ENTITYINFO_ACTIONS(player_actions),
};

static const entityinfo_t enemy_reginfo = {
		.name = "enemy",
		.datasize = sizeof(player_t),
		.flags = ENTITYFLAG_SOLIDWALL,
		.bodybox = 16,
		ENTITYINFO_VARS(player_vars),
		ENTITYINFO_ENTMODELS(tank_enemy_models),
		.init = enemy_init,
		.done = enemy_done,
		.handle = enemy_handle,
		.client_store = NULL,
		.client_restore = NULL,
};

static const entityinfo_t boss_reginfo = {
		.name = "boss",
		.datasize = sizeof(player_t),
		.flags = ENTITYFLAG_SOLIDWALL,
		.bodybox = 16,
		ENTITYINFO_VARS(player_vars),
		ENTITYINFO_ENTMODELS(tank_boss_models),
		.init = boss_init,
		.done = boss_done,
		.handle = boss_handle,
		.client_store = NULL,
		.client_restore = NULL,
};

void entity_player_init(void)
{
	entity_register(&player_reginfo);
	entity_register(&enemy_reginfo);
	entity_register(&boss_reginfo);
	ui_register(player_ui_draw);
}
