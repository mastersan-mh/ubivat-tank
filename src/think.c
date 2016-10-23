/*
 *        Ubivat Tank
 *        управление игроками
 * by Master San
 */
#include <game.h>
#include <weap.h>
#include <map.h>
#include <player.h>
#include <x10_time.h>
#include <x10_kbrd.h>
#include <types.h>

#include <stdlib.h>

/*
 * управление игроком
 */
void think_human(int Pnum, player_t * player)
{
	/*

	switch(kbrd.port)
	{
	case KP0_ESCAPE_1:
		game.ingame = false;
		break;
	}
	player_checkcode();

	//клав. буфер пуст
	if(kbrd.port == 0)return;

	control_t * control;

	if(Pnum == 0)
		control = &(game.controlP0);
	else
		control = &(game.controlP1);
	int i;
	for(i = 0; i < 14; i++)
	{
		if((*control)[i] == kbrd.port)break;
	}
#define control_forward      0
#define control_forward_stop 1
#define control_backward     2
#define control_backward_stop 3
#define control_left         4
#define control_left_stop    5
#define control_right        6
#define control_right_stop   7
#define control_attack1      8
#define control_attack1stop  9
#define control_attack2      10
#define control_attack2stop  11
#define control_attack3      12
#define control_attack3stop  13
	switch(i)
	{
	case control_forward_stop:
	case control_backward_stop:
	case control_left_stop:
	case control_right_stop:
		player->move.go = false;
		break;
	case control_forward:
		player->move.go = true;
		player->move.dir = c_DIR_up;
		break;
	case control_backward:
		player->move.go = true;
		player->move.dir = c_DIR_dn;
		break;
	case control_left:
		player->move.go = true;
		player->move.dir = c_DIR_lf;
		break;
	case control_right:
		player->move.go = true;
		player->move.dir = c_DIR_rt;
		break;
	case control_attack1stop:
	case control_attack2stop:
	case control_attack3stop:
		player->w.attack = 0;
		break;
	case control_attack1:
		player->w.attack = 1;
		break;
	case control_attack2:
		player->w.attack = 2;
		break;
	case control_attack3:
		player->w.attack = 3;
		break;
	}
	*/
};

/*
 * управление вражеским игроком
 */
void think_enemy(struct player_s * player)
{
	if(0<player->charact.health)
	{
		ctrl_AI_checkdanger(player);
		if(!player->brain.danger)
		{
			if(!game.P1)
			{
				if(game.P0->charact.health<=0) player->w.attack = 0;
				else {
					if(!player->bull &&  !player->w.attack) ctrl_AI_findenemy(player, game.P0);
					ctrl_AI_attack(player,game.P0);
				};
			}
			else
			{
				if(xrand(2)==0)
				{
					if(0<game.P0->charact.health)
					{
						if( !player->bull && !player->w.attack) ctrl_AI_findenemy(player,game.P0);
						ctrl_AI_attack(player,game.P0);
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
 * инициализация AI
 */
void ctrl_AI_init(TAIbrain * brain)
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
void ctrl_AI_done(TAIbrain * brain)
{
	brain->target = NULL;
};
/*
 * уворачивание от снарядов
 */
void ctrl_AI_checkdanger(player_t * player)
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
	bool flag = false;
	bull = bullList;
	while(bull && !flag)
	{
		//не своя пуля
		if(bull->player != player)
		{
			//верхняя ближайшая стена
			map_clip_find_near(&player->move.orig, 0, c_DIR_up, 0xF0, 100, &Udist);
			//нижняя ближайшая стена
			map_clip_find_near(&player->move.orig, 0, c_DIR_dn, 0xF0, 100, &Ddist);
			//правая ближайшая стена
			map_clip_find_near(&player->move.orig, 0, c_DIR_rt, 0xF0, 160, &Rdist);
			//левая ближайшая стена
			map_clip_find_near(&player->move.orig, 0, c_DIR_lf, 0xF0, 160, &Ldist);
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
						((bull->dir=c_DIR_up) || (wtable[bull->_weap_].bullspeed<0)) &&
						(abs(player->move.orig.y-bull->orig.y)<Ddist) &&
						(bull->orig.y+wtable[bull->_weap_].radius<player->move.orig.y-c_p_MDL_box/2)
				)
			{
					flag = true;
					if(!player->brain.Fdanger)
					{
						player->move.go = true;
						player->brain.Fdanger = true;
						if(0 <= Ld && 0 <= Rd) player->move.dir = xrand(2) + 2;
						else
							if(0 < Ld) player->move.dir = c_DIR_lf;
							else
								if(0<Rd) player->move.dir = c_DIR_rt;
					}
			}
				else
				{
					if(
							(bull->dir=c_DIR_dn || wtable[bull->_weap_].bullspeed<0) &&
							(abs(player->move.orig.y-bull->orig.y)<Udist) &&
							(player->move.orig.y+c_p_MDL_box/2<bull->orig.y-wtable[bull->_weap_].radius)
					)
					{
						flag = true;
						if(!player->brain.Fdanger) {
							player->move.go = true;
							player->brain.Fdanger = true;
							if( 0 <= Ld && 0 <= Rd) player->move.dir = xrand(2)+2;
							else
								if(0 < Ld) player->move.dir = c_DIR_lf;
								else
									if(0<Rd) player->move.dir = c_DIR_rt;
						}
					}
				}
			}
			else {
				if(
						(player->move.orig.y-c_p_MDL_box/2<=bull->orig.y+wtable[bull->_weap_].radius) &&
						(bull->orig.y-wtable[bull->_weap_].radius<=player->move.orig.y+c_p_MDL_box/2) &&
						(abs(player->move.orig.x-bull->orig.x)<128)
				)
				{
					if (
							((bull->dir=c_DIR_lf) || (wtable[bull->_weap_].bullspeed<0))&&
							(abs(player->move.orig.x-bull->orig.x)<Rdist)&&
							(player->move.orig.x+c_p_MDL_box/2<bull->orig.x-wtable[bull->_weap_].radius)
					)
					{
						flag = 1;
						if(!player->brain.Fdanger)
						{
							player->move.go = true;
							player->brain.Fdanger = true;
							if(0 <= Ud && 0 <= Dd) player->move.dir = xrand(2);
							else
								if(0<Ud) player->move.dir = c_DIR_up;
								else
									if(0<Dd) player->move.dir = c_DIR_dn;
						}
					}
					else
					{
						if(
								((bull->dir=c_DIR_rt) || (wtable[bull->_weap_].bullspeed<0)) &&
								(abs(player->move.orig.x-bull->orig.x)<Ldist) &&
								(bull->orig.x+wtable[bull->_weap_].radius<player->move.orig.x-c_p_MDL_box/2)
						)
						{
							flag = true;
							if(!player->brain.Fdanger)
							{
								player->move.go = true;
								player->brain.Fdanger = true;
								if(0 <= Ud && 0 <= Dd) player->move.dir = xrand(2);
								else
									if(0<Ud) player->move.dir = c_DIR_up;
									else
										if(0<Dd) player->move.dir = c_DIR_dn;
							}
						}
					}
				}
			}
		}
		bull = bull->next;
	}
	player->brain.danger = flag;
	if(!player->brain.danger)
	{
		player->brain.Fdanger = false;
		//player->move.go = $00;
	}
}
/********атака********/
void ctrl_AI_attack(player_t * player, player_t * target)
{
	float dist;
	char wall;
	if( player->bull && player->brain.target )
	{
		if(
				abs(player->bull->orig.y-player->brain.target->move.orig.y)<
				abs(player->bull->orig.x-player->brain.target->move.orig.x)
		)
		{
			if(player->bull->orig.x < player->brain.target->move.orig.x)
				player->move.dir = c_DIR_rt;
			else
				player->move.dir = c_DIR_lf;
		}
		else
		{
			if(player->bull->orig.y < player->brain.target->move.orig.y)
				player->move.dir = c_DIR_up;
			else
				player->move.dir = c_DIR_dn;
		}
		return;
	}
	if
	(
			(abs(player->move.orig.x-target->move.orig.x)>160) ||
			(abs(player->move.orig.y-target->move.orig.y)>100)
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
		if(target->move.orig.y<player->move.orig.y)
			player->move.dir = c_DIR_dn;
		else
			player->move.dir = c_DIR_up;
		map_clip_find_near_wall(&player->move.orig, player->move.dir, &dist, &wall);
		if(
				//противник в прямой видимости
				(abs(player->move.orig.y-target->move.orig.y)<dist-c_p_MDL_box/2) ||
				((wall       ) == c_m_w_w0  ) || ((wall)==c_m_w_w1  ) ||
				((wall & 0x0F) == c_m_water ) || ((wall)==c_m_f_clip)
		){
			if(player->w.ammo[2]>0)
			{
				//выбираем наугад ракету или мину
				player->brain.weapon = 1+1+xrand(2);
				if(player->brain.weapon == 1+2)
				{
					//мина
					if(player->move.dir == c_DIR_dn)
						player->move.dir = c_DIR_up;
					else
						player->move.dir = c_DIR_dn;
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
							if(player->brain.weapon==1+2) {                           //мина
								if(player->move.dir==c_DIR_dn)
									player->move.dir = c_DIR_up;
								else
									player->move.dir = c_DIR_dn;
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
			if(target->move.orig.x<player->move.orig.x) player->move.dir = c_DIR_lf;
			else                                        player->move.dir = c_DIR_rt;
			map_clip_find_near_wall(&player->move.orig,player->move.dir,&dist,&wall);
			if(
					//противник в прямой видимости
					(abs(player->move.orig.x-target->move.orig.x)<dist-c_p_MDL_box/2)||
					((wall       ) == c_m_w_w0  ) || ((wall)==c_m_w_w1  )||
					((wall & 0x0F) == c_m_water ) || ((wall)==c_m_f_clip)
			)
			{
				if(player->w.ammo[2]>0)
				{
					player->brain.weapon = 1+1+xrand(2);                               //выбираем наугад ракету или мину
					if(player->brain.weapon==1+2) {                             //мина
						if(player->move.dir==c_DIR_lf) player->move.dir = c_DIR_rt;
						else                            player->move.dir = c_DIR_lf;
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
									if(player->move.dir==c_DIR_lf)
										player->move.dir = c_DIR_rt;
									else
										player->move.dir = c_DIR_lf;
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
void ctrl_AI_findenemy(player_t * player, player_t * target)
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
					player->move.dir = c_DIR_rt;
				else
					player->move.dir = c_DIR_lf;
			}
			else
			{
				if(player->move.orig.y < target->move.orig.y)
					player->move.dir = c_DIR_up;
				else
					player->move.dir = c_DIR_dn;
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
	player->brain.count += player->time.delta;
	if(c_BOT_time*2<player->brain.count) player->brain.count = 0;
}
