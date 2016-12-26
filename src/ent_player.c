/*
 *        Ubivat Tank
 *        обработка игрока
 *
 * by Master San
 */

#include "ent_player.h"
#include "ent_spawn.h"
#include "ent_message.h"
#include "ent_exit.h"
#include "ent_items.h"
#include "ent_bull.h"
#include "ent_explode.h"
#include "ent_message.h"
#include "ent_weap.h"

#include "system.h"
#include "types.h"
#include "entity.h"
#include "video.h"
#include "img.h"
#include "map.h"
#include "game.h"
#include "_gr2D.h"
#include "sound.h"
#include "client.h"

playerinfo_t playerinfo_table[__PLAYER_LEVEL_NUM] =
{
		/* SCORES                 , HEALTH, ARMOR,  AMMO_ARTILLERY,   AMMO_MISSILE,     AMMO_MINE */
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
static void player_handle(entity_t * this);
static void * player_store(client_storedata_t * storedata, const void * mobj);
static void player_restore(void * data, const client_storedata_t * storedata, const void * userstoredata);


static ENTITY_FUNCTION_INIT(enemy_init);
static ENTITY_FUNCTION_DONE(enemy_done);
static void enemy_handle(entity_t * this);

static ENTITY_FUNCTION_INIT(boss_init);
static ENTITY_FUNCTION_DONE(boss_done);
static void boss_handle(entity_t * this);

static const entityinfo_t player_reginfo = {
		.name = "player",
		.datasize = sizeof(player_t),
		.init = player_init,
		.done = player_done,
		.handle   = player_handle,
		.client_store = player_store,
		.client_restore = player_restore,
		.entmodels_num = 2,
		.entmodels = tank_player_models
};

static const entityinfo_t enemy_reginfo = {
		.name = "enemy",
		.datasize = sizeof(player_t),
		.init = enemy_init,
		.done = enemy_done,
		.handle   = enemy_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 2,
		.entmodels = tank_enemy_models
};

static const entityinfo_t boss_reginfo = {
		.name = "boss",
		.datasize = sizeof(player_t),
		.init = boss_init,
		.done = boss_done,
		.handle   = boss_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 2,
		.entmodels = tank_boss_models
};

void entity_player_init()
{
	entity_register(&player_reginfo);
	entity_register(&enemy_reginfo);
	entity_register(&boss_reginfo);
}

static void player_handle_common(entity_t * player);

ENTITY_FUNCTION_INIT(player_init)
{
	player_t * pl = thisdata;

#if defined(_DEBUG_PLAYERMAXLEVEL)
	for( int i = 0; i < __ITEM_NUM; i++)
	{
		pl->items[i] = 50;
	}
	pl->items[ITEM_SCORES] = 9000;
#endif

	player_spawn_init(this, pl, parent);// parent = spawn

}

ENTITY_FUNCTION_DONE(player_done)
{
	player_t * pl = thisdata;
	sound_play_stop(pl->soundId_move);
	ctrl_AI_done(&(pl->brain));
}

void player_handle(entity_t * this)
{
	think_human(this);
	player_handle_common(this);
}

void * player_store(client_storedata_t * storedata, const void * data)
{
	storedata->fragstotal = ((player_t *)data)->charact.fragstotal;
	storedata->frags      = ((player_t *)data)->charact.frags;
	storedata->level      = ((player_t *)data)->level;
	storedata->scores     = ((player_t *)data)->items[ITEM_SCORES];

	void * usersoredata = Z_malloc(sizeof(int) * __ITEM_NUM);
	memcpy(
		usersoredata,
		((player_t *)data)->items,
		sizeof(int) * __ITEM_NUM
	);
	return usersoredata;
}

void player_restore(void * data, const client_storedata_t * storedata, const void * userstoredata)
{
	((player_t *)data)->charact.fragstotal = storedata->fragstotal;
	((player_t *)data)->charact.frags      = storedata->frags;
	((player_t *)data)->level              = storedata->level;
	((player_t *)data)->items[ITEM_SCORES] = storedata->scores;
	memcpy(
		((player_t *)data)->items,
		userstoredata,
		sizeof(int) * __ITEM_NUM
	);
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
void enemy_handle(entity_t * this)
{
	think_enemy(this);
	player_handle_common(this);
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
void boss_handle(entity_t * this)
{
	//think_enemy(player);
	player_handle_common(this);
}




static void _move(int id, direction_t dir, bool go)
{
	client_t * cl = client_get(id);
	if(!cl)return;
	player_t * pl = cl->entity->data;

	if(go)
		cl->entity->dir = dir;
	else
	{
		if(cl->entity->dir != dir)
			return;
	}
	pl->move.go = go;
}

static void _attack(int id, bool attack, weapontype_t weap)
{
	client_t * cl = client_get(id);
	if(!cl)return;
	player_t * pl = cl->entity->data;

	if(attack)
		pl->weap = weap;
	pl->attack = attack;
}


void player_moveUp_ON()
{
	_move(0, DIR_UP, true);
}

void player_moveUp_OFF()
{
	_move(0, DIR_UP, false);
}

void player_moveDown_ON()
{
	_move(0, DIR_DOWN, true);
}

void player_moveDown_OFF()
{
	_move(0, DIR_DOWN, false);
}

void player_moveLeft_ON()
{
	_move(0, DIR_LEFT, true);
}

void player_moveLeft_OFF()
{
	_move(0, DIR_LEFT, false);
}

void player_moveRight_ON()
{
	_move(0, DIR_RIGHT, true);
}

void player_moveRight_OFF()
{
	_move(0, DIR_RIGHT, false);
}

void player_attack_weapon1_ON()
{
	_attack(0, true, WEAP_ARTILLERY);
}
void player_attack_weapon1_OFF()
{
	_attack(0, false, WEAP_ARTILLERY);
}

void player_attack_weapon2_ON()
{
	_attack(0, true, WEAP_MISSILE);
}
void player_attack_weapon2_OFF()
{
	_attack(0, false, WEAP_MISSILE);
}

void player_attack_weapon3_ON()
{
	_attack(0, true, WEAP_MINE);
}
void player_attack_weapon3_OFF()
{
	_attack(0, false, WEAP_MINE);
}

void player2_moveUp_ON()
{
	_move(1, DIR_UP, true);
}

void player2_moveUp_OFF()
{
	_move(1, DIR_UP, false);
}

void player2_moveDown_ON()
{
	_move(1, DIR_DOWN, true);
}

void player2_moveDown_OFF()
{
	_move(1, DIR_DOWN, false);
}

void player2_moveLeft_ON()
{
	_move(1, DIR_LEFT, true);
}

void player2_moveLeft_OFF()
{
	_move(1, DIR_LEFT, false);
}

void player2_moveRight_ON()
{
	_move(1, DIR_RIGHT, true);
}

void player2_moveRight_OFF()
{
	_move(1, DIR_RIGHT, false);
}

void player2_attack_weapon1_ON()
{
	_attack(1, true, WEAP_ARTILLERY);
}
void player2_attack_weapon1_OFF()
{
	_attack(1, false, WEAP_ARTILLERY);
}

void player2_attack_weapon2_ON()
{
	_attack(1, true, WEAP_MISSILE);
}
void player2_attack_weapon2_OFF()
{
	_attack(1, false, WEAP_MISSILE);
}

void player2_attack_weapon3_ON()
{
	_attack(1, true, WEAP_MINE);
}
void player2_attack_weapon3_OFF()
{
	_attack(1, false, WEAP_MINE);
}

/*
 * набор и проверка кодов (добавление 10.05.2006)
 */
void player_checkcode()
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

void check_value_int(int * val, int min, int max)
{
	if(*val > max) *val = max;
	else if(*val < min) *val = min;
}

static void player_influence_message(entity_t * actor, entity_t * exposed)
{
	//отправим сообщение игроку
	game_message_send(ENT_MESSAGE(exposed)->message);
}

static void player_influence_exit(entity_t * actor, entity_t * exposed)
{
	//отправим сообщение игроку
	game_message_send(ENT_EXIT(exposed)->message);
	game._win_ = true;
}

static void player_influence_item(entity_t * player, entity_t * entity)
{
	player_t * pl = player->data;

	itemtype_t itemtype = items_enttype_to_itemtype(entity->info->name);
	if((int) itemtype < 0)
	{
		game_halt("player_items_get(): invalid itemtype = %d\n", itemtype);
	}
	item_t * item = entity->data;
	if(!item->exist) return;

	playerinfo_t *playerinfo = &playerinfo_table[pl->level];

	if(
			playerinfo->items[itemtype] != ITEM_AMOUNT_INF &&
			playerinfo->items[itemtype] != ITEM_AMOUNT_NA &&
			(
			((pl->items[itemtype] < playerinfo->items[itemtype]) || item->amount < 0) ||
			itemtype == ITEM_SCORES
			)
	)
	{
		item->exist = false;
		pl->items[itemtype] += item->amount;
		check_value_int(&pl->items[itemtype], 0, playerinfo->items[itemtype]);
	};

	if(itemtype == ITEM_SCORES)
	{
		player_class_init(player, player->data);
		if(5 <= pl->items[ITEM_SCORES] / ITEM_SCOREPERCLASS)
		{
			if(pl->items[ITEM_HEALTH] < playerinfo->items[ITEM_HEALTH])
				pl->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
			if(pl->items[ITEM_ARMOR] < pl->items[ITEM_ARMOR])
				pl->items[ITEM_ARMOR] = playerinfo->items[ITEM_ARMOR];
		}
	}
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
static void player_obj_check(entity_t * player)
{
	player_t * pl = player->data;

	if(pl->items[ITEM_HEALTH] <= 0) return;

	entity_t * entity;

	FOR_ENTITIES("item_scores", entity)
	{
		if( inbox(entity, player) )
			player_influence_item(player, entity);
	}

	FOR_ENTITIES("item_health", entity)
	{
		if( inbox(entity, player) )
			player_influence_item(player, entity);
	}

	FOR_ENTITIES("item_armor", entity)
	{
		if( inbox(entity, player) )
			player_influence_item(player, entity);
	}

	FOR_ENTITIES("item_ammo_missile", entity)
	{
		if( inbox(entity, player) )
			player_influence_item(player, entity);
	}

	FOR_ENTITIES("item_ammo_mine", entity)
	{
		if( inbox(entity, player) )
			player_influence_item(player, entity);
	}

	FOR_ENTITIES("message", entity)
	{
		if( inbox(entity, player) )
			player_influence_message(player, entity);
	}

	FOR_ENTITIES("exit", entity)
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
static void player_handle_common(entity_t * player)
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

	player_t * pl = player->data;

	playerinfo_t * playerinfo = &playerinfo_table[pl->level];

	if(pl->items[ITEM_HEALTH] <= 0)
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
			if(pl->charact.spawned)
			{
				entity_new(
					"explode_missile",
					player->pos.x,
					player->pos.y,
					player->dir,
					player,
					NULL
				);
				pl->charact.spawned = false;
				pl->items[ITEM_ARMOR] = 0;
				pl->items[ITEM_AMMO_MISSILE] = 0;
				pl->items[ITEM_AMMO_MINE] = 0;
			};
			if(strcmp(player->info->name, "boss") == 0)
				game._win_ = true;
			break;
	}

	ENTITY_ALLOW_DRAW_SET(player, pl->charact.spawned);

	if(pl->items[ITEM_HEALTH] > 0)
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
		if(pl->items[ITEM_HEALTH] <= 0)
		{
			if(
				((game.flags & c_g_f_2PLAYERS) != 0) &&
				(strcmp(player->info->name, "player") == 0)
				)
			{
				player_respawn(player);
				pl->reloadtime_d = c_p_WEAP_reloadtime;
			};
		}
		else
		{
			if(pl->reloadtime_d > 0) pl->reloadtime_d -= dtime;//учитываем время на перезарядку
			else
			{
				if(!pl->bull)
				{
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
							(pl->items[item] > 0)
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
								(strcmp(player->info->name, "player") == 0)    // игрок не монстр(у монстров пули не кончаются)
						)
							pl->items[item]--;
					}
				}
			}
		}
	}
	if(pl->reloadtime_d < 0) pl->reloadtime_d = 0;
	//подбираем предметы
	player_obj_check(player);
}

/*
 * инициализируем игрока при спавне
 */
void player_spawn_init(entity_t * player, player_t * pl, const entity_t * spawn)
{
	if(!spawn)
		game_halt("Error: Player spawn is NULL, can not create player.");
	spawn_t * sp = spawn->data;

	if(0 <= sp->items[ITEM_SCORES] && sp->items[ITEM_SCORES] <= c_score_max)
		pl->items[ITEM_SCORES] = sp->items[ITEM_SCORES];
	player_class_init(player, pl);
	playerinfo_t * playerinfo = &playerinfo_table[pl->level];

	if(0 <= sp->items[ITEM_HEALTH] && sp->items[ITEM_HEALTH] <= playerinfo->items[ITEM_HEALTH])
		pl->items[ITEM_HEALTH] = sp->items[ITEM_HEALTH];
	else
	{
		if( strcmp(player->info->name, "player") != 0 )
			pl->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
		else
		{
			if(game.flags & c_g_f_CASE)
				pl->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
			else
			{
				//по уровням
				if(game.gamemap == mapList) // первая карта
					pl->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
				else // не первая карта
					if(!pl->charact.spawned && pl->items[ITEM_HEALTH] <= 0)
						pl->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
			}
		}
	}
	if(0 <= sp->items[ITEM_ARMOR] && sp->items[ITEM_ARMOR] <= playerinfo->items[ITEM_ARMOR] )
		pl->items[ITEM_ARMOR] = sp->items[ITEM_ARMOR];
	else
	{
		if( strcmp(player->info->name, "player") != 0 )
			pl->items[ITEM_ARMOR] = playerinfo->items[ITEM_ARMOR];
		else
			if(game.flags & c_g_f_CASE) pl->items[ITEM_ARMOR] = playerinfo->items[ITEM_ARMOR];
	};
	pl->charact.frags = 0;
	pl->charact.spawned = true;
	pl->bull            = NULL;
	pl->move.speed      = 0;
	pl->move.go         = false;
	pl->attack          = false;
	pl->reloadtime_d    = 0;
	pl->soundId_move    = 0;

};


/**
 * получить любой спавн-поинт для игрока
 */
static entity_t * player_spawn_get() /* TODO: erase this func */
{

	int count = 0;

	entity_t * ent;

	//считаем количество спавн-поинтов
	for(ent = entity_getfirst("spawn_player"); ent; ent = ent->next)
	{
		count++;
	};
	if(count == 0)
		return NULL;
	count = xrand(count);

	//выбираем случайным образом
	for(ent = entity_getfirst("spawn_player"); ent; ent = ent->next)
	{
		if(count == 0) return ent;
		count--;
	};
	return NULL;
}


/**
 * появление/восстановление игрока на карте
 * @return = 0 -спавнинг прошел успешно
 * @return = 1 -игрок является монстром, ошибка спавнинга
 */
int player_respawn(entity_t * player)
{
	/* игрок является монстром, ошибка спавнинга */
	if( strcmp(player->info->name, "player") != 0 )
		return 1;

	entity_t * spawn = player_spawn_get();

	player->pos.x = spawn->pos.x;
	player->pos.y = spawn->pos.y;
	player_spawn_init(player, player->data, spawn);
	return 0;
}

/*
 * вывод информации об игроке
 */
void player_draw_status(camera_t * cam, entity_t * player)
{
	static image_index_t list[] =
	{
			IMG_TANK0,
			IMG_TANK1,
			IMG_TANK2,
			IMG_TANK3,
			IMG_TANK4
	};

	player_t * pl = player->data;

	playerinfo_t * playerinfo = &playerinfo_table[pl->level];
	int ref_y = VIDEO_SCREEN_H - 16 * 2;

	font_color_set3i(COLOR_1);
	// gr2D_settext(cam->x,cam_y,0,'('+realtostr(player->move.orig.x,8,4)+';'+realtostr(player->move.orig.y,8,4)+')');
	// gr2D_settext(cam->x,cam_y,0,"PING %d", player->time.delta);

	gr2D_setimage0(cam->x + 16 * 0     , ref_y, game.i_health);
	gr2D_setimage0(cam->x + 16 * 6     , ref_y, game.i_armor);

	video_printf(cam->x + 16 * 0 + 16, ref_y, orient_horiz, "%d", pl->items[ITEM_HEALTH]);
	video_printf(cam->x + 16 * 6 + 16, ref_y, orient_horiz, "%d", pl->items[ITEM_ARMOR]);
	video_printf(cam->x + 16 * 0 + 16, ref_y + 8, orient_horiz, "%d", playerinfo->items[ITEM_HEALTH]);
	video_printf(cam->x + 16 * 6 + 16, ref_y + 8, orient_horiz, "%d", playerinfo->items[ITEM_ARMOR]);

	/* вторая строка */
	ref_y += 16;
	gr2D_setimage1(cam->x + 16 * 0, ref_y, image_get( list[pl->level] ), 0, 0, c_p_MDL_box,c_p_MDL_box);
	gr2D_setimage0(cam->x + 16 * 4, ref_y, image_get( wtable[0].icon ));
	gr2D_setimage0(cam->x + 16 * 6, ref_y, image_get( wtable[1].icon ));
	gr2D_setimage0(cam->x + 16 * 8, ref_y, image_get( wtable[2].icon ));

	video_printf(cam->x + 16 * 4 + 16, ref_y + 4, orient_horiz, "@"); // player->items[ITEM_AMMO_ARTILLERY]
	if(pl->items[ITEM_AMMO_MISSILE] >= 0)
		video_printf(cam->x + 16 * 6 + 16, ref_y + 4, orient_horiz, "%d", pl->items[ITEM_AMMO_MISSILE]);
	if(pl->items[ITEM_AMMO_MINE] >= 0)
		video_printf(cam->x + 16 * 8 + 16, ref_y + 4, orient_horiz , "%d", pl->items[ITEM_AMMO_MINE]);
	video_printf(cam->x + 16 * 0 + 16, ref_y +  4, orient_horiz, "%d", pl->items[ITEM_SCORES]);

}

/*
 * инициализация класса танка
 */
void player_class_init(entity_t * player, player_t * pl)
{
	int level = pl->items[ITEM_SCORES] / ITEM_SCOREPERCLASS;
	if(level > PLAYER_LEVEL5) level = PLAYER_LEVEL5;
	if( strcmp(player->info->name, "boss") == 0 )
		level = PLAYER_LEVEL_BOSS;
	pl->level = level;
	playerinfo_t * playerinfo = &playerinfo_table[level];

	int i;
	for(i = 0; i< __ITEM_NUM; i++)
	{
		if(playerinfo->items[i] == ITEM_AMOUNT_NA)
			pl->items[i] = playerinfo->items[i];
		else if(playerinfo->items[i] == ITEM_AMOUNT_INF)
			pl->items[i] = playerinfo->items[i];
		else
		{
			if( pl->items[i] < 0 ) pl->items[i] = 0;
			if( strcmp(player->info->name, "player") != 0 )
			{
				pl->items[i] = playerinfo->items[i];
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
	player_t * pl = player->data;

	int damage_full;
	int armor;

	//weapon_info_t * weapinfo = &wtable[explode->explode.type];
	explode_t * expl = explode->data;
	explodeinfo_t * explode_info = &explodeinfo_table[expl->type];

	if( pl->items[ITEM_HEALTH] > 0 && radius <= explode_info->radius )
	{
		if(self)
			damage_full = explode_info->selfdamage*(1-radius/explode_info->radius);
		else
			damage_full = explode_info->damage    *(1-radius/explode_info->radius);

		int damage_armor = damage_full*2/3;
		int damage_health = damage_full - damage_armor;

		armor = pl->items[ITEM_ARMOR] - damage_armor;
		pl->items[ITEM_HEALTH] = pl->items[ITEM_HEALTH] - damage_health;
		if(armor < 0)
		{
			pl->items[ITEM_HEALTH] = pl->items[ITEM_HEALTH] + armor;
			pl->items[ITEM_ARMOR] = 0;
		}
		else
			pl->items[ITEM_ARMOR] = armor;
		if(pl->items[ITEM_HEALTH] <= 0)
		{
			entity_t * eplayer = explode->parent;
			player_t * pl = eplayer->data;
			if(!self)
			{

				//атакующему добавим очки
				pl->items[ITEM_SCORES] += c_score_pertank;
				pl->charact.fragstotal++;
				pl->charact.frags++;
			}
			else
			{
				//атакующий умер от своей пули
				pl->items[ITEM_SCORES] = 0;
				pl->charact.fragstotal--;
				pl->charact.frags--;
			}
			player_class_init(eplayer, pl);
		}
	}
}

