/*
 *        Ubivat Tank
 *        управление оружием, пулями, взрывами
 * by Master San
 */

#include <video.h>
#include <game.h>
#include <weap.h>
#include <img.h>
#include <map.h>
#include <_gr2D.h>
#include "sound.h"

#include "mobjs.h"
#include "explode.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <player.h>
#include <types.h>

//оружия
weapon_info_t wtable[3];

//список пуль
bull_t * bullList = NULL;

extern player_t * playerList;

/*
 * проверка на попадание в игрока
 */
static int checkdamage(struct player_s * player, bull_t * bull)
{
	if(
			(bull->player != player  )&& //попали не в себя
			(0<player->charact.health)&&
			(player->move.pos.x-c_p_MDL_box/2<=bull->pos.x+wtable[bull->_weap_].bullbox/2)&&
			(bull->pos.x-wtable[bull->_weap_].bullbox/2<=player->move.pos.x+c_p_MDL_box/2)&&
			(player->move.pos.y-c_p_MDL_box/2<=bull->pos.y+wtable[bull->_weap_].bullbox/2)&&
			(bull->pos.y-wtable[bull->_weap_].bullbox/2<=player->move.pos.y+c_p_MDL_box/2)
	) return true;
	return false;
};
/*
 * создание пули
 */
void bull_add()
{
	bull_t * bull;
	bull = Z_malloc(sizeof(*bull));
	bull->next      = bullList;
	bullList   = bull         ;
};
/*
 * даление пули из списка, после удаления
 *        bull указывает на предыдущий эл-т если
 *        он существует, если нет - то на следующий
 */
void bull_remove(bull_t ** bull)
{
	bull_t *p;

	if (bullList && *bull)
	{
		p = bullList;

		if(p->player->bull == *bull) p->player->bull = NULL;

		if(bullList == *bull)
		{                                    //если HEAD и bull совпадают
			bullList = p->next;
			*bull = bullList;
			Z_free(p);
			p = NULL;
		}
		else {
			if(p->next != NULL) {                                         //если в списке не один эл-т
				while((p->next != *bull) && (p != NULL)) p = p->next;                     //находим ссылку на нужный эл-т
				p->next = (*bull)->next;

				if((*bull)->player->bull == *bull) (*bull)->player->bull = NULL;

				//удаляем его
				Z_free(*bull);
				//bull указ. на пред. эл-т
				*bull = p;
			}
		}
	}
}
/*
 * очистка всего списка пуль
 */
void bull_removeall()
{
	bull_t * bull;

	while(bullList)
	{
		bull          = bullList;
		bull->player->bull = NULL;
		bullList = bullList->next;
		Z_free(bull);
	};
};
/*
 * рисование пули
 */
static void bull_draw(camera_t * cam, bull_t * bull)
{
	//размер кадра X,Y
	int mdlbox = bull->image->img_sx;
	//количество кадров
	int Fmax = bull->image->img_sy / (mdlbox * 4);
	if
	(
			(cam->pos.x-cam->sx/2<=bull->pos.x+(mdlbox >> 1)) && (bull->pos.x-(mdlbox >> 1)<=cam->pos.x+cam->sx/2) &&
			(cam->pos.y-cam->sy/2<=bull->pos.y+(mdlbox >> 1)) && (bull->pos.y-(mdlbox >> 1)<=cam->pos.y+cam->sy/2)
	)
	{
		gr2D_setimage1(
				roundf(cam->x+bull->pos.x-(cam->pos.x-cam->sx/2))-(mdlbox >> 1),
				roundf(cam->y-bull->pos.y+(cam->pos.y+cam->sy/2))-(mdlbox >> 1),
				bull->image,
				0,
				mdlbox*(bull->dir*Fmax+trunc(bull->frame)),
				mdlbox,
				mdlbox
		);
	};
};



void bull_draw_all(camera_t * cam)
{
	bull_t * bull = bullList;
	while(bull)
	{
		bull_draw(cam, bull);
		bull = bull->next;
	}

}

/*
 * поведение пули
 */
void bull_control()
{
	bool Ul,Ur,Dl,Dr,Lu,Ld,Ru,Rd;
	float s;
	bool flag;
	player_t * player;
	bull_t * bull;

	bull = bullList;
	while(bull)
	{
		s = wtable[bull->_weap_].bullspeed * dtimed1000;
		switch(bull->dir)
		{
		case 0: bull->pos.y = bull->pos.y+s;break; //c_p_DIR_up
		case 1: bull->pos.y = bull->pos.y-s;break; //c_p_DIR_dn
		case 2: bull->pos.x = bull->pos.x-s;break; //c_p_DIR_lf
		case 3: bull->pos.x = bull->pos.x+s;break; //c_p_DIR_rt
		};
		bull->delta_s = bull->delta_s+abs(s);                                //подсчитываем пройденный путь
		if(
				(-1<wtable[bull->_weap_].range) && (wtable[bull->_weap_].range<bull->delta_s)
		)
		{//предельное расстояние пройдено
			explode_new(bull->pos.x, bull->pos.y, bull_type_to_explode_type(bull->_weap_), bull->player);
			bull_remove(&bull);
		}
		else
		{                                                          //предел на расстояние не превышен
			map_clip_find(                                                     //найдем препятствия
					&bull->pos, wtable[bull->_weap_].bullbox,
					c_m_w_w0 | c_m_w_w1 | c_m_w_brick,
					&Ul,&Ur,&Dl,&Dr,&Lu,&Ld,&Ru,&Rd
			);
			if((Ul || Ur || Dl || Dr || Lu || Ld || Ru || Rd) != 0)
			{     //пуля попала в стену
				explode_new(bull->pos.x,bull->pos.y, bull_type_to_explode_type(bull->_weap_), bull->player);
				bull_remove(&bull);
			}
			else {                                                         //пуля не попала в стену
				player = playerList;
				flag = 0;
				while(player && !flag)
				{                               //проверим на попадание в игрока
					flag = checkdamage(player,bull);
					player = player->next;
				};
				if(flag)
				{                                              //попадание в игрока
					explode_new(bull->pos.x,bull->pos.y, bull_type_to_explode_type(bull->_weap_), bull->player);
					bull_remove(&bull);
				}
				else
				{
					//в игрока не попали, продолжение полета


					int mdlbox = bull->image->img_sx;
					//количество кадров
					int Fmax = bull->image->img_sy / (mdlbox * 4);
					bull->frame = bull->frame + c_bull_FPS * dtimed1000;
					if(bull->frame < 0 || Fmax<=bull->frame) bull->frame = 0;
					bull = bull->next;
				}
			}
		}
	}
}





