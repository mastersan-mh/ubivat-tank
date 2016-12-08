/*
 *        Ubivat Tank
 *        управление игроками
 * by Master San
 */
#include "types.h"
#include "game.h"
#include "weap.h"
#include "bull.h"
#include "explode.h"
#include "map.h"
#include "player.h"

#include <stdlib.h>
#include <types.h>

/*
 * инициализация AI
 */
void ctrl_AI_init(think_t * brain)
{
	brain->danger  = 0;
	brain->Fdanger = 0;
	brain->attack  = false;
	brain->target  = NULL;
	brain->count   = xrand(c_BOT_time*2);
};
/*
 * удаление AI
 */
void ctrl_AI_done(think_t * brain)
{
	brain->target = NULL;
};
/*
 * уворачивание от снарядов
 */
static void ctrl_AI_checkdanger(mobj_t * player)
{
	float Udist;
	float Ddist;
	float Ldist;
	float Rdist;
	float Ud;
	float Dd;
	float Ld;
	float Rd;


	bool danger = false;

	mobj_t * bull;

	for(bull = map.mobjs; bull; bull = bull->next)
	{
		if(danger) break;
		if(bull->type != MOBJ_BULL) continue;
		if(bull->bull.owner == player) continue;

		explodeinfo_t * explodeinfo = &explodeinfo_table[mobj_bull_type_to_explode_type(bull->bull.type)];
		bullinfo_t * bullinfo = &bullinfo_table[bull->bull.type];

		//верхняя ближайшая стена
		map_clip_find_near(&player->pos, 0, DIR_UP, 0xF0, 100, &Udist);
		//нижняя ближайшая стена
		map_clip_find_near(&player->pos, 0, DIR_DOWN, 0xF0, 100, &Ddist);
		//левая ближайшая стена
		map_clip_find_near(&player->pos, 0, DIR_LEFT, 0xF0, 160, &Ldist);
		//правая ближайшая стена
		map_clip_find_near(&player->pos, 0, DIR_RIGHT, 0xF0, 160, &Rdist);
		Ud = (player->pos.y + Udist - c_p_MDL_box/2) - (bull->pos.y + explodeinfo->radius);
		Dd = (bull->pos.y - explodeinfo->radius) - (player->pos.y-Ddist + c_p_MDL_box / 2);
		Rd = (player->pos.x + Rdist - c_p_MDL_box/2) - (bull->pos.x + explodeinfo->radius);
		Ld = (bull->pos.x - explodeinfo->radius) - (player->pos.x - Ldist + c_p_MDL_box / 2);
		if(
				(player->pos.x-c_p_MDL_box/2 <= bull->pos.x+ explodeinfo->radius)&&
				(bull->pos.x- explodeinfo->radius <= player->pos.x+c_p_MDL_box/2)&&
				(abs(player->pos.y-bull->pos.y) < 128)
		)
		{
			if(
					(bull->dir == DIR_UP || bullinfo->speed < 0) &&
					(abs(player->pos.y-bull->pos.y)<Ddist) &&
					(bull->pos.y + explodeinfo->radius < player->pos.y-c_p_MDL_box/2)
			)
			{
				danger = true;
				if(!player->player->brain.Fdanger)
				{
					player->player->move.go = true;
					player->player->brain.Fdanger = true;
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
						(bull->dir == DIR_DOWN ||  bullinfo->speed < 0) &&
						(abs(player->pos.y-bull->pos.y)<Udist) &&
						(player->pos.y+c_p_MDL_box/2 < bull->pos.y - explodeinfo->radius)
				)
				{
					danger = true;
					if(!player->player->brain.Fdanger) {
						player->player->move.go = true;
						player->player->brain.Fdanger = true;
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
					(player->pos.y - c_p_MDL_box/2 <= bull->pos.y + explodeinfo->radius) &&
					(bull->pos.y - explodeinfo->radius <= player->pos.y + c_p_MDL_box/2) &&
					(abs(player->pos.x - bull->pos.x) < 128)
			)
			{
				if (
						(bull->dir == DIR_LEFT || bullinfo->speed < 0)&&
						(abs(player->pos.x-bull->pos.x)<Rdist)&&
						(player->pos.x+c_p_MDL_box/2 < bull->pos.x- explodeinfo->radius)
				)
				{
					danger = true;
					if(!player->player->brain.Fdanger)
					{
						player->player->move.go = true;
						player->player->brain.Fdanger = true;
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
							(bull->dir == DIR_RIGHT || bullinfo->speed < 0) &&
							(abs(player->pos.x - bull->pos.x)<Ldist) &&
							(bull->pos.x + explodeinfo->radius < player->pos.x - c_p_MDL_box / 2)
					)
					{
						danger = true;
						if(!player->player->brain.Fdanger)
						{
							player->player->move.go = true;
							player->player->brain.Fdanger = true;
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
	player->player->brain.danger = danger;
	if(!player->player->brain.danger)
	{
		player->player->brain.Fdanger = false;
		//player->move.go = false;
	}
}
/*
 * атака
 */
static void ctrl_AI_attack(mobj_t * player, mobj_t * target)
{
	explodeinfo_t * explodeinfo = &explodeinfo_table[
													mobj_weapon_type_to_bull_type(
														mobj_weapon_type_to_bull_type(player->player->brain.weap)
													)
													];

	float dist;
	char wall;
	if( player->player->bull && player->player->brain.target )
	{
		if(
				abs(player->player->bull->pos.y - player->player->brain.target->pos.y) <
				abs(player->player->bull->pos.x - player->player->brain.target->pos.x)
		)
		{
			if(player->player->bull->pos.x < player->player->brain.target->pos.x)
				player->dir = DIR_RIGHT;
			else
				player->dir = DIR_LEFT;
		}
		else
		{
			if(player->player->bull->pos.y < player->player->brain.target->pos.y)
				player->dir = DIR_UP;
			else
				player->dir = DIR_DOWN;
		}
		return;
	}
	if
	(
			abs(player->pos.x - target->pos.x) > 160.0 ||
			abs(player->pos.y - target->pos.y) > 100.0
	)
	{
		player->player->attack = false;
		return;
	};
	//если оружие не перезарядилось
	if(0 < player->player->reloadtime_d) return;
	player->player->brain.target = NULL;
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
				(abs(player->pos.y - target->pos.y) < dist - c_p_MDL_box/2) ||
				(wall       ) == c_m_w_w0   ||
				(wall       ) == c_m_w_w1   ||
				(wall & 0x0F) == c_m_water  ||
				(wall       ) == c_m_f_clip
		){
			if(player->player->items[ITEM_AMMO_MINE] > 0)
			{
				//выбираем наугад ракету или мину
				player->player->brain.weap = 1+xrand(2);
				if(player->player->brain.weap == WEAP_MINE)
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
				if(player->player->items[ITEM_AMMO_MISSILE] > 0)
					player->player->brain.weap = WEAP_MISSILE;
				else
					player->player->brain.weap = WEAP_ARTILLERY;
				player->player->brain.attack = true;
			}
		}
		else
		{ //противник за стеной, пытаемся пробиться через стену
			if((wall & 0x0F) == c_m_w_w0)
				player->player->brain.attack = false;              //сильная броня, не стреляем
			else
			{
				if((wall & 0x0F)==c_m_w_brick)
				{
					player->player->brain.weap = WEAP_ARTILLERY;          //кирпич
					player->player->brain.attack = true;
				}
				else
				{
					if((wall & 0x0F) == c_m_w_w1)
					{                            //слабая броня
						if(player->player->items[ITEM_AMMO_MINE] > 0)
						{
							player->player->brain.weap = 1+xrand(2);                             //выбираем наугад ракету или мину
							player->player->brain.attack = true;
							if(player->player->brain.weap == WEAP_MINE)
							{                           //мина
								if(player->dir == DIR_DOWN)
									player->dir = DIR_UP;
								else
									player->dir = DIR_DOWN;
							}
						}
						else
						{
							if(player->player->items[ITEM_AMMO_MISSILE] > 0)
								player->player->brain.weap = WEAP_MISSILE;
							else
								player->player->brain.weap = WEAP_ARTILLERY;
							player->player->brain.attack = true;
						}
					}
				}
			}
			if(
					(dist-c_p_MDL_box/2 < explodeinfo->radius)&&
					(wall == (c_m_w_w0 | c_m_f_clip) || wall == (c_m_w_w1 | c_m_f_clip))
			) player->player->brain.attack = false;
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
					(abs(player->pos.x - target->pos.x) < dist - c_p_MDL_box/2)||
					(wall       ) == c_m_w_w0  ||
					(wall       ) == c_m_w_w1  ||
					(wall & 0x0F) == c_m_water ||
					(wall       ) == c_m_f_clip
			)
			{
				if(player->player->items[ITEM_AMMO_MINE] > 0)
				{
					player->player->brain.weap = 1+xrand(2);                               //выбираем наугад ракету или мину
					if(player->player->brain.weap == WEAP_MINE)
					{                             //мина
						if(player->dir == DIR_LEFT) player->dir = DIR_RIGHT;
						else                        player->dir = DIR_LEFT;
					};
				}
				else
				{
					if(player->player->items[ITEM_AMMO_MISSILE] > 0)
						player->player->brain.weap = WEAP_MISSILE;
					else
						player->player->brain.weap = WEAP_ARTILLERY;
					player->player->brain.attack = true;
				}
			}
			else {                                                         //противник за стеной, пытаемся пробиться через стену
				if((wall & 0x0F)==c_m_w_w0) player->player->brain.attack = false;             //сильная броня, не стреляем
				else {
					if((wall & 0x0F)==c_m_w_brick)
					{ //кирпич
						player->player->brain.weap = WEAP_ARTILLERY;
						player->player->brain.attack = true;
					}
					else {
						if((wall & 0x0F)==c_m_w_w1) {                           //слабая броня
							if(player->player->items[ITEM_AMMO_MINE] > 0)
							{
								player->player->brain.weap = 1+xrand(2);                            //выбираем наугад ракету или мину
								if(player->player->brain.weap == WEAP_MINE)
								{ //мина
									if(player->dir == DIR_LEFT)
										player->dir = DIR_RIGHT;
									else
										player->dir = DIR_LEFT;
								}
							}
							else
							{
								if(player->player->items[ITEM_AMMO_MISSILE] > 0)
									player->player->brain.weap = WEAP_MISSILE;
								else
									player->player->brain.weap = WEAP_ARTILLERY;
								player->player->brain.attack = true;
							}
						}
					}
				}
				if(
						(dist-c_p_MDL_box/2 < explodeinfo->radius) &&
						((wall==c_m_w_w0+c_m_f_clip) || (wall==c_m_w_w1+c_m_f_clip))
				)
					player->player->brain.attack = false;
			}
		}
		else
		{
			if(player->player->items[ITEM_AMMO_MISSILE] > 0)
			{
				map_clip_find_near_wall(&player->pos, player->dir, &dist, &wall);
				if(
						(dist-c_p_MDL_box/2<explodeinfo_table[1].radius) &&
						((wall==c_m_w_w0+c_m_f_clip) || (wall==c_m_w_w1+c_m_f_clip))
				)
					player->player->brain.attack = false;
				else
				{
					player->player->brain.weap = WEAP_ARTILLERY;
					player->player->brain.attack = true;
				}
			}
		}
	}
	player->player->weap = player->player->brain.weap;
	player->player->attack = player->player->brain.attack;
	if(player->player->brain.weap == WEAP_MISSILE) player->player->brain.target = target; //если выпустил ракету, тогда цель постоянная
	player->player->brain.attack = false;
}
/*
 * поиск врага
 */
static void ctrl_AI_findenemy(mobj_t * player, mobj_t * target)
{
	if(
			(160<abs(player->pos.x-target->pos.x))||
			(100<abs(player->pos.y-target->pos.y))
	)
	{
		player->player->attack = false;
		return;
	};
	if(player->player->brain.count == 0) {
		player->dir = xrand(4);
		player->player->move.go = true;
	}
	else
	{
		if(c_BOT_time+xrand(c_BOT_time)<player->player->brain.count)
		{
			if(abs(player->pos.x-target->pos.x)>abs(player->pos.y-target->pos.y))
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
					(abs(player->pos.x-target->pos.x) < c_BOT_dist)&&
					(abs(player->pos.y-target->pos.y) < c_BOT_dist)
			)
				player->player->move.go = false;
			else
				player->player->move.go = true;
		}
	}
	player->player->brain.count += dtime;
	if(c_BOT_time * 2 < player->player->brain.count) player->player->brain.count = 0;
}

/*
 * управление вражеским игроком
 */
void think_enemy(struct mobj_s * player)
{
	if(debug_noAI)return;
	if(player->player->items[ITEM_HEALTH] > 0)
	{
		ctrl_AI_checkdanger(player);
		if(!player->player->brain.danger)
		{
			if(!game.P1)
			{
				if(game.P0->player->items[ITEM_HEALTH] <= 0) player->player->attack = false;
				else
				{
					if(!player->player->bull && !player->player->attack) ctrl_AI_findenemy(player, game.P0);
					ctrl_AI_attack(player, game.P0);
				};
			}
			else
			{
				if(xrand(2) == 0)
				{
					if(0 < game.P0->player->items[ITEM_HEALTH])
					{
						if( !player->player->bull && !player->player->attack) ctrl_AI_findenemy(player, game.P0);
						ctrl_AI_attack(player, game.P0);
					}
				}
				else
				{
					if(0 < game.P1->player->items[ITEM_HEALTH])
					{
						if(!player->player->bull && !player->player->attack) ctrl_AI_findenemy(player, game.P1);
						ctrl_AI_attack(player,game.P1);
					}
				}
			}
		}
	}
}

/*
 * управление игроком
 */
void think_human(int Pnum, struct mobj_s * player)
{

};
