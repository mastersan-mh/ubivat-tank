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
	thiker->count   = xrand(c_BOT_time*2);
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
static void ctrl_AI_checkdanger(entity_t * player)
{
	float Udist;
	float Ddist;
	float Ldist;
	float Rdist;
	float Ud;
	float Dd;
	float Ld;
	float Rd;

	player_t * pl = player->data;

	bool danger = false;

	static char *list[] =
	{
			"bull_artillery",
			"bull_missile",
			"bull_mine"
	};

	int i;
	for(i = 0; i < 3; i++)
	{
		entity_t * dangerous = entity_getfirst(list[i]);

		for(; dangerous; dangerous = dangerous->next)
		{
			if(danger)
				break;
			if(dangerous->parent == player)
				continue;

			vec_t radius = dangerous->info->bodybox * 0.5f;

			//верхняя ближайшая стена
			map_clip_find_near(&player->pos, 0, DIR_UP, MAP_WALL_CLIP, 100, &Udist);
			//нижняя ближайшая стена
			map_clip_find_near(&player->pos, 0, DIR_DOWN, MAP_WALL_CLIP, 100, &Ddist);
			//левая ближайшая стена
			map_clip_find_near(&player->pos, 0, DIR_LEFT, MAP_WALL_CLIP, 160, &Ldist);
			//правая ближайшая стена
			map_clip_find_near(&player->pos, 0, DIR_RIGHT, MAP_WALL_CLIP, 160, &Rdist);
			Ud = (player->pos.y + Udist - c_p_MDL_box/2) - (dangerous->pos.y + radius);
			Dd = (dangerous->pos.y - radius) - (player->pos.y-Ddist + c_p_MDL_box / 2);
			Rd = (player->pos.x + Rdist - c_p_MDL_box/2) - (dangerous->pos.x + radius);
			Ld = (dangerous->pos.x - radius) - (player->pos.x - Ldist + c_p_MDL_box / 2);
			if(
					(player->pos.x-c_p_MDL_box/2 <= dangerous->pos.x+ radius)&&
					(dangerous->pos.x- radius <= player->pos.x+c_p_MDL_box/2)&&
					(VEC_ABS(player->pos.y-dangerous->pos.y) < 128)
			)
			{
				if(
						(dangerous->dir == DIR_UP) &&
						(VEC_ABS(player->pos.y-dangerous->pos.y)<Ddist) &&
						(dangerous->pos.y + radius < player->pos.y-c_p_MDL_box/2)
				)
				{
					danger = true;
					if(!pl->brain.Fdanger)
					{
						pl->move.go = true;
						pl->brain.Fdanger = true;
						if(0 <= Ld && 0 <= Rd) player->dir = xrand(2) + 2;
						else
							if(0 < Ld) player->dir = DIR_LEFT;
							else
								if(0<Rd) player->dir = DIR_RIGHT;
					}
				}
				else
				{
					if(
							(dangerous->dir == DIR_DOWN) &&
							(VEC_ABS(player->pos.y-dangerous->pos.y)<Udist) &&
							(player->pos.y+c_p_MDL_box/2 < dangerous->pos.y - radius)
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
									if(0<Rd) player->dir = DIR_RIGHT;
						}
					}
				}
			}
			else {
				if(
						(player->pos.y - c_p_MDL_box/2 <= dangerous->pos.y + radius) &&
						(dangerous->pos.y - radius <= player->pos.y + c_p_MDL_box/2) &&
						(VEC_ABS(player->pos.x - dangerous->pos.x) < 128)
				)
				{
					if (
							(dangerous->dir == DIR_LEFT)&&
							(VEC_ABS(player->pos.x-dangerous->pos.x)<Rdist)&&
							(player->pos.x+c_p_MDL_box/2 < dangerous->pos.x- radius)
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
					else
					{
						if(
								(dangerous->dir == DIR_RIGHT) &&
								(VEC_ABS(player->pos.x - dangerous->pos.x)<Ldist) &&
								(dangerous->pos.x + radius < player->pos.x - c_p_MDL_box / 2)
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
		} // end for;
	}
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

	float dist;
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
				VEC_ABS(pl->bull->pos.y - pl->brain.target->pos.y) <
				VEC_ABS(pl->bull->pos.x - pl->brain.target->pos.x)
		)
		{
			if(pl->bull->pos.x < pl->brain.target->pos.x)
				player->dir = DIR_RIGHT;
			else
				player->dir = DIR_LEFT;
		}
		else
		{
			if(pl->bull->pos.y < pl->brain.target->pos.y)
				player->dir = DIR_UP;
			else
				player->dir = DIR_DOWN;
		}
		return;
	}
	if
	(
			VEC_ABS(player->pos.x - target->pos.x) > 160.0 ||
			VEC_ABS(player->pos.y - target->pos.y) > 100.0
	)
	{
		pl->attack = false;
		return;
	};
	//если оружие не перезарядилось
	if(0 < pl->reloadtime_d) return;
	pl->brain.target = NULL;
	if(
			(player->pos.x-c_p_MDL_box/2<target->pos.x) &&
			(target->pos.x<player->pos.x+c_p_MDL_box/2)
	)
	{
		if(target->pos.y < player->pos.y)
			player->dir = DIR_DOWN;
		else
			player->dir = DIR_UP;
		map_clip_find_near_wall(&player->pos, player->dir, &dist, &wall);
		if(
				//противник в прямой видимости
				(VEC_ABS(player->pos.y - target->pos.y) < dist - c_p_MDL_box/2) ||
				!MAP_WALL_CLIPPED(wall) ||
				MAP_WALL_TEXTURE(wall) == MAP_WALL_water
		){
			if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_mine") > 0)
			{
				//выбираем наугад ракету или мину
				pl->brain.weap = 1+xrand(2);
				if(pl->brain.weap == WEAP_MINE)
				{
					//мина
					player->dir = entity_direction_invert(player->dir);
				}
			}
			else
			{
				if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") > 0)
					pl->brain.weap = WEAP_MISSILE;
				else
					pl->brain.weap = WEAP_ARTILLERY;
				pl->brain.attack = true;
			}
		}
		else
		{ //противник за стеной, пытаемся пробиться через стену
			if(MAP_WALL_TEXTURE(wall) == MAP_WALL_W0)
				pl->brain.attack = false;              //сильная броня, не стреляем
			else
			{
				if(MAP_WALL_TEXTURE(wall)==MAP_WALL_brick)
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
							pl->brain.attack = true;
							if(pl->brain.weap == WEAP_MINE)
							{ //мина
								player->dir = entity_direction_invert(player->dir);
							}
						}
						else
						{
							if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") > 0)
								pl->brain.weap = WEAP_MISSILE;
							else
								pl->brain.weap = WEAP_ARTILLERY;
							pl->brain.attack = true;
						}
					}
				}
			}
			if(
					(dist-c_p_MDL_box/2 < radius)&&
					(wall == (MAP_WALL_W0 | MAP_WALL_CLIP) || wall == (MAP_WALL_W1 | MAP_WALL_CLIP))
			) pl->brain.attack = false;
		}
	}
	else
	{
		if(
				(player->pos.y - c_p_MDL_box/2 < target->pos.y)&&
				(target->pos.y < player->pos.y + c_p_MDL_box/2)
		)
		{
			if(target->pos.x<player->pos.x) player->dir = DIR_LEFT;
			else                            player->dir = DIR_RIGHT;
			map_clip_find_near_wall(&player->pos, player->dir, &dist, &wall);
			if(
					//противник в прямой видимости
					(VEC_ABS(player->pos.x - target->pos.x) < dist - c_p_MDL_box/2)||
					!MAP_WALL_CLIPPED(wall) ||
					MAP_WALL_TEXTURE(wall) == MAP_WALL_water
			)
			{
				if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_mine") > 0)
				{
					pl->brain.weap = 1+xrand(2);                               //выбираем наугад ракету или мину
					if(pl->brain.weap == WEAP_MINE)
					{                             //мина
						player->dir = entity_direction_invert(player->dir);
					};
				}
				else
				{
					if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") > 0)
						pl->brain.weap = WEAP_MISSILE;
					else
						pl->brain.weap = WEAP_ARTILLERY;
					pl->brain.attack = true;
				}
			}
			else {                                                         //противник за стеной, пытаемся пробиться через стену
				if(MAP_WALL_TEXTURE(wall)==MAP_WALL_W0) pl->brain.attack = false;             //сильная броня, не стреляем
				else {
					if(MAP_WALL_TEXTURE(wall)==MAP_WALL_brick)
					{ //кирпич
						pl->brain.weap = WEAP_ARTILLERY;
						pl->brain.attack = true;
					}
					else {
						if(MAP_WALL_TEXTURE(wall)==MAP_WALL_W1) {                           //слабая броня
							if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_mine") > 0)
							{
								pl->brain.weap = 1+xrand(2);                            //выбираем наугад ракету или мину
								if(pl->brain.weap == WEAP_MINE)
								{ //мина
									player->dir = entity_direction_invert(player->dir);
								}
							}
							else
							{
								if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") > 0)
									pl->brain.weap = WEAP_MISSILE;
								else
									pl->brain.weap = WEAP_ARTILLERY;
								pl->brain.attack = true;
							}
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
		else
		{
			if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") > 0)
			{
				map_clip_find_near_wall(&player->pos, player->dir, &dist, &wall);
				if(
						(dist-c_p_MDL_box/2 < radius) &&
						((wall==MAP_WALL_W0+MAP_WALL_CLIP) || (wall==MAP_WALL_W1+MAP_WALL_CLIP))
				)
					pl->brain.attack = false;
				else
				{
					pl->brain.weap = WEAP_ARTILLERY;
					pl->brain.attack = true;
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
			(160<VEC_ABS(player->pos.x-target->pos.x))||
			(100<VEC_ABS(player->pos.y-target->pos.y))
	)
	{
		pl->attack = false;
		return;
	};
	if(pl->brain.count == 0) {
		player->dir = xrand(4);
		pl->move.go = true;
	}
	else
	{
		if(c_BOT_time+xrand(c_BOT_time)<pl->brain.count)
		{
			if(VEC_ABS(player->pos.x-target->pos.x)>VEC_ABS(player->pos.y-target->pos.y))
			{
				if(player->pos.x<target->pos.x)
					player->dir = DIR_RIGHT;
				else
					player->dir = DIR_LEFT;
			}
			else
			{
				if(player->pos.y < target->pos.y)
					player->dir = DIR_UP;
				else
					player->dir = DIR_DOWN;
			}
			if(
					(VEC_ABS(player->pos.x-target->pos.x) < c_BOT_dist)&&
					(VEC_ABS(player->pos.y-target->pos.y) < c_BOT_dist)
			)
				pl->move.go = false;
			else
				pl->move.go = true;
		}
	}
	pl->brain.count += dtime;
	if(c_BOT_time * 2 < pl->brain.count) pl->brain.count = 0;
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
	ctrl_AI_checkdanger(player);
	if(!pl->brain.danger)
	{
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
}
