/*
 *        Ubivat Tank
 *        управление игроками
 * by Master San
 */
#include "ent_bull.h"
#include "ent_explode.h"
#include "ent_player.h"

#include "types.h"
#include "game.h"
#include "weap.h"
#include "map.h"
#include "client.h"

#include <stdlib.h>
#include <types.h>

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
		entity_t * ent = entity_getfirst(list[i]);

		for(; ent; ent = ent->next)
		{
			if(danger) break;
			if(BULL(ent)->owner == player) continue;

			explodeinfo_t * explodeinfo = &explodeinfo_table[entity_bull_type_to_explode_type(BULL(ent)->type)];
			bullinfo_t * bullinfo = &bullinfo_table[BULL(ent)->type];

			//верхняя ближайшая стена
			map_clip_find_near(&player->pos, 0, DIR_UP, 0xF0, 100, &Udist);
			//нижняя ближайшая стена
			map_clip_find_near(&player->pos, 0, DIR_DOWN, 0xF0, 100, &Ddist);
			//левая ближайшая стена
			map_clip_find_near(&player->pos, 0, DIR_LEFT, 0xF0, 160, &Ldist);
			//правая ближайшая стена
			map_clip_find_near(&player->pos, 0, DIR_RIGHT, 0xF0, 160, &Rdist);
			Ud = (player->pos.y + Udist - c_p_MDL_box/2) - (ent->pos.y + explodeinfo->radius);
			Dd = (ent->pos.y - explodeinfo->radius) - (player->pos.y-Ddist + c_p_MDL_box / 2);
			Rd = (player->pos.x + Rdist - c_p_MDL_box/2) - (ent->pos.x + explodeinfo->radius);
			Ld = (ent->pos.x - explodeinfo->radius) - (player->pos.x - Ldist + c_p_MDL_box / 2);
			if(
					(player->pos.x-c_p_MDL_box/2 <= ent->pos.x+ explodeinfo->radius)&&
					(ent->pos.x- explodeinfo->radius <= player->pos.x+c_p_MDL_box/2)&&
					(VEC_ABS(player->pos.y-ent->pos.y) < 128)
			)
			{
				if(
						(ent->dir == DIR_UP || bullinfo->speed < 0) &&
						(VEC_ABS(player->pos.y-ent->pos.y)<Ddist) &&
						(ent->pos.y + explodeinfo->radius < player->pos.y-c_p_MDL_box/2)
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
							(ent->dir == DIR_DOWN ||  bullinfo->speed < 0) &&
							(VEC_ABS(player->pos.y-ent->pos.y)<Udist) &&
							(player->pos.y+c_p_MDL_box/2 < ent->pos.y - explodeinfo->radius)
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
						(player->pos.y - c_p_MDL_box/2 <= ent->pos.y + explodeinfo->radius) &&
						(ent->pos.y - explodeinfo->radius <= player->pos.y + c_p_MDL_box/2) &&
						(VEC_ABS(player->pos.x - ent->pos.x) < 128)
				)
				{
					if (
							(ent->dir == DIR_LEFT || bullinfo->speed < 0)&&
							(VEC_ABS(player->pos.x-ent->pos.x)<Rdist)&&
							(player->pos.x+c_p_MDL_box/2 < ent->pos.x- explodeinfo->radius)
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
								(ent->dir == DIR_RIGHT || bullinfo->speed < 0) &&
								(VEC_ABS(player->pos.x - ent->pos.x)<Ldist) &&
								(ent->pos.x + explodeinfo->radius < player->pos.x - c_p_MDL_box / 2)
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

	explodeinfo_t * explodeinfo = &explodeinfo_table[
													entity_weapon_type_to_bull_type(
														entity_weapon_type_to_bull_type(pl->brain.weap)
													)
													];

	float dist;
	char wall;
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
				(wall       ) == MAP_WALL_W0   ||
				(wall       ) == MAP_WALL_w1   ||
				(wall & 0x0F) == MAP_WALL_water  ||
				(wall       ) == c_m_f_clip
		){
			if(pl->items[ITEM_AMMO_MINE] > 0)
			{
				//выбираем наугад ракету или мину
				pl->brain.weap = 1+xrand(2);
				if(pl->brain.weap == WEAP_MINE)
				{
					//мина
					if(player->dir == DIR_DOWN)
						player->dir = DIR_UP;
					else
						player->dir = DIR_DOWN;
				}
			}
			else
			{
				if(pl->items[ITEM_AMMO_MISSILE] > 0)
					pl->brain.weap = WEAP_MISSILE;
				else
					pl->brain.weap = WEAP_ARTILLERY;
				pl->brain.attack = true;
			}
		}
		else
		{ //противник за стеной, пытаемся пробиться через стену
			if((wall & 0x0F) == MAP_WALL_W0)
				pl->brain.attack = false;              //сильная броня, не стреляем
			else
			{
				if((wall & 0x0F)==MAP_WALL_brick)
				{
					pl->brain.weap = WEAP_ARTILLERY;          //кирпич
					pl->brain.attack = true;
				}
				else
				{
					if((wall & 0x0F) == MAP_WALL_w1)
					{                            //слабая броня
						if(pl->items[ITEM_AMMO_MINE] > 0)
						{
							pl->brain.weap = 1+xrand(2);                             //выбираем наугад ракету или мину
							pl->brain.attack = true;
							if(pl->brain.weap == WEAP_MINE)
							{                           //мина
								if(player->dir == DIR_DOWN)
									player->dir = DIR_UP;
								else
									player->dir = DIR_DOWN;
							}
						}
						else
						{
							if(pl->items[ITEM_AMMO_MISSILE] > 0)
								pl->brain.weap = WEAP_MISSILE;
							else
								pl->brain.weap = WEAP_ARTILLERY;
							pl->brain.attack = true;
						}
					}
				}
			}
			if(
					(dist-c_p_MDL_box/2 < explodeinfo->radius)&&
					(wall == (MAP_WALL_W0 | c_m_f_clip) || wall == (MAP_WALL_w1 | c_m_f_clip))
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
			else                                      player->dir = DIR_RIGHT;
			map_clip_find_near_wall(&player->pos, player->dir, &dist, &wall);
			if(
					//противник в прямой видимости
					(VEC_ABS(player->pos.x - target->pos.x) < dist - c_p_MDL_box/2)||
					(wall       ) == MAP_WALL_W0  ||
					(wall       ) == MAP_WALL_w1  ||
					(wall & 0x0F) == MAP_WALL_water ||
					(wall       ) == c_m_f_clip
			)
			{
				if(pl->items[ITEM_AMMO_MINE] > 0)
				{
					pl->brain.weap = 1+xrand(2);                               //выбираем наугад ракету или мину
					if(pl->brain.weap == WEAP_MINE)
					{                             //мина
						if(player->dir == DIR_LEFT) player->dir = DIR_RIGHT;
						else                        player->dir = DIR_LEFT;
					};
				}
				else
				{
					if(pl->items[ITEM_AMMO_MISSILE] > 0)
						pl->brain.weap = WEAP_MISSILE;
					else
						pl->brain.weap = WEAP_ARTILLERY;
					pl->brain.attack = true;
				}
			}
			else {                                                         //противник за стеной, пытаемся пробиться через стену
				if((wall & 0x0F)==MAP_WALL_W0) pl->brain.attack = false;             //сильная броня, не стреляем
				else {
					if((wall & 0x0F)==MAP_WALL_brick)
					{ //кирпич
						pl->brain.weap = WEAP_ARTILLERY;
						pl->brain.attack = true;
					}
					else {
						if((wall & 0x0F)==MAP_WALL_w1) {                           //слабая броня
							if(pl->items[ITEM_AMMO_MINE] > 0)
							{
								pl->brain.weap = 1+xrand(2);                            //выбираем наугад ракету или мину
								if(pl->brain.weap == WEAP_MINE)
								{ //мина
									if(player->dir == DIR_LEFT)
										player->dir = DIR_RIGHT;
									else
										player->dir = DIR_LEFT;
								}
							}
							else
							{
								if(pl->items[ITEM_AMMO_MISSILE] > 0)
									pl->brain.weap = WEAP_MISSILE;
								else
									pl->brain.weap = WEAP_ARTILLERY;
								pl->brain.attack = true;
							}
						}
					}
				}
				if(
						(dist-c_p_MDL_box/2 < explodeinfo->radius) &&
						((wall==MAP_WALL_W0+c_m_f_clip) || (wall==MAP_WALL_w1+c_m_f_clip))
				)
					pl->brain.attack = false;
			}
		}
		else
		{
			if(pl->items[ITEM_AMMO_MISSILE] > 0)
			{
				map_clip_find_near_wall(&player->pos, player->dir, &dist, &wall);
				if(
						(dist-c_p_MDL_box/2<explodeinfo_table[1].radius) &&
						((wall==MAP_WALL_W0+c_m_f_clip) || (wall==MAP_WALL_w1+c_m_f_clip))
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
	if(pl->brain.weap == WEAP_MISSILE) pl->brain.target = target; //если выпустил ракету, тогда цель постоянная
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
void think_enemy(struct entlink_s * player)
{
	player_t * pl = player->data;

	if(debug_noAI)return;
	if(pl->items[ITEM_HEALTH] > 0)
	{
		ctrl_AI_checkdanger(player);
		if(!pl->brain.danger)
		{
			int client_num = client_num_get();

			int id = xrand(client_num);
			entity_t * target = client_get(id)->entity;

			player_t * enemy_pl = target->data;

			if(enemy_pl->items[ITEM_HEALTH] <= 0) pl->attack = false;
			else
			{
				if(!pl->bull && !pl->attack) ctrl_AI_findenemy(player, target);
				ctrl_AI_attack(player, target);
			};
		}
	}
}

/*
 * управление игроком
 */
void think_human(entity_t * player)
{

};
