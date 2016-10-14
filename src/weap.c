/*
 *        Ubivat Tank
 *        управление оружием, пулями, взрывами
 * by Master San
 */

#include <defs.h>
#include <game.h>
#include <weap.h>
#include <img.h>
#include <map.h>
#include <plr.h>
#include <_gr2D.h>
#include <x10_time.h>

#include <string.h>
#include <stdlib.h>
#include <math.h>

//оружия
weapon_info_t wtable[3];

//список пуль
bull_t * bullList;
explode_t * explList;

extern player_t * playerList;


void explode_add(bull_t * bull, float Xexpl, float Yexpl);

/*
 * проверка на попадание в игрока
 */
static int checkdamage(struct player_s * player, bull_t * bull)
{
	if(
			(bull->player != player  )&& //попали не в себя
			(0<player->charact.health)&&
			(player->move.orig.x-c_p_MDL_box/2<=bull->orig.x+wtable[bull->_weap_].bullbox/2)&&
			(bull->orig.x-wtable[bull->_weap_].bullbox/2<=player->move.orig.x+c_p_MDL_box/2)&&
			(player->move.orig.y-c_p_MDL_box/2<=bull->orig.y+wtable[bull->_weap_].bullbox/2)&&
			(bull->orig.y-wtable[bull->_weap_].bullbox/2<=player->move.orig.y+c_p_MDL_box/2)
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

				Z_free(*bull);                                                    //удаляем его
				*bull = p;                                                          //bull указ. на пред. эл-т
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
void bull_draw(camera_t * cam, bull_t * bull, bool play)
{
	int mdlbox;                                                          //размер кадра X,Y
	int Fmax;

	mdlbox = bull->image->IMG.sx;
	Fmax   = ((bull->image->IMG.sy)/ mdlbox)>> 2;                      //количество кадров
	if(play)
	{
		bull->frame = bull->frame+c_bull_FPS*bull->time.delta/100;
		if((bull->frame<0)||(Fmax<=bull->frame)) bull->frame = 0;
	};
	if (
			(cam->orig.x-cam->sx/2<=bull->orig.x+(mdlbox >> 1)) && (bull->orig.x-(mdlbox >> 1)<=cam->orig.x+cam->sx/2) &&
			(cam->orig.y-cam->sy/2<=bull->orig.y+(mdlbox >> 1)) && (bull->orig.y-(mdlbox >> 1)<=cam->orig.y+cam->sy/2)
	)
	{
		gr2D.WIN.x0 = cam->x;gr2D.WIN.x1 = cam->x+cam->sx-1;
		gr2D.WIN.y0 = cam->y;gr2D.WIN.y1 = cam->y+cam->sy-1;
		gr2D_setimage1(
				roundf(cam->x+bull->orig.x-(cam->orig.x-cam->sx/2))-(mdlbox >> 1),
				roundf(cam->y-bull->orig.y+(cam->orig.y+cam->sy/2))-(mdlbox >> 1),
				bull->image->IMG.sx,
				bull->image->IMG.sy,
				0,
				mdlbox*(bull->dir*Fmax+trunc(bull->frame)),
				mdlbox,
				mdlbox,
				bull->image->IMG.pic
		);
		gr2D.WIN.x0 = 0;gr2D.WIN.x1 = gr2D_SCR_sx-1;
		gr2D.WIN.y0 = 0;gr2D.WIN.y1 = gr2D_SCR_sy-1;
	};
};
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
		time_Sget();
		bull->time.t1 = time.s*100+time.hs;                        //системное время в сотых долях секунд
		if(bull->time.t0>bull->time.t1) {
			bull->time.delta = bull->time.last_delta;                           //t0 должно быть меньше t1
		}
		else {
			bull->time.delta = bull->time.t1-bull->time.t0;
		};
		bull->time.last_delta = bull->time.delta;
		time_Sget();
		bull->time.t0 = time.s*100+time.hs;                                  //системное время в сотых долях секунд
		s = wtable[bull->_weap_].bullspeed*bull->time.delta/1000;
		switch(bull->dir)
		{
		case 0: bull->orig.y = bull->orig.y+s;break; //c_p_DIR_up
		case 1: bull->orig.y = bull->orig.y-s;break; //c_p_DIR_dn
		case 2: bull->orig.x = bull->orig.x-s;break; //c_p_DIR_lf
		case 3: bull->orig.x = bull->orig.x+s;break; //c_p_DIR_rt
		};
		bull->delta_s = bull->delta_s+abs(s);                                //подсчитываем пройденный путь
		if(
				(-1<wtable[bull->_weap_].range) && (wtable[bull->_weap_].range<bull->delta_s)
		)
		{//предельное расстояние пройдено
			explode_add(bull, bull->orig.x,bull->orig.y);
			bull_remove(&bull);
		}
		else
		{                                                          //предел на расстояние не превышен
			map_clip_find(                                                     //найдем препятствия
					&bull->orig, wtable[bull->_weap_].bullbox,
					c_m_w_w0 | c_m_w_w1 | c_m_w_brick,
					&Ul,&Ur,&Dl,&Dr,&Lu,&Ld,&Ru,&Rd
			);
			if((Ul || Ur || Dl || Dr || Lu || Ld || Ru || Rd) != 0)
			{     //пуля попала в стену
				explode_add(bull,bull->orig.x,bull->orig.y);
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
					explode_add(bull,bull->orig.x,bull->orig.y);
					bull_remove(&bull);
				}
				else
				{                                                        //в игрока не попали, продолжение полета
					bull_draw(&game.P0cam, bull, true);
					if(game.P1) bull_draw(&game.P1cam, bull, false);
					bull = bull->next;
				};
			};
		};
	};
};
/*
 * добавление взрыва
 */
void explode_add(bull_t * bull, float Xexpl, float Yexpl)
{
	explode_t * p;

	p = Z_malloc(sizeof(*p));
	p->orig.x = Xexpl;                                                    //координаты
	p->orig.y = Yexpl;                                                    //координаты
	if(!bull)
	{
		p->player = NULL;                                                     //игрок, выпустивший пулю
		p->_weap_ = 1;                                                       //
	}
	else
	{
		p->player = bull->player;                                            //игрок, выпустивший пулю
		p->_weap_ = bull->_weap_;                                            //
	};
	p->frame  = -1;                                                       //№ кадра
	switch(bull->_weap_){                                                 //изображение взрыва
	case 0  :p->image = IMG_connect(game.HEADimg,"E_SMALL"    );break;
	case 1:
	case 2:p->image = IMG_connect(game.HEADimg,"E_BIG"      );break;
	};
	p->next       = explList;
	explList = p;
	explList->time.delta      = 0;
	explList->time.last_delta = 0;
	time_Sget();
	explList->time.t0         = time.s*100+time.hs;
}
/*
 * удаление взрыва из списка, после удаления
 *        explode указывает на предыдущий эл-т если
 *        он существует, если нет - то на следующий
 */
void explode_remove(explode_t ** explode)
{
	explode_t * p;

	if (explList && *explode != NULL)
	{
		p = explList;
		if(explList== *explode) {                                    //если HEAD и explode совпадают
			explList = p->next;
			*explode          = explList;
			Z_free(p);
			p = NULL;
		}
		else
		{
			if(p->next)
			{                                         //если в списке не один эл-т
				while(p && p->next != *explode) p = p->next;                     //находим ссылку на нужный эл-т
				p->next = (*explode)->next;
				Z_free(*explode);                                                    //удаляем его
				*explode = p;                                                          //bull указ. на пред. эл-т
			}
		}
	}
}
/*
 * удаление всех взрывов
 */
void explode_removeall()
{
	explode_t * explode;
	while(explList)
	{
		explode          = explList;
		explList = explList->next;
		Z_free(explode);
	};
};
/*
 * отрисовка
 */
void explode_draw(camera_t * cam, explode_t * explode, bool play)
{
	int mdlbox;

	mdlbox = explode->image->IMG.sx;
	if(play) explode->frame = explode->frame+c_explode_FPS*explode->time.delta/100;
	if(
			(cam->orig.x-cam->sx/2<=explode->orig.x+(mdlbox >> 1)) &&
			(explode->orig.x-(mdlbox >> 1)<=cam->orig.x+cam->sx/2) &&
			(cam->orig.y-cam->sy/2<=explode->orig.y+(mdlbox >> 1)) &&
			(explode->orig.y-(mdlbox >> 1)<=cam->orig.y+cam->sy/2)
			)
	{
		gr2D.WIN.x0 = cam->x;gr2D.WIN.x1 = cam->x+cam->sx-1;
		gr2D.WIN.y0 = cam->y;gr2D.WIN.y1 = cam->y+cam->sy-1;
		gr2D_setimage1(
				roundf(cam->x+explode->orig.x-(cam->orig.x-cam->sx/2))-(mdlbox >> 1),
				roundf(cam->y-explode->orig.y+(cam->orig.y+cam->sy/2))-(mdlbox >> 1),
				explode->image->IMG.sx,
				explode->image->IMG.sy,
				0,
				mdlbox*trunc(explode->frame),
				mdlbox,
				mdlbox,
				explode->image->IMG.pic
		);
		gr2D.WIN.x0 = 0;gr2D.WIN.x1 = gr2D_SCR_sx-1;
		gr2D.WIN.y0 = 0;gr2D.WIN.y1 = gr2D_SCR_sy-1;
	}
}
/*
 * управление взрывом
 */
void explode_control()
{
	explode_t * explode;
	player_t * player;
	float r,sp_x,sp_y;
	bool self;
	int x,y;
	char wall;
	char wall_type;

	explode = explList;
	while(explode)
	{
		time_Sget();
		explode->time.t1 = time.s*100+time.hs;
		if(explode->time.t0>explode->time.t1)
			explode->time.delta = explode->time.last_delta;
		else
			explode->time.delta = explode->time.t1-explode->time.t0;
		explode->time.last_delta = explode->time.delta;
		time_Sget();
		explode->time.t0 = time.s*100+time.hs;
		if(explode->frame == -1)
		{
			explode->frame = 0;
//проверка попаданий в стены
			for(y = -wtable[explode->_weap_].radius; y<= wtable[explode->_weap_].radius; y++)
			{
				for(x = -wtable[explode->_weap_].radius; x<= wtable[explode->_weap_].radius; x++)
				{
					if(
							(0<=trunc((explode->orig.x+x)/8)) && (trunc((explode->orig.x+x)/8)<=c_MAP_sx) &&
							(0<=trunc((explode->orig.y+y)/8)) && (trunc((explode->orig.y+y)/8)<=c_MAP_sy)
					){
						wall = map.map[(int)trunc((explode->orig.y+y)/8)][(int)trunc((explode->orig.x+x)/8)];
						if(wall & c_m_f_clip)
						{                   //присутствует зажим
							wall_type = wall & 0x0F;
							if((wall_type | c_m_w_w0 | c_m_water) != (c_m_w_w0 | c_m_water))
							{//если не броня0 и не вода
								if(
										(wall_type == c_m_w_brick) ||//если кирпич
										(
												(wall_type == c_m_w_w1) &&
												((explode->_weap_==1)||(explode->_weap_==2)))//если броня1
								)
								{
									map.map[(int)trunc((explode->orig.y+y)/8)][(int)trunc((explode->orig.x+x)/8)] = 0;
								}
							}
						}
					}
				}
			}
//проверка попаданий в игрока
			player = playerList;
			while(player)
			{
				sp_x = player->move.orig.x-explode->orig.x;
				sp_y = player->move.orig.y-explode->orig.y;
				if(
						(abs(sp_x)<=c_p_MDL_box/2) &&
						(abs(sp_y)<=c_p_MDL_box/2)) r = 0;
				else
				{
					r = sqrt(sp_x*sp_x+sp_y*sp_y)-sqrt(sqrf(c_p_MDL_box/2)+sqrf(c_p_MDL_box/2))/2;
					if(r<=wtable[explode->_weap_].bullbox) r = 0;
				};
				if((0<wtable[explode->_weap_].radius) && (r<=wtable[explode->_weap_].radius))
				{
					if(
							(explode->player == player) ||
							(
									(
											(explode->player==game.P0) || (explode->player==game.P1)
									)&&
									(
											(player == game.P0)||
											(player == game.P1)
									)
							)
					)
						self = true;          //взрывом задели себя или товарища по команде(не для монстров)
					else self = false;                                                  //взрывом задели другого игрока
					player_getdamage(player, explode, self, r);
				};
				player = player->next;
			};
		};
		if(c_explode_Famnt-1<explode->frame) explode->frame = c_explode_Famnt-1;
		explode_draw(&game.P0cam, explode, true);                                  //рисуем взрыв
		if(game.P1 != NULL) explode_draw(&game.P1cam, explode, false);             //рисуем взрыв
		if(explode->frame>c_explode_Famnt-1) explode_remove(&explode);
		else                                 explode = explode->next;
	};
};
