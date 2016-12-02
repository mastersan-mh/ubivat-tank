/*
 *        Ubivat Tank
 *        управление игроками
 * by Master San
 */
#include <game.h>
#include <weap.h>
#include "explode.h"
#include <map.h>
#include <player.h>

#include <stdlib.h>
#include <types.h>

/*
 * инициализация AI
 */
void ctrl_AI_init(think_t * brain)
{
	brain->danger  = 0;
	brain->Fdanger = 0;
	brain->weapon  = 0;
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
static void ctrl_AI_checkdanger(player_t * player)
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

		weapon_info_t * weapinfo = &wtable[bull->bull._weap_];

		explodeinfo_t * explodeinfo = &explodeinfo_table[bull->bull._weap_];

		//верхняя ближайшая стена
		map_clip_find_near(&player->move.pos, 0, DIR_UP, 0xF0, 100, &Udist);
		//нижняя ближайшая стена
		map_clip_find_near(&player->move.pos, 0, DIR_DOWN, 0xF0, 100, &Ddist);
		//левая ближайшая стена
		map_clip_find_near(&player->move.pos, 0, DIR_LEFT, 0xF0, 160, &Ldist);
		//правая ближайшая стена
		map_clip_find_near(&player->move.pos, 0, DIR_RIGHT, 0xF0, 160, &Rdist);
		Ud = (player->move.pos.y+Udist-c_p_MDL_box/2)-(bull->pos.y+ explodeinfo->radius);
		Dd = (bull->pos.y- explodeinfo->radius)-(player->move.pos.y-Ddist+c_p_MDL_box/2);
		Rd = (player->move.pos.x+Rdist-c_p_MDL_box/2)-(bull->pos.x+ explodeinfo->radius);
		Ld = (bull->pos.x- explodeinfo->radius)-(player->move.pos.x-Ldist+c_p_MDL_box/2);
		if(
				(player->move.pos.x-c_p_MDL_box/2 <= bull->pos.x+ explodeinfo->radius)&&
				(bull->pos.x- explodeinfo->radius <= player->move.pos.x+c_p_MDL_box/2)&&
				(abs(player->move.pos.y-bull->pos.y) < 128)
		)
		{
			if(
					(bull->dir == DIR_UP || weapinfo->bullspeed < 0) &&
					(abs(player->move.pos.y-bull->pos.y)<Ddist) &&
					(bull->pos.y+ explodeinfo->radius<player->move.pos.y-c_p_MDL_box/2)
			)
			{
				danger = true;
				if(!player->brain.Fdanger)
				{
					player->move.go = true;
					player->brain.Fdanger = true;
					if(0 <= Ld && 0 <= Rd) player->move.dir = xrand(2) + 2;
					else
						if(0 < Ld) player->move.dir = DIR_LEFT;
						else
							if(0<Rd) player->move.dir = DIR_RIGHT;
				}
			}
			else
			{
				if(
						(bull->dir == DIR_DOWN ||  weapinfo->bullspeed < 0) &&
						(abs(player->move.pos.y-bull->pos.y)<Udist) &&
						(player->move.pos.y+c_p_MDL_box/2<bull->pos.y- explodeinfo->radius)
				)
				{
					danger = true;
					if(!player->brain.Fdanger) {
						player->move.go = true;
						player->brain.Fdanger = true;
						if( 0 <= Ld && 0 <= Rd) player->move.dir = xrand(2)+2;
						else
							if(0 < Ld) player->move.dir = DIR_LEFT;
							else
								if(0<Rd) player->move.dir = DIR_RIGHT;
					}
				}
			}
		}
		else {
			if(
					(player->move.pos.y - c_p_MDL_box/2 <= bull->pos.y + explodeinfo->radius) &&
					(bull->pos.y - explodeinfo->radius <= player->move.pos.y + c_p_MDL_box/2) &&
					(abs(player->move.pos.x - bull->pos.x) < 128)
			)
			{
				if (
						(bull->dir == DIR_LEFT || weapinfo->bullspeed < 0)&&
						(abs(player->move.pos.x-bull->pos.x)<Rdist)&&
						(player->move.pos.x+c_p_MDL_box/2<bull->pos.x- explodeinfo->radius)
				)
				{
					danger = true;
					if(!player->brain.Fdanger)
					{
						player->move.go = true;
						player->brain.Fdanger = true;
						if(0 <= Ud && 0 <= Dd) player->move.dir = xrand(2);
						else
							if(0 < Ud) player->move.dir = DIR_UP;
							else
								if(0 < Dd) player->move.dir = DIR_DOWN;
					}
				}
				else
				{
					if(
							(bull->dir == DIR_RIGHT || weapinfo->bullspeed < 0) &&
							(abs(player->move.pos.x-bull->pos.x)<Ldist) &&
							(bull->pos.x + explodeinfo->radius < player->move.pos.x - c_p_MDL_box / 2)
					)
					{
						danger = true;
						if(!player->brain.Fdanger)
						{
							player->move.go = true;
							player->brain.Fdanger = true;
							if(0 <= Ud && 0 <= Dd) player->move.dir = xrand(2);
							else
								if(0 < Ud) player->move.dir = DIR_UP;
								else
									if(0 < Dd) player->move.dir = DIR_DOWN;
						}
					}
				}
			}
		}
	} // end for;
	player->brain.danger = danger;
	if(!player->brain.danger)
	{
		player->brain.Fdanger = false;
		//player->move.go = false;
	}
}
/*
 * атака
 */
static void ctrl_AI_attack(player_t * player, player_t * target)
{
	weapon_info_t * weap_info = &wtable[player->brain.weapon - 1];
	explodeinfo_t * explode_info = &explodeinfo_table[player->brain.weapon - 1];

	float dist;
	char wall;
	if( player->bull && player->brain.target )
	{
		if(
				abs(player->bull->pos.y - player->brain.target->move.pos.y) <
				abs(player->bull->pos.x - player->brain.target->move.pos.x)
		)
		{
			if(player->bull->pos.x < player->brain.target->move.pos.x)
				player->move.dir = DIR_RIGHT;
			else
				player->move.dir = DIR_LEFT;
		}
		else
		{
			if(player->bull->pos.y < player->brain.target->move.pos.y)
				player->move.dir = DIR_UP;
			else
				player->move.dir = DIR_DOWN;
		}
		return;
	}
	if
	(
			abs(player->move.pos.x - target->move.pos.x) > 160 ||
			abs(player->move.pos.y - target->move.pos.y) > 100
	)
	{
		player->w.attack = 0;
		return;
	};
	//если оружие не перезарядилось
	if(0 < player->w.reloadtime_d) return;
	player->brain.target = NULL;
	if(
			(player->move.pos.x-c_p_MDL_box/2<target->move.pos.x) &&
			(target->move.pos.x<player->move.pos.x+c_p_MDL_box/2)
	)
	{
		if(target->move.pos.y < player->move.pos.y)
			player->move.dir = DIR_DOWN;
		else
			player->move.dir = DIR_UP;
		map_clip_find_near_wall(&player->move.pos, player->move.dir, &dist, &wall);
		if(
				//противник в прямой видимости
				(abs(player->move.pos.y - target->move.pos.y) < dist - c_p_MDL_box/2) ||
				(wall       ) == c_m_w_w0   ||
				(wall       ) == c_m_w_w1   ||
				(wall & 0x0F) == c_m_water  ||
				(wall       ) == c_m_f_clip
		){
			if(player->w.ammo[2]>0)
			{
				//выбираем наугад ракету или мину
				player->brain.weapon = 1+1+xrand(2);
				if(player->brain.weapon == 1+2)
				{
					//мина
					if(player->move.dir == DIR_DOWN)
						player->move.dir = DIR_UP;
					else
						player->move.dir = DIR_DOWN;
				}
			}
			else {
				if(player->w.ammo[1]>0)
					player->brain.weapon = 1+1;
				else
					player->brain.weapon = 1;
			}
		}
		else
		{ //противник за стеной, пытаемся пробиться через стену
			if((wall & 0x0F)==c_m_w_w0) player->brain.weapon = 0;              //сильная броня, не стреляем
			else {
				if((wall & 0x0F)==c_m_w_brick) player->brain.weapon = 1;          //кирпич
				else {
					if((wall & 0x0F)==c_m_w_w1) {                            //слабая броня
						if(player->w.ammo[2]>0) {
							player->brain.weapon = 1+1+xrand(2);                             //выбираем наугад ракету или мину
							if(player->brain.weapon == 1+2) {                           //мина
								if(player->move.dir == DIR_DOWN)
									player->move.dir = DIR_UP;
								else
									player->move.dir = DIR_DOWN;
							}
						}
						else
						{
							if(player->w.ammo[1]>0)
								player->brain.weapon = 1+1;
							else
								player->brain.weapon = 0;
						}
					}
				}
			}
			if(
					(dist-c_p_MDL_box/2 < explode_info->radius)&&
					(wall == (c_m_w_w0 | c_m_f_clip) || wall == (c_m_w_w1 | c_m_f_clip))
			) player->brain.weapon = 0;
		}
	}
	else
	{
		if(
				(player->move.pos.y - c_p_MDL_box/2 < target->move.pos.y)&&
				(target->move.pos.y < player->move.pos.y + c_p_MDL_box/2)
		)
		{
			if(target->move.pos.x<player->move.pos.x) player->move.dir = DIR_LEFT;
			else                                      player->move.dir = DIR_RIGHT;
			map_clip_find_near_wall(&player->move.pos, player->move.dir, &dist, &wall);
			if(
					//противник в прямой видимости
					(abs(player->move.pos.x - target->move.pos.x) < dist - c_p_MDL_box/2)||
					(wall       ) == c_m_w_w0  ||
					(wall       ) == c_m_w_w1  ||
					(wall & 0x0F) == c_m_water ||
					(wall       ) == c_m_f_clip
			)
			{
				if(player->w.ammo[2]>0)
				{
					player->brain.weapon = 1+1+xrand(2);                               //выбираем наугад ракету или мину
					if(player->brain.weapon == 1+2) {                             //мина
						if(player->move.dir == DIR_LEFT) player->move.dir = DIR_RIGHT;
						else                             player->move.dir = DIR_LEFT;
					};
				}
				else {
					if(player->w.ammo[1]>0) player->brain.weapon = 1+1;
					else player->brain.weapon = 1;
				};
			}
			else {                                                         //противник за стеной, пытаемся пробиться через стену
				if((wall & 0x0F)==c_m_w_w0) player->brain.weapon = 0;             //сильная броня, не стреляем
				else {
					if((wall & 0x0F)==c_m_w_brick) player->brain.weapon = 1;         //кирпич
					else {
						if((wall & 0x0F)==c_m_w_w1) {                           //слабая броня
							if(player->w.ammo[2]>0) {
								player->brain.weapon = 1+1+xrand(2);                            //выбираем наугад ракету или мину
								if(player->brain.weapon == 1+2)
								{ //мина
									if(player->move.dir == DIR_LEFT)
										player->move.dir = DIR_RIGHT;
									else
										player->move.dir = DIR_LEFT;
								};
							}
							else {
								if(player->w.ammo[1] > 0)
									player->brain.weapon = 1+1;
								else
									player->brain.weapon = 0;
							}
						}
					}
				}
				if(
						(dist-c_p_MDL_box/2 < explode_info->radius) &&
						((wall==c_m_w_w0+c_m_f_clip) || (wall==c_m_w_w1+c_m_f_clip))
				)
					player->brain.weapon = 0;
			}
		}
		else
		{
			if(player->w.ammo[1]>0) {
				map_clip_find_near_wall(&player->move.pos,player->move.dir, &dist, &wall);
				if(
						(dist-c_p_MDL_box/2<explodeinfo_table[1].radius) &&
						((wall==c_m_w_w0+c_m_f_clip) || (wall==c_m_w_w1+c_m_f_clip))
				)
					player->brain.weapon = 0;
				else
					player->brain.weapon = 1+1;
			}
		}
	}
	player->w.attack = player->brain.weapon;
	if(player->brain.weapon == 2) player->brain.target = target; //если выпустил ракету, тогда цель постоянная
	player->brain.weapon = 0;
}
/*
 * поиск врага
 */
static void ctrl_AI_findenemy(player_t * player, player_t * target)
{
	if(
			(160<abs(player->move.pos.x-target->move.pos.x))||
			(100<abs(player->move.pos.y-target->move.pos.y))
	)
	{
		player->w.attack = 0;
		return;
	};
	if(player->brain.count == 0) {
		player->move.dir = xrand(4);
		player->move.go = true;
	}
	else
	{
		if(c_BOT_time+xrand(c_BOT_time)<player->brain.count)
		{
			if(abs(player->move.pos.x-target->move.pos.x)>abs(player->move.pos.y-target->move.pos.y))
			{
				if(player->move.pos.x<target->move.pos.x)
					player->move.dir = DIR_RIGHT;
				else
					player->move.dir = DIR_LEFT;
			}
			else
			{
				if(player->move.pos.y < target->move.pos.y)
					player->move.dir = DIR_UP;
				else
					player->move.dir = DIR_DOWN;
			}
			if(
					(abs(player->move.pos.x-target->move.pos.x)<c_BOT_dist)&&
					(abs(player->move.pos.y-target->move.pos.y)<c_BOT_dist)
			)
				player->move.go = false;
			else
				player->move.go = true;
		}
	}
	player->brain.count += dtime;
	if(c_BOT_time*2<player->brain.count) player->brain.count = 0;
}

/*
 * управление вражеским игроком
 */
void think_enemy(struct player_s * player)
{
	if(debug_noAI)return;
	if(0<player->charact.health)
	{
		ctrl_AI_checkdanger(player);
		if(!player->brain.danger)
		{
			if(!game.P1)
			{
				if(game.P0->charact.health <= 0) player->w.attack = 0;
				else {
					if(!player->bull &&  !player->w.attack) ctrl_AI_findenemy(player, game.P0);
					ctrl_AI_attack(player, game.P0);
				};
			}
			else
			{
				if(xrand(2) == 0)
				{
					if(0 < game.P0->charact.health)
					{
						if( !player->bull && !player->w.attack) ctrl_AI_findenemy(player,game.P0);
						ctrl_AI_attack(player, game.P0);
					}
				}
				else
				{
					if(0 < game.P1->charact.health)
					{
						if(!player->bull && !player->w.attack) ctrl_AI_findenemy(player,game.P1);
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
void think_human(int Pnum, player_t * player)
{

};
