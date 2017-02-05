/*
 *        Ubivat Tank
 *        управление игроками
 * by Master San
 */
#include "ent_bull.h"
#include "ent_explode.h"
#include "ent_player_think.h"
#include "ent_player.h"
#include "ent_weap.h"

#include "entity_helpers.h"
#include "types.h"
#include "game.h"
#include "map.h"
#include "client.h"
#include "system.h"

#include <stdlib.h>
#include <types.h>

static bool debug_noAI = false;

static explodetype_t weapontype_to_explodetype(weapontype_t weapontype)
{
	switch(weapontype)
	{
		case WEAP_ARTILLERY: return EXPLODE_ARTILLERY;
		case WEAP_MISSILE  : return EXPLODE_MISSILE;
		case WEAP_MINE     : return EXPLODE_MINE;
		default: ;
	}
	return EXPLODE_ARTILLERY;
}

static explodetype_t bullentity_to_explodetype(entity_t * bull)
{
	if(ENTITY_IS(bull, "bull_artillery"))
	{
		return EXPLODE_ARTILLERY;
	}
	if(ENTITY_IS(bull, "bull_missile"))
	{
		return EXPLODE_MISSILE;
	}
	if(ENTITY_IS(bull, "bull_mine"))
	{
		return EXPLODE_MINE;
	}
	return EXPLODE_ARTILLERY;
}

/*
 * инициализация AI
 */
void ctrl_AI_init(think_t * thiker)
{
	thiker->danger  = 0;
	thiker->Fdanger = 0;
	thiker->attack  = false;
	thiker->target  = NULL;
	thiker->count   = xrand(BOT_THINK_TIME*2);
};
/*
 * удаление AI
 */
void ctrl_AI_done(think_t * thinker)
{
	thinker->target = NULL;
};


/*
 * уворачивание от снарядов
 */
static bool ctrl_AI_checkdanger(entity_t * player, entity_t * dangerous)
{
	vec_t Udist;
	vec_t Ddist;
	vec_t Ldist;
	vec_t Rdist;
	vec_t Ud;
	vec_t Dd;
	vec_t Ld;
	vec_t Rd;

	player_t * pl = player->data;

	bool danger = false;
	if(pl->bull)
		return false;
	if(dangerous->parent == player)
		return false;

	vec_t radius = dangerous->info->bodybox * 0.5f;

	//верхняя ближайшая стена
	map_clip_find_near(&player->origin, 0, DIR_UP, MAP_WALL_CLIP, 100, &Udist);
	//нижняя ближайшая стена
	map_clip_find_near(&player->origin, 0, DIR_DOWN, MAP_WALL_CLIP, 100, &Ddist);
	//левая ближайшая стена
	map_clip_find_near(&player->origin, 0, DIR_LEFT, MAP_WALL_CLIP, 160, &Ldist);
	//правая ближайшая стена
	map_clip_find_near(&player->origin, 0, DIR_RIGHT, MAP_WALL_CLIP, 160, &Rdist);
	Ud = (player->origin[1] + Udist - c_p_MDL_box/2) - (dangerous->origin[1] + radius);
	Dd = (dangerous->origin[1] - radius) - (player->origin[1]-Ddist + c_p_MDL_box / 2);
	Rd = (player->origin[0] + Rdist - c_p_MDL_box/2) - (dangerous->origin[0] + radius);
	Ld = (dangerous->origin[0] - radius) - (player->origin[0] - Ldist + c_p_MDL_box / 2);
	if(
			(player->origin[0] - c_p_MDL_box/2 <= dangerous->origin[0] + radius)&&
			(dangerous->origin[0] - radius <= player->origin[0] + c_p_MDL_box/2)&&
			(VEC_ABS(player->origin[1]-dangerous->origin[1]) < 128)
	)
	{
		if(
				(dangerous->dir == DIR_UP) &&
				(VEC_ABS(player->origin[1]-dangerous->origin[1])<Ddist) &&
				(dangerous->origin[1] + radius < player->origin[1]-c_p_MDL_box/2)
		)
		{
			danger = true;
			if(!pl->brain.Fdanger)
			{
				pl->move.go = true;
				pl->brain.Fdanger = true;
				if(0 <= Ld && 0 <= Rd)
					player->dir = xrand(2) + 2;
				else
					if(0 < Ld) player->dir = DIR_LEFT;
					else
						if(0 < Rd) player->dir = DIR_RIGHT;
			}
		}
		else
		{
			if(
					(dangerous->dir == DIR_DOWN) &&
					(VEC_ABS(player->origin[1]-dangerous->origin[1])<Udist) &&
					(player->origin[1]+c_p_MDL_box/2 < dangerous->origin[1] - radius)
			)
			{
				danger = true;
				if(!pl->brain.Fdanger) {
					pl->move.go = true;
					pl->brain.Fdanger = true;
					if( 0 <= Ld && 0 <= Rd) player->dir = xrand(2)+2;
					else
						if(0 < Ld) player->dir = DIR_LEFT;
						else
							if(0 < Rd) player->dir = DIR_RIGHT;
				}
			}
		}
	}
	else
	{
		if(
				(player->origin[1] - c_p_MDL_box/2 <= dangerous->origin[1] + radius) &&
				(dangerous->origin[1] - radius <= player->origin[1] + c_p_MDL_box/2) &&
				(VEC_ABS(player->origin[0] - dangerous->origin[0]) < 128)
		)
		{
			if (
					(dangerous->dir == DIR_LEFT)&&
					(VEC_ABS(player->origin[0]-dangerous->origin[0])<Rdist)&&
					(player->origin[0]+c_p_MDL_box/2 < dangerous->origin[0]- radius)
			)
			{
				danger = true;
				if(!pl->brain.Fdanger)
				{
					pl->move.go = true;
					pl->brain.Fdanger = true;
					if(0 <= Ud && 0 <= Dd)
						player->dir = xrand(2);
					else
						if(0 < Ud)
							player->dir = DIR_UP;
						else
							if(0 < Dd)
								player->dir = DIR_DOWN;
				}
			}
			else
			{
				if(
						(dangerous->dir == DIR_RIGHT) &&
						(VEC_ABS(player->origin[0] - dangerous->origin[0])<Ldist) &&
						(dangerous->origin[0] + radius < player->origin[0] - c_p_MDL_box / 2)
				)
				{
					danger = true;
					if(!pl->brain.Fdanger)
					{
						pl->move.go = true;
						pl->brain.Fdanger = true;
						if(0 <= Ud && 0 <= Dd) player->dir = xrand(2);
						else
							if(0 < Ud) player->dir = DIR_UP;
							else
								if(0 < Dd) player->dir = DIR_DOWN;
					}
				}
			}
		}
	}
	return danger;
}



/*
 * уворачивание от снарядов
 */
static void ctrl_AI_checkdangers(entity_t * player)
{
	player_t * pl = player->data;
	bool danger = false;
	entity_t * dangerous;

	ENTITIES_FOREACH("bull_artillery", dangerous)
	{
		danger = ctrl_AI_checkdanger(player, dangerous);
		if(danger)
			goto next;
	}
	ENTITIES_FOREACH("bull_missile", dangerous)
	{
		danger = ctrl_AI_checkdanger(player, dangerous);
		if(danger)
			goto next;
	}
	ENTITIES_FOREACH("bull_mine", dangerous)
	{
		danger = ctrl_AI_checkdanger(player, dangerous);
		if(danger)
			goto next;
	}

	next:
	pl->brain.danger = danger;
	if(!pl->brain.danger)
	{
		pl->brain.Fdanger = false;
		//player->move.go = false;
	}
}

/*
 * атака
 */
static void ctrl_AI_attack(entity_t * player, entity_t * target)
{
	player_t * pl = player->data;

	vec_t dist;
	char wall;

	static char *list[] =
	{
			"explode_artillery",
			"explode_missile",
			"explode_mine",
	};
	const entityinfo_t * explode_entityinfo = entityinfo_get(list[weapontype_to_explodetype(pl->brain.weap)]);
	vec_t radius = explode_entityinfo->bodybox * 0.5f;

	if( pl->bull && pl->brain.target )
	{
		if(
				VEC_ABS(pl->bull->origin[1] - pl->brain.target->origin[1]) <
				VEC_ABS(pl->bull->origin[0] - pl->brain.target->origin[0])
		)
		{
			if(pl->bull->origin[0] < pl->brain.target->origin[0])
				player->dir = DIR_RIGHT;
			else
				player->dir = DIR_LEFT;
		}
		else
		{
			if(pl->bull->origin[1] < pl->brain.target->origin[1])
				player->dir = DIR_UP;
			else
				player->dir = DIR_DOWN;
		}
		return;
	}
	if
	(
			VEC_ABS(player->origin[0] - target->origin[0]) > 160.0 ||
			VEC_ABS(player->origin[1] - target->origin[1]) > 100.0
	)
	{
		pl->attack = false;
		return;
	};	//если оружие не перезарядилось
	if(0 < pl->reloadtime_d)
		return;
	pl->brain.target = NULL;
	if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") > 0)
	{
		map_clip_find_near_wall(&player->origin, player->dir, &dist, &wall);
		if(
				(wall == (MAP_WALL_W0 | MAP_WALL_CLIP)) ||
				(
						(dist-c_p_MDL_box/2 < radius) &&
						((wall == (MAP_WALL_W0 | MAP_WALL_CLIP)) || (wall == (MAP_WALL_W1 | MAP_WALL_CLIP)))
				)
		)
			pl->brain.attack = false;
		else
		{
			pl->brain.weap = WEAP_MISSILE;
			pl->brain.attack = true;
		}
	}
	else
	{
		if(
				(player->origin[0] - c_p_MDL_box / 2 < target->origin[0]) &&
				(target->origin[0] < player->origin[0] + c_p_MDL_box/2)
		)
		{
			if(target->origin[1] < player->origin[1])
				player->dir = DIR_DOWN;
			else
				player->dir = DIR_UP;
			map_clip_find_near_wall(&player->origin, player->dir, &dist, &wall);
			if(
					//противник в прямой видимости
					(VEC_ABS(player->origin[1] - target->origin[1]) < dist - c_p_MDL_box/2) ||
					!MAP_WALL_CLIPPED(wall) ||
					MAP_WALL_TEXTURE(wall) == MAP_WALL_water
			){
				if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_mine") > 0)
				{
					//выбираем наугад ракету или мину
					pl->brain.weap = 1+xrand(2);
					if(pl->brain.weap == WEAP_MINE)
						player->dir = entity_direction_invert(player->dir);
				}
				else
				{
					if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") > 0)
						pl->brain.weap = WEAP_MISSILE;
					else
						pl->brain.weap = WEAP_ARTILLERY;
				}
				pl->brain.attack = true;
			}
			else
			{ //противник за стеной, пытаемся пробиться через стену
				if(MAP_WALL_TEXTURE(wall) == MAP_WALL_W0)
					pl->brain.attack = false;              //сильная броня, не стреляем
				else
				{
					if(MAP_WALL_TEXTURE(wall) == MAP_WALL_brick)
					{
						pl->brain.weap = WEAP_ARTILLERY;          //кирпич
						pl->brain.attack = true;
					}
					else
					{
						if(MAP_WALL_TEXTURE(wall) == MAP_WALL_W1)
						{                            //слабая броня
							if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_mine") > 0)
							{
								pl->brain.weap = 1+xrand(2);                             //выбираем наугад ракету или мину
								if(pl->brain.weap == WEAP_MINE)
									player->dir = entity_direction_invert(player->dir);
							}
							else
							{
								if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") > 0)
									pl->brain.weap = WEAP_MISSILE;
								else
									pl->brain.weap = WEAP_ARTILLERY;
							}
							pl->brain.attack = true;
						}
					}
				}
				if(
						(dist-c_p_MDL_box/2 < radius)&&
						(wall == (MAP_WALL_W0 | MAP_WALL_CLIP) || wall == (MAP_WALL_W1 | MAP_WALL_CLIP))
				)
					pl->brain.attack = false;
			}
		}
		else
		{
			if(
					(player->origin[1] - c_p_MDL_box/2 < target->origin[1])&&
					(target->origin[1] < player->origin[1] + c_p_MDL_box/2)
			)
			{
				if(target->origin[0]<player->origin[0])
					player->dir = DIR_LEFT;
				else
					player->dir = DIR_RIGHT;
				map_clip_find_near_wall(&player->origin, player->dir, &dist, &wall);
				if(
						//противник в прямой видимости
						(VEC_ABS(player->origin[0] - target->origin[0]) < dist - c_p_MDL_box/2)||
						!MAP_WALL_CLIPPED(wall) ||
						MAP_WALL_TEXTURE(wall) == MAP_WALL_water
				)
				{
					if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_mine") > 0)
					{
						pl->brain.weap = 1 + xrand(2);                               //выбираем наугад ракету или мину
						if(pl->brain.weap == WEAP_MINE)
							player->dir = entity_direction_invert(player->dir);
					}
					else
					{
						if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") > 0)
							pl->brain.weap = WEAP_MISSILE;
						else
							pl->brain.weap = WEAP_ARTILLERY;
					}
					pl->brain.attack = true;
				}
				else
				{   //противник за стеной, пытаемся пробиться через стену
					if(MAP_WALL_TEXTURE(wall) == MAP_WALL_W0)
						pl->brain.attack = false; //сильная броня, не стреляем
					else
					{
						if(MAP_WALL_TEXTURE(wall) == MAP_WALL_brick)
						{ //кирпич
							pl->brain.weap = WEAP_ARTILLERY;
							pl->brain.attack = true;
						}
						else
						{
							if(MAP_WALL_TEXTURE(wall) == MAP_WALL_W1)
							{                           //слабая броня
								if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_mine") > 0)
								{
									pl->brain.weap = 1+xrand(2);                            //выбираем наугад ракету или мину
									if(pl->brain.weap == WEAP_MINE)
										player->dir = entity_direction_invert(player->dir);
								}
								else
								{
									if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") > 0)
										pl->brain.weap = WEAP_MISSILE;
									else
										pl->brain.weap = WEAP_ARTILLERY;
								}
								pl->brain.attack = true;
							}
						}
					}
					if(
							(dist-c_p_MDL_box/2 < radius) &&
							((wall==MAP_WALL_W0+MAP_WALL_CLIP) || (wall==MAP_WALL_W1+MAP_WALL_CLIP))
					)
						pl->brain.attack = false;
				}
			}
		}






	}
	pl->weap = pl->brain.weap;
	pl->attack = pl->brain.attack;
	if(pl->brain.weap == WEAP_MISSILE)
		pl->brain.target = target; //если выпустил ракету, тогда цель постоянная
	pl->brain.attack = false;
}
/*
 * поиск врага
 */
static void ctrl_AI_findenemy(entity_t * player, entity_t * target)
{
	player_t * pl = player->data;

	if(
			(160<VEC_ABS(player->origin[0]-target->origin[0]))||
			(100<VEC_ABS(player->origin[1]-target->origin[1]))
	)
	{
		pl->attack = false;
		return;
	}
	if(pl->brain.count == 0) {
		player->dir = xrand(4);
		pl->move.go = true;
	}
	else
	{
		if(BOT_THINK_TIME + xrand(BOT_THINK_TIME) < pl->brain.count)
		{
			if(VEC_ABS(player->origin[0]-target->origin[0])>VEC_ABS(player->origin[1]-target->origin[1]))
			{
				if(player->origin[0] < target->origin[0])
					player->dir = DIR_RIGHT;
				else
					player->dir = DIR_LEFT;
			}
			else
			{
				if(player->origin[1] < target->origin[1])
					player->dir = DIR_UP;
				else
					player->dir = DIR_DOWN;
			}
			if(
					(VEC_ABS(player->origin[0] - target->origin[0]) < c_BOT_dist)&&
					(VEC_ABS(player->origin[1] - target->origin[1]) < c_BOT_dist)
			)
				pl->move.go = false;
			else
				pl->move.go = true;
		}
	}
	pl->brain.count += dtime;
	if(BOT_THINK_TIME * 2 < pl->brain.count)
		pl->brain.count = 0;
}

/*
 * управление вражеским игроком
 */
void think_enemy(entity_t * player)
{
	player_t * pl = player->data;
	if(debug_noAI)
		return;
	if(!player->alive)
		return;
	ctrl_AI_checkdangers(player);
	if(pl->brain.danger)
		return;
	entity_t * target = entity_get_random("player");
	if(!target)
		return;

	if(!target->alive)
		pl->attack = false;
	else
	{
		if(!pl->bull && !pl->attack)
			ctrl_AI_findenemy(player, target);
		ctrl_AI_attack(player, target);
	}
}
