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
#include "client.h"

/*
 * инициализируем игрока при спавне
 */
void player_spawn_init(entity_t * player, player_t * pl, const entity_t * spawn)
{
	if(!spawn)
		game_halt("Error: Player spawn is NULL, can not create player.");
	spawn_t * sp = spawn->data;

	player->pos.x = spawn->pos.x;
	player->pos.y = spawn->pos.y;

	if(0 <= sp->items[ITEM_SCORES] && sp->items[ITEM_SCORES] <= PLAYER_SCORES_MAX)
		ENTITY_VARIABLE_INTEGER(player, "scores") = sp->items[ITEM_SCORES];
	player_class_init(player, pl);
	entity_int_t level = ENTITY_VARIABLE_INTEGER(player, "level");
	playerinfo_t * playerinfo = &playerinfo_table[level];

	if(0 <= sp->items[ITEM_HEALTH] && sp->items[ITEM_HEALTH] <= playerinfo->items[ITEM_HEALTH])
		ENTITY_VARIABLE_INTEGER(player, "item_health") = sp->items[ITEM_HEALTH];
	else
	{
		if( !ENTITY_IS(player, "player") )
			ENTITY_VARIABLE_INTEGER(player, "item_health") = playerinfo->items[ITEM_HEALTH];
		else
		{
			if(sv_game_is_custom_game())
				ENTITY_VARIABLE_INTEGER(player, "item_health") = playerinfo->items[ITEM_HEALTH];
			else
			{
				//по уровням
				if(sv_game_is_first_map()) // первая карта
					ENTITY_VARIABLE_INTEGER(player, "item_health") = playerinfo->items[ITEM_HEALTH];
				else // не первая карта
					if(ENTITY_VARIABLE_INTEGER(player, "item_health") <= 0)
						ENTITY_VARIABLE_INTEGER(player, "item_health") = playerinfo->items[ITEM_HEALTH];
			}
		}
	}
	if(0 <= sp->items[ITEM_ARMOR] && sp->items[ITEM_ARMOR] <= playerinfo->items[ITEM_ARMOR] )
		ENTITY_VARIABLE_INTEGER(player, "item_armor") = sp->items[ITEM_ARMOR];
	else
	{
		if( ENTITY_IS(player, "player") )
			ENTITY_VARIABLE_INTEGER(player, "item_armor") = playerinfo->items[ITEM_ARMOR];
		else
			if(sv_game_is_custom_game())
				ENTITY_VARIABLE_INTEGER(player, "item_armor") = playerinfo->items[ITEM_ARMOR];
	};
	ENTITY_VARIABLE_INTEGER(player, "frags") = 0;
	pl->bull          = NULL;
	pl->move.speed    = 0;
	pl->move.go       = false;
	pl->attack        = false;
	pl->reloadtime_d  = 0;
	pl->soundId_move  = 0;
};


playerinfo_t playerinfo_table[__PLAYER_LEVEL_NUM] =
{
		/* SCORES                 , HEALTH, ARMOR,  AMMO_ARTILLERY,   AMMO_MISSILE,     AMMO_MINE,     speed,            modelname */
		{ { ITEM_SCOREPERCLASS * 1,    100,     0, ITEM_AMOUNT_INF, ITEM_AMOUNT_NA, ITEM_AMOUNT_NA }, 40/2 * SPEEDSCALE, "tank1"},
		{ { ITEM_SCOREPERCLASS * 2,    100,    50, ITEM_AMOUNT_INF, ITEM_AMOUNT_NA, ITEM_AMOUNT_NA }, 50/2 * SPEEDSCALE, "tank2"},
		{ { ITEM_SCOREPERCLASS * 3,    100,   100, ITEM_AMOUNT_INF, 50            , ITEM_AMOUNT_NA }, 60/2 * SPEEDSCALE, "tank3"},
		{ { ITEM_SCOREPERCLASS * 4,    200,   150, ITEM_AMOUNT_INF, 50            , ITEM_AMOUNT_NA }, 70/2 * SPEEDSCALE, "tank4"},
		{ { ITEM_SCOREPERCLASS * 5,    200,   200, ITEM_AMOUNT_INF, 50            , 50             }, 90/2 * SPEEDSCALE, "tank5"},
		{ { ITEM_SCOREPERCLASS * 6,   5000,  5000, ITEM_AMOUNT_INF, 50            , 50             }, 90/2 * SPEEDSCALE, "tank6"}  /* BOSS */
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

static entmodel_t tank_player_models[] =
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

static entmodel_t tank_enemy_models[] =
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

static entmodel_t tank_boss_models[] =
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
		{ "fragstotal", ENTITYVARTYPE_INTEGER }, /* фрагов за пройденые карты */
		{ "frags"     , ENTITYVARTYPE_INTEGER }, /* фрагов за карту */
		{ "scores"    , ENTITYVARTYPE_INTEGER }, /* набрано очков */
		{ "level"     , ENTITYVARTYPE_INTEGER }, /* уровень игрока */

		{ "item_health"        , ENTITYVARTYPE_INTEGER },
		{ "item_armor"         , ENTITYVARTYPE_INTEGER },
		{ "item_ammo_artillery", ENTITYVARTYPE_INTEGER },
		{ "item_ammo_missile"  , ENTITYVARTYPE_INTEGER },
		{ "item_ammo_mine"     , ENTITYVARTYPE_INTEGER },
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
	sound_play_stop(pl->soundId_move);
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
	//think_enemy(player);
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

void coerce_value_int(int * val, int min, int max)
{
	if(*val > max) *val = max;
	else if(*val < min) *val = min;
}

static void player_influence_message(entity_t * actor, entity_t * exposed)
{
	//отправим сообщение игроку
	sv_game_message_send(ENTITY_VARIABLE_STRING(exposed, "message"));
}

static void player_influence_exit(entity_t * actor, entity_t * exposed)
{
	//отправим сообщение игроку
	sv_game_message_send(ENTITY_VARIABLE_STRING(exposed, "message"));
	sv_game_win();
}

static void player_pickup_item_scores(entity_t * player, entity_t * entity)
{
	item_t * item = entity->data;
	if(!item->exist)
		return;
	entity_int_t level = ENTITY_VARIABLE_INTEGER(player, "level");
	playerinfo_t *playerinfo = &playerinfo_table[level];


	item->exist = false;
	entity_int_t scores = ( ENTITY_VARIABLE_INTEGER(player, "scores") += item->amount );

	player_class_init(player, player->data);
	if(scores / ITEM_SCOREPERCLASS >= 5)
	{
		if(ENTITY_VARIABLE_INTEGER(player, "item_health") < playerinfo->items[ITEM_HEALTH])
			ENTITY_VARIABLE_INTEGER(player, "item_health") = playerinfo->items[ITEM_HEALTH];
		if(ENTITY_VARIABLE_INTEGER(player, "item_armor") < playerinfo->items[ITEM_ARMOR])
			ENTITY_VARIABLE_INTEGER(player, "item_armor") = playerinfo->items[ITEM_ARMOR];
	}
}

static void player_influence_item(entity_t * player, entity_t * entity)
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

	itemtype_t itemtype = items_enttype_to_itemtype(entity->info->name);
	if((int) itemtype < 0)
	{
		game_halt("player_items_get(): invalid itemtype = %d\n", itemtype);
	}
	item_t * item = entity->data;
	if(!item->exist)
		return;
	entity_int_t level = ENTITY_VARIABLE_INTEGER(player, "level");
	playerinfo_t *playerinfo = &playerinfo_table[level];

	if(
			playerinfo->items[itemtype] != ITEM_AMOUNT_INF &&
			playerinfo->items[itemtype] != ITEM_AMOUNT_NA &&
			(
			((ENTITY_VARIABLE_INTEGER(player, list[itemtype]) < playerinfo->items[itemtype]) || item->amount < 0)
			)
	)
	{
		item->exist = false;
		int itemamount = (ENTITY_VARIABLE_INTEGER(player, list[itemtype]) += item->amount);

		coerce_value_int(&itemamount, 0, playerinfo->items[itemtype]);
		ENTITY_VARIABLE_INTEGER(player, list[itemtype]) = itemamount;
	};
}

bool inbox(entity_t * entity, entity_t * player)
{
	return
			( entity->pos.x - c_item_MDL_box / 2 <= player->pos.x + c_p_MDL_box    / 2 ) &&
			( player->pos.x - c_p_MDL_box    / 2 <= entity->pos.x + c_item_MDL_box / 2 ) &&
			( entity->pos.y - c_item_MDL_box / 2 <= player->pos.y + c_p_MDL_box    / 2 ) &&
			( player->pos.y - c_p_MDL_box    / 2 <= entity->pos.y + c_item_MDL_box / 2 )
			;
}

/*
 * подбирание предметов игроком
 */
static void player_pickup(entity_t * player)
{

	if(!player->alive)
		return;

	entity_t * entity;

	ENTITIES_FOREACH("item_scores", entity)
	{
		if( inbox(entity, player) )
			player_pickup_item_scores(player, entity);
	}

	ENTITIES_FOREACH("item_health", entity)
	{
		if( inbox(entity, player) )
			player_influence_item(player, entity);
	}

	ENTITIES_FOREACH("item_armor", entity)
	{
		if( inbox(entity, player) )
			player_influence_item(player, entity);
	}

	ENTITIES_FOREACH("item_ammo_missile", entity)
	{
		if( inbox(entity, player) )
			player_influence_item(player, entity);
	}

	ENTITIES_FOREACH("item_ammo_mine", entity)
	{
		if( inbox(entity, player) )
			player_influence_item(player, entity);
	}

	ENTITIES_FOREACH("message", entity)
	{
		if( inbox(entity, player) )
			player_influence_message(player, entity);
	}

	ENTITIES_FOREACH("exit", entity)
	{
		if( inbox(entity, player) )
			player_influence_exit(player, entity);
	}

}

/*
 * передвижение игрока
 */
static void player_move(entity_t * player, int dir, vec_t * speed)
{
	vec2_t * pos = &player->pos;
	vec_t dway = (*speed) * dtimed1000;
	vec_t halfbox = c_p_MDL_box/2;
	vec_t dist;

	map_clip_find_near(pos, c_p_MDL_box, dir, 0xF0, c_p_MDL_box, &dist);//найдем препятствия
	if(dist < dway + halfbox) dway = dist - halfbox;

	switch(dir)
	{
	case DIR_UP   : pos->y += dway; break;
	case DIR_DOWN : pos->y -= dway; break;
	case DIR_LEFT : pos->x -= dway; break;
	case DIR_RIGHT: pos->x += dway; break;
	}
}

/*
 * обработка игрока
 */
static void player_handle_common(entity_t * player, player_t * pl)
{
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
	vec_t speed_s;

	entity_int_t level = ENTITY_VARIABLE_INTEGER(player, "level");
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

			if(pl->soundId_move)
			{
				sound_play_stop(pl->soundId_move);
				pl->soundId_move = 0;
			}

			//если игрок мертв
			if(ENTITY_IS_SPAWNED(player))
			{
				entity_new(
					"explode_missile",
					player->pos.x,
					player->pos.y,
					player->dir,
					player,
					NULL
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

			if(!pl->soundId_move)
			{
				pl->soundId_move = sound_play_start(SOUND_PLAYER_TANKMOVE, -1);
			}

		}
		else
		{
			//игрок останавливается
			pl->move.speed -= PLAYER_DECEL * dtime;
		};
		if(pl->move.speed < 0)
		{
			if(pl->soundId_move)
			{
				sound_play_stop(pl->soundId_move);
				pl->soundId_move = 0;
			}
			pl->move.speed = 0;
		}
		player_move(player, player->dir, &pl->move.speed);

		speed_s = playerinfo->speed / 4;

		//стрейф
		switch(player->dir)
		{
		case DIR_UP:
		case DIR_DOWN:
			Sorig = player->pos;
			Sorig.x = Sorig.x-c_p_MDL_box/4;
			map_clip_find_near(&Sorig,c_p_MDL_box/2,player->dir,0xF0,c_p_MDL_box/2+2, &L);
			Sorig = player->pos;
			Sorig.x = Sorig.x+c_p_MDL_box/4;
			map_clip_find_near(&Sorig,c_p_MDL_box/2,player->dir,0xF0,c_p_MDL_box/2+2, &R);
			if((c_p_MDL_box/2<L) && (R-1<=c_p_MDL_box/2)) player_move(player,DIR_LEFT, &speed_s);//strafe left
			if((c_p_MDL_box/2<R) && (L-1<=c_p_MDL_box/2)) player_move(player,DIR_RIGHT, &speed_s);//strafe right
			break;
		case DIR_LEFT:
		case DIR_RIGHT:
			Sorig = player->pos;
			Sorig.y = Sorig.y-c_p_MDL_box/4;
			map_clip_find_near(&Sorig,c_p_MDL_box/2,player->dir,0xF0,c_p_MDL_box/2+2, &D);
			Sorig = player->pos;
			Sorig.y = Sorig.y+c_p_MDL_box/4;
			map_clip_find_near(&Sorig,c_p_MDL_box/2,player->dir,0xF0,c_p_MDL_box/2+2, &U);
			if((c_p_MDL_box/2<U)&&(D-1<=c_p_MDL_box/2)) player_move(player,DIR_UP, &speed_s);//strafe up
			if((c_p_MDL_box/2<D)&&(U-1<=c_p_MDL_box/2)) player_move(player,DIR_DOWN, &speed_s);//strafe down
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

					int item;
					switch(pl->weap)
					{
					case WEAP_ARTILLERY: item = ITEM_AMMO_ARTILLERY; break;
					case WEAP_MISSILE  : item = ITEM_AMMO_MISSILE; break;
					case WEAP_MINE     : item = ITEM_AMMO_MINE; break;
					default: item = ITEM_AMMO_ARTILLERY;
					}

					//если не стреляем управляемой ракетой
					if(
							(playerinfo->items[item] == ITEM_AMOUNT_INF)||            //если пуль бесконечно много
							(ENTITY_VARIABLE_INTEGER(player, list[item]) > 0)
					)
					{
						// пули не кончились
						pl->reloadtime_d = c_p_WEAP_reloadtime;
						bulltype_t bulltype = entity_weapon_type_to_bull_type(pl->weap);

						//создаем пулю
						entity_new(
							entity_bulltype_to_mobj(bulltype),
							player->pos.x,
							player->pos.y,
							player->dir,
							player,
							NULL
							);
						switch(pl->weap)
						{
						case WEAP_ARTILLERY:
							sound_play_start(SOUND_WEAPON_ARTILLERY_1, 1);
							break;
						case WEAP_MISSILE:
							sound_play_start(SOUND_WEAPON_ARTILLERY_2, 1);
							break;
						case WEAP_MINE:
							sound_play_start(SOUND_WEAPON_ARTILLERY_2, 1);
							break;
						default: ;
						};
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
	//подбираем предметы
	player_pickup(player);
}

/*
 * инициализация класса танка
 */
void player_class_init(entity_t * player, player_t * pl)
{
	int level = ENTITY_VARIABLE_INTEGER(player, "scores") / ITEM_SCOREPERCLASS;
	if(level > PLAYER_LEVEL5) level = PLAYER_LEVEL5;
	if( ENTITY_IS(player, "boss") )
		level = PLAYER_LEVEL_BOSS;

	ENTITY_VARIABLE_INTEGER(player, "level") = level;

	playerinfo_t * playerinfo = &playerinfo_table[level];

	int i;
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
				playerinfo->items[i] == ITEM_AMOUNT_NA ||
				playerinfo->items[i] == ITEM_AMOUNT_INF
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
void player_getdamage(entity_t * player, entity_t * explode, bool self, float radius)
{
	int damage_full;
	int armor;

	//weapon_info_t * weapinfo = &wtable[explode->explode.type];
	explode_t * expl = explode->data;
	explodeinfo_t * explode_info = &explodeinfo_table[expl->type];

	if( ENTITY_VARIABLE_INTEGER(player, "item_health") > 0 && radius <= explode_info->radius )
	{
		if(self)
			damage_full = explode_info->selfdamage*(1-radius/explode_info->radius);
		else
			damage_full = explode_info->damage    *(1-radius/explode_info->radius);

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
	entity_int_t level = ENTITY_VARIABLE_INTEGER(player, "level");
	playerinfo_t *playerinfo = &playerinfo_table[level];

	int ref_y = VIDEO_SCREEN_H - 16 * 2;

	font_color_set3i(COLOR_1);
	// gr2D_settext(cam->x,cam_y,0,'('+realtostr(player->move.orig.x,8,4)+';'+realtostr(player->move.orig.y,8,4)+')');
	// gr2D_settext(cam->x,cam_y,0,"PING %d", player->time.delta);

	ui_drawimage(cam, 16 * 0     , ref_y, game.i_health);
	ui_drawimage(cam, 16 * 6     , ref_y, game.i_armor);

	ui_printf(cam, 16 * 0 + 16, ref_y, "%d", ENTITY_VARIABLE_INTEGER(player, "item_health"));
	ui_printf(cam, 16 * 6 + 16, ref_y, "%d", ENTITY_VARIABLE_INTEGER(player, "item_armor"));
	ui_printf(cam, 16 * 0 + 16, ref_y + 8, "%d", playerinfo->items[ITEM_HEALTH]);
	ui_printf(cam, 16 * 6 + 16, ref_y + 8, "%d", playerinfo->items[ITEM_ARMOR]);

	/* вторая строка */
	ref_y += 16;
	//gr2D_setimage1(cam->x + 16 * 0, ref_y, image_get( list[pl->level] ), 0, 0, c_p_MDL_box,c_p_MDL_box);
	ui_drawimage(cam, 16 * 4, ref_y, image_get( wtable[0].icon ));
	ui_drawimage(cam, 16 * 6, ref_y, image_get( wtable[1].icon ));
	ui_drawimage(cam, 16 * 8, ref_y, image_get( wtable[2].icon ));

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
		ENTITYINFO_VARS(player_vars),
		.init = player_init,
		.done = player_done,
		.spawn = player_spawn,
		.handle = player_handle,
		.client_store = NULL,
		.client_restore = player_restore,
		ENTITYINFO_ACTIONS(player_actions),
		ENTITYINFO_ENTMODELS(tank_player_models)
};

static const entityinfo_t enemy_reginfo = {
		.name = "enemy",
		.datasize = sizeof(player_t),
		ENTITYINFO_VARS(player_vars),
		.init = enemy_init,
		.done = enemy_done,
		.handle   = enemy_handle,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_ENTMODELS(tank_enemy_models)
};

static const entityinfo_t boss_reginfo = {
		.name = "boss",
		.datasize = sizeof(player_t),
		ENTITYINFO_VARS(player_vars),
		.init = boss_init,
		.done = boss_done,
		.handle   = boss_handle,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_ENTMODELS(tank_boss_models)
};

void entity_player_init(void)
{
	entity_register(&player_reginfo);
	entity_register(&enemy_reginfo);
	entity_register(&boss_reginfo);
	ui_register(player_ui_draw);
}
