/*
 *        Ubivat Tank
 *        управление игроками
 * by Master San
 */
#include <game.h>
#include <weap.h>
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
	bull_t * bull;
	float Udist;
	float Ddist;
	float Ldist;
	float Rdist;
	float Ud;
	float Dd;
	float Ld;
	float Rd;
	bool danger = false;
	bull = bullList;
	while(bull && !danger)
	{
		//не своя пуля
		if(bull->player != player)
		{
			//верхняя ближайшая стена
			map_clip_find_near(&player->move.orig, 0, DIR_UP, 0xF0, 100, &Udist);
			//нижняя ближайшая стена
			map_clip_find_near(&player->move.orig, 0, DIR_DOWN, 0xF0, 100, &Ddist);
			//левая ближайшая стена
			map_clip_find_near(&player->move.orig, 0, DIR_LEFT, 0xF0, 160, &Ldist);
			//правая ближайшая стена
			map_clip_find_near(&player->move.orig, 0, DIR_RIGHT, 0xF0, 160, &Rdist);
			Ud = (player->move.orig.y+Udist-c_p_MDL_box/2)-(bull->orig.y+wtable[bull->_weap_].radius);
			Dd = (bull->orig.y-wtable[bull->_weap_].radius)-(player->move.orig.y-Ddist+c_p_MDL_box/2);
			Rd = (player->move.orig.x+Rdist-c_p_MDL_box/2)-(bull->orig.x+wtable[bull->_weap_].radius);
			Ld = (bull->orig.x-wtable[bull->_weap_].radius)-(player->move.orig.x-Ldist+c_p_MDL_box/2);
			if(
					(player->move.orig.x-c_p_MDL_box/2 <= bull->orig.x+wtable[bull->_weap_].radius)&&
					(bull->orig.x-wtable[bull->_weap_].radius <= player->move.orig.x+c_p_MDL_box/2)&&
					(abs(player->move.orig.y-bull->orig.y) < 128)
			)
			{
				if(
						(bull->dir == DIR_UP || wtable[bull->_weap_].bullspeed < 0) &&
						(abs(player->move.orig.y-bull->orig.y)<Ddist) &&
						(bull->orig.y+wtable[bull->_weap_].radius<player->move.orig.y-c_p_MDL_box/2)
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
							(bull->dir == DIR_DOWN || wtable[bull->_weap_].bullspeed<0) &&
							(abs(player->move.orig.y-bull->orig.y)<Udist) &&
							(player->move.orig.y+c_p_MDL_box/2<bull->orig.y-wtable[bull->_weap_].radius)
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
						(player->move.orig.y - c_p_MDL_box/2 <= bull->orig.y + wtable[bull->_weap_].radius) &&
						(bull->orig.y - wtable[bull->_weap_].radius <= player->move.orig.y + c_p_MDL_box/2) &&
						(abs(player->move.orig.x - bull->orig.x) < 128)
				)
				{
					if (
							(bull->dir == DIR_LEFT || wtable[bull->_weap_].bullspeed < 0)&&
							(abs(player->move.orig.x-bull->orig.x)<Rdist)&&
							(player->move.orig.x+c_p_MDL_box/2<bull->orig.x-wtable[bull->_weap_].radius)
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
								(bull->dir == DIR_RIGHT || wtable[bull->_weap_].bullspeed < 0) &&
								(abs(player->move.orig.x-bull->orig.x)<Ldist) &&
								(bull->orig.x + wtable[bull->_weap_].radius < player->move.orig.x - c_p_MDL_box / 2)
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
		}
		bull = bull->next;
	}
	player->brain.danger = danger;
	if(!player->brain.danger)
	{
		player->brain.Fdanger = false;
		//player->move.go = $00;
	}
}
/*
 * атака
 */
static void ctrl_AI_attack(player_t * player, player_t * target)
{
	float dist;
	char wall;
	if( player->bull && player->brain.target )
	{
		if(
				abs(player->bull->orig.y - player->brain.target->move.orig.y) <
				abs(player->bull->orig.x - player->brain.target->move.orig.x)
		)
		{
			if(player->bull->orig.x < player->brain.target->move.orig.x)
				player->move.dir = DIR_RIGHT;
			else
				player->move.dir = DIR_LEFT;
		}
		else
		{
			if(player->bull->orig.y < player->brain.target->move.orig.y)
				player->move.dir = DIR_UP;
			else
				player->move.dir = DIR_DOWN;
		}
		return;
	}
	if
	(
			abs(player->move.orig.x - target->move.orig.x) > 160 ||
			abs(player->move.orig.y - target->move.orig.y) > 100
	)
	{
		player->w.attack = 0;
		return;
	};
	//если оружие не перезарядилось
	if(0<player->w.reloadtime_d) return;
	player->brain.target = NULL;
	if(
			(player->move.orig.x-c_p_MDL_box/2<target->move.orig.x) &&
			(target->move.orig.x<player->move.orig.x+c_p_MDL_box/2)
	)
	{
		if(target->move.orig.y < player->move.orig.y)
			player->move.dir = DIR_DOWN;
		else
			player->move.dir = DIR_UP;
		map_clip_find_near_wall(&player->move.orig, player->move.dir, &dist, &wall);
		if(
				//противник в прямой видимости
				(abs(player->move.orig.y - target->move.orig.y) < dist - c_p_MDL_box/2) ||
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
					(dist-c_p_MDL_box/2<wtable[player->brain.weapon-1].radius)&&
					((wall==c_m_w_w0+c_m_f_clip)||(wall==c_m_w_w1+c_m_f_clip))
			) player->brain.weapon = 0;
		}
	}
	else
	{
		if(
				(player->move.orig.y-c_p_MDL_box/2<target->move.orig.y)&&
				(target->move.orig.y<player->move.orig.y+c_p_MDL_box/2)
		)
		{
			if(target->move.orig.x<player->move.orig.x) player->move.dir = DIR_LEFT;
			else                                        player->move.dir = DIR_RIGHT;
			map_clip_find_near_wall(&player->move.orig,player->move.dir,&dist,&wall);
			if(
					//противник в прямой видимости
					(abs(player->move.orig.x-target->move.orig.x)<dist-c_p_MDL_box/2)||
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
								if(player->brain.weapon==1+2) {                          //мина
									if(player->move.dir==DIR_LEFT)
										player->move.dir = DIR_RIGHT;
									else
										player->move.dir = DIR_LEFT;
								};
							}
							else {
								if(player->w.ammo[1]>0)
									player->brain.weapon = 1+1;
								else
									player->brain.weapon = 0;
							}
						}
					}
				}
				if(
						(dist-c_p_MDL_box/2 < wtable[player->brain.weapon-1].radius) &&
						((wall==c_m_w_w0+c_m_f_clip) || (wall==c_m_w_w1+c_m_f_clip))
				)
					player->brain.weapon = 0;
			}
		}
		else
		{
			if(player->w.ammo[1]>0) {
				map_clip_find_near_wall(&player->move.orig,player->move.dir, &dist, &wall);
				if(
						(dist-c_p_MDL_box/2<wtable[1].radius) &&
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
			(160<abs(player->move.orig.x-target->move.orig.x))||
			(100<abs(player->move.orig.y-target->move.orig.y))
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
			if(abs(player->move.orig.x-target->move.orig.x)>abs(player->move.orig.y-target->move.orig.y))
			{
				if(player->move.orig.x<target->move.orig.x)
					player->move.dir = DIR_RIGHT;
				else
					player->move.dir = DIR_LEFT;
			}
			else
			{
				if(player->move.orig.y < target->move.orig.y)
					player->move.dir = DIR_UP;
				else
					player->move.dir = DIR_DOWN;
			}
			if(
					(abs(player->move.orig.x-target->move.orig.x)<c_BOT_dist)&&
					(abs(player->move.orig.y-target->move.orig.y)<c_BOT_dist)
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
