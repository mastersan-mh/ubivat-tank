/*
 *        Ubivat Tank
 *        обработка игрока
 *
 * by Master San
 */

#include <video.h>
#include <img.h>
#include <map.h>
#include <weap.h>
#include <game.h>
#include <_gr2D.h>
#include <fonts.h>
#include <player.h>

#include "mobjs.h"
#include "items.h"
#include "bull.h"
#include "explode.h"

#include "sound.h"

#include <string.h>
#include <think.h>
#include <types.h>

playerinfo_t playerinfo_table[__PLAYER_LEVEL_NUM] =
{
		/*SCORES, HEALTH, ARMOR,                   AMMO_ARTILLERY,   AMMO_MISSILE,     AMMO_MINE */
		{ { ITEM_SCOREPERCLASS * 1,  100,     0, ITEM_AMOUNT_INF, ITEM_AMOUNT_NA, ITEM_AMOUNT_NA }, 40/2 * SPEEDSCALE, IMG_TANK0 },
		{ { ITEM_SCOREPERCLASS * 2,  100,    50, ITEM_AMOUNT_INF, ITEM_AMOUNT_NA, ITEM_AMOUNT_NA }, 50/2 * SPEEDSCALE, IMG_TANK1 },
		{ { ITEM_SCOREPERCLASS * 3,  100,   100, ITEM_AMOUNT_INF, 50            , ITEM_AMOUNT_NA }, 60/2 * SPEEDSCALE, IMG_TANK2 },
		{ { ITEM_SCOREPERCLASS * 4,  200,   150, ITEM_AMOUNT_INF, 50            , ITEM_AMOUNT_NA }, 70/2 * SPEEDSCALE, IMG_TANK3 },
		{ { ITEM_SCOREPERCLASS * 5,  200,   200, ITEM_AMOUNT_INF, 50            , 50              }, 90/2 * SPEEDSCALE, IMG_TANK4 },
		{ { ITEM_SCOREPERCLASS * 6, 5000,  5000, ITEM_AMOUNT_INF, 50            , 50              }, 90/2 * SPEEDSCALE, IMG_TANK4 }  /* BOSS */
};

void * player_mobj_init(const mobj_t * mobj);

const mobj_register_t playermobjinfo = {
		.name = "player",
		.mobjinit = player_mobj_init
};

void mobj_player_init()
{
	mobjinfo_register(&playermobjinfo);
}

void * player_mobj_init(const mobj_t * mobj)
{
	mobj_player_t * player = Z_malloc(sizeof(mobj_player_t));
	return player;
}


void player_moveUp_ON()
{
	if(!game.P0)return;
	game.P0->dir = DIR_UP;
	game.P0->player->move.go = true;
}

void player_moveUp_OFF()
{
	if(!game.P0)return;
	if(game.P0->dir != DIR_UP) return;
	game.P0->player->move.go = false;
}

void player_moveDown_ON()
{
	if(!game.P0)return;
	game.P0->dir = DIR_DOWN;
	game.P0->player->move.go = true;
}

void player_moveDown_OFF()
{
	if(!game.P0)return;
	if(game.P0->dir != DIR_DOWN) return;
	game.P0->player->move.go = false;
}

void player_moveLeft_ON()
{
	if(!game.P0)return;
	game.P0->dir = DIR_LEFT;
	game.P0->player->move.go = true;
}

void player_moveLeft_OFF()
{
	if(!game.P0)return;
	if(game.P0->dir != DIR_LEFT) return;
	game.P0->player->move.go = false;
}

void player_moveRight_ON()
{
	if(!game.P0)return;
	game.P0->dir = DIR_RIGHT;
	game.P0->player->move.go = true;
}

void player_moveRight_OFF()
{
	if(!game.P0)return;
	if(game.P0->dir != DIR_RIGHT) return;
	game.P0->player->move.go = false;
}

void player_attack_weapon1_ON()
{
	game.P0->player->weap = WEAP_ARTILLERY;
	game.P0->player->attack = true;
}
void player_attack_weapon1_OFF()
{
	game.P0->player->attack = false;
}

void player_attack_weapon2_ON()
{
	game.P0->player->weap = WEAP_MISSILE;
	game.P0->player->attack = true;
}
void player_attack_weapon2_OFF()
{
	game.P0->player->attack = false;
}

void player_attack_weapon3_ON()
{
	game.P0->player->weap = WEAP_MINE;
	game.P0->player->attack = true;
}
void player_attack_weapon3_OFF()
{
	game.P0->player->attack = false;
}

void player2_moveUp_ON()
{
	if(!game.P1)return;
	game.P1->dir = DIR_UP;
	game.P1->player->move.go = true;
}

void player2_moveUp_OFF()
{
	if(!game.P1)return;
	if(game.P1->dir != DIR_UP) return;
	game.P1->player->move.go = false;
}

void player2_moveDown_ON()
{
	if(!game.P1)return;
	game.P1->dir = DIR_DOWN;
	game.P1->player->move.go = true;
}

void player2_moveDown_OFF()
{
	if(!game.P1)return;
	if(game.P1->dir != DIR_DOWN) return;
	game.P1->player->move.go = false;
}

void player2_moveLeft_ON()
{
	if(!game.P1)return;
	game.P1->dir = DIR_LEFT;
	game.P1->player->move.go = true;
}

void player2_moveLeft_OFF()
{
	if(!game.P1)return;
	if(game.P1->dir != DIR_LEFT) return;
	game.P1->player->move.go = false;
}

void player2_moveRight_ON()
{
	if(!game.P1)return;
	game.P1->dir = DIR_RIGHT;
	game.P1->player->move.go = true;
}

void player2_moveRight_OFF()
{
	if(!game.P1)return;
	if(game.P1->dir != DIR_RIGHT) return;
	game.P1->player->move.go = false;
}

void player2_attack_weapon1_ON()
{
	if(!game.P1)return;
	game.P1->player->weap = WEAP_ARTILLERY;
	game.P1->player->attack = true;
}
void player2_attack_weapon1_OFF()
{
	if(!game.P1)return;
	game.P1->player->attack = false;
}

void player2_attack_weapon2_ON()
{
	if(!game.P1)return;
	game.P1->player->weap = WEAP_MISSILE;
	game.P1->player->attack = true;
}
void player2_attack_weapon2_OFF()
{
	if(!game.P1)return;
	game.P1->player->attack = false;
}

void player2_attack_weapon3_ON()
{
	if(!game.P1)return;
	game.P1->player->weap = WEAP_MINE;
	game.P1->player->attack = true;
}
void player2_attack_weapon3_OFF()
{
	if(!game.P1)return;
	game.P1->player->attack = false;
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

/*
 * подбирание предметов игроком
 */
void player_item_get(mobj_t * player)
{
	if(!
			(
					(player->player->items[ITEM_HEALTH] > 0) &&
					((player->player->charact.status==c_p_P0)||(player->player->charact.status==c_p_P1))
			)
	)return;
	// не монстр(монстрюки оружие не подбирают)
	mobj_t * mobj;
	for(mobj = map.mobjs; mobj; mobj = mobj->next)
	{
		if(mobj->type != MOBJ_ITEM) continue;

		mobj_item_t * item = &mobj->item;
		if(!item->exist)continue;

		if(
				(mobj->pos.x - c_i_MDL_box/2 < player->pos.x + c_p_MDL_box/2 )&&
				(player->pos.x - c_p_MDL_box/2 <= mobj->pos.x + c_i_MDL_box/2)&&
				(mobj->pos.y - c_i_MDL_box/2 <= player->pos.y + c_p_MDL_box/2)&&
				(player->pos.y - c_p_MDL_box/2 < mobj->pos.y + c_i_MDL_box/2 )&&
				(mobj->type == MOBJ_ITEM)
		)
		{
			playerinfo_t *playerinfo = &playerinfo_table[player->player->level];
			itemtype_t itemtype = mobj->item.type;

			if(
					playerinfo->items[itemtype] != ITEM_AMOUNT_INF &&
					playerinfo->items[itemtype] != ITEM_AMOUNT_NA &&
					((player->player->items[itemtype] < playerinfo->items[itemtype]) || item->amount < 0)
			)
			{
				item->exist = false;
				player->player->items[itemtype] += item->amount;
				check_value_int(&player->player->items[itemtype], 0, playerinfo->items[itemtype]);
			};


			if(itemtype == ITEM_SCORES)
			{
				player_class_init(player);
				if(5 <= player->player->items[ITEM_SCORES] / ITEM_SCOREPERCLASS)
				{
					if(player->player->items[ITEM_HEALTH] < playerinfo->items[ITEM_HEALTH])
						player->player->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
					if(player->player->items[ITEM_ARMOR] < player->player->items[ITEM_ARMOR])
						player->player->items[ITEM_ARMOR] = playerinfo->items[ITEM_ARMOR];
				}
			}
		}
	} //end for
}

/*
 * проверка объектов на карте
 */
void player_obj_check(mobj_t * player)
{
	if(
			!((player->player->charact.status==c_p_P0)||(player->player->charact.status==c_p_P1))
	) return;

	mobj_t * mobj;
	for(mobj = map.mobjs; mobj; mobj = mobj->next)
	{
		if(
				(mobj->pos.x-c_o_MDL_box/2 <= player->pos.x + c_p_MDL_box / 2)&&
				(player->pos.x-c_p_MDL_box/2 <= mobj->pos.x + c_o_MDL_box / 2)&&
				(mobj->pos.y-c_o_MDL_box/2 <= player->pos.y + c_p_MDL_box / 2)&&
				(player->pos.y-c_p_MDL_box/2 <= mobj->pos.y + c_o_MDL_box / 2)
		)
		{
			switch (mobj->type)
			{
			case MOBJ_MESSAGE:
				//отправим сообщение игроку
				game_message_send(mobj->mesage.message);
				break;
			case MOBJ_EXIT:
				game._win_ = true;
				//отправим сообщение игроку
				game_message_send(mobj->exit.message);
				break;
			default: ;
			}
		}
	}
}
/*
 * рисование игрока
 */
void player_draw(camera_t * cam, mobj_t * player)
{
	if(player->player->items[ITEM_HEALTH] > 0)
	{ //если игрок жив
		if(
				(cam->pos.x-cam->sx/2<=player->pos.x+(c_p_MDL_box/2))&&(player->pos.x-(c_p_MDL_box/2)<=cam->pos.x+cam->sx/2)&&
				(cam->pos.y-cam->sy/2<=player->pos.y+(c_p_MDL_box/2))&&(player->pos.y-(c_p_MDL_box/2)<=cam->pos.y+cam->sy/2)
		)
		{
			gr2D_setimage1(
					round(cam->x+player->pos.x-(cam->pos.x-cam->sx/2))+c_p_MDL_pos,
					round(cam->y-player->pos.y+(cam->pos.y+cam->sy/2))+c_p_MDL_pos,
					player->img,
					0,
					c_p_MDL_box*((player->dir * 4)+round(player->player->Fbase)),
					c_p_MDL_box,
					c_p_MDL_box
			);
			gr2D_setimage0(
					round(cam->x+player->pos.x-(cam->pos.x-cam->sx/2))+c_p_MDL_pos,
					round(cam->y-player->pos.y+(cam->pos.y+cam->sy/2))+c_p_MDL_pos,
					player->player->Iflag
			);
		};
	};
};

/*
 * передвижение игрока
 */
static void player_move(mobj_t * player, int dir, vec_t * speed)
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
};
/*
 * управление игроком
 */
void player_handle(struct mobj_s * player)
{
	vec2_t Sorig;
	vec_t L,R,U,D;
	vec_t speed_s;

	playerinfo_t * playerinfo = &playerinfo_table[player->player->level];
	if(player->player->items[ITEM_HEALTH] <= 0)
	{
		if(player->player->soundId_move)
		{
			sound_play_stop(player->player->soundId_move);
			player->player->soundId_move = 0;
		}

		//если игрок мертв
		if(player->player->charact.spawned)
		{
			explode_new(player->pos.x, player->pos.y, EXPLODE_MISSILE, player);
			player->player->charact.spawned = false;
			player->player->items[ITEM_ARMOR] = 0;
			player->player->items[ITEM_AMMO_MISSILE] = 0;
			player->player->items[ITEM_AMMO_MINE] = 0;
		};
		if(player->player->charact.status == c_p_BOSS) game._win_ = true;
	}
	else
	{
		//игрок жив
		if(player->player->bull)
		{
			player->player->bull->dir = player->dir;
			player->player->move.go = false;
		};
		if(player->player->move.go)
		{
			//игрок едет
			player->player->move.speed += PLAYER_ACCEL * dtime;
			if(player->player->move.speed > playerinfo->speed) player->player->move.speed = playerinfo->speed;

			player->player->Fbase = player->player->Fbase + PLAYER_FPS_RUN * dtimed1000;
			if(player->player->Fbase < 0 || player->player->Fbase > 3) player->player->Fbase = 0;
			if(!player->player->soundId_move)
			{
				player->player->soundId_move = sound_play_start(SOUND_PLAYER_TANKMOVE, -1);
			}

		}
		else
		{
			//игрок останавливается
			player->player->move.speed -= PLAYER_DECEL * dtime;
		};
		if(player->player->move.speed < 0)
		{
			if(player->player->soundId_move)
			{
				sound_play_stop(player->player->soundId_move);
				player->player->soundId_move = 0;
			}
			player->player->move.speed = 0;
		}
		player_move(player, player->dir, &player->player->move.speed);

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
	if(!player->player->attack)
	{
		//игрок не атакует
		if(player->player->reloadtime_d>0) player->player->reloadtime_d -= dtime;//учитываем время на перезарядку
	}
	else
	{
		//игрок атакует
		if(player->player->items[ITEM_HEALTH] <= 0)
		{
			if(
				((game.flags & c_g_f_2PLAYERS) != 0) &&
				((player->player->charact.status == c_p_P0)||(player->player->charact.status == c_p_P1))
				)
			{
				player_respawn(player);
				player->player->reloadtime_d = c_p_WEAP_reloadtime;
			};
		}
		else
		{
			if(player->player->reloadtime_d>0) player->player->reloadtime_d -= dtime;//учитываем время на перезарядку
			else
			{
				if(!player->player->bull)
				{
					int item;
					switch(player->player->weap)
					{
					case WEAP_ARTILLERY: item = ITEM_AMMO_ARTILLERY; break;
					case WEAP_MISSILE  : item = ITEM_AMMO_MISSILE; break;
					case WEAP_MINE     : item = ITEM_AMMO_MINE; break;
					default: item = ITEM_AMMO_ARTILLERY;
					}

					//если не стреляем управляемой ракетой
					if(
							(playerinfo->items[item] == ITEM_AMOUNT_INF)||            //если пуль бесконечно много
							(player->player->items[item] > 0)
					)
					{
						// пули не кончились
						player->player->reloadtime_d = c_p_WEAP_reloadtime;
						mobj_bulltype_t bulltype = mobj_weapon_type_to_bull_type(player->player->weap);
						mobj_t * bull = bull_new(
							player->pos.x,
							player->pos.y,
							bulltype,
							player->dir,
							player
							);                                                       //создаем пулю
						if(bulltype == BULL_MISSILE) player->player->bull = bull;
						//присоединяем изображение пули
						switch(player->player->weap)
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
								(player->player->charact.status==c_p_P0 || player->player->charact.status == c_p_P1) // игрок не монстр(у монстрюков пули не кончаются)
						)
							player->player->items[item]--;
					}
				}
			}
		}
	}
	if(player->player->reloadtime_d < 0) player->player->reloadtime_d = 0;
	//подбираем предметы
	player_item_get (player);
	//проверяем объекты
	player_obj_check(player);
}

/*
 * инициализируем игрока при спавне
 */
void player_spawn_init(mobj_t * player, const mobj_spawn_t * sp)
{
	if(0 <= sp->items[ITEM_SCORES] && sp->items[ITEM_SCORES] <= c_score_max)
		player->player->items[ITEM_SCORES] = sp->items[ITEM_SCORES];
	player_class_init(player);
	playerinfo_t * playerinfo = &playerinfo_table[player->player->level];

	if(0 <= sp->items[ITEM_HEALTH] && sp->items[ITEM_HEALTH] <= playerinfo->items[ITEM_HEALTH])
		player->player->items[ITEM_HEALTH] = sp->items[ITEM_HEALTH];
	else
	{
		if(player->player->charact.status == c_p_BOSS || player->player->charact.status == c_p_ENEMY)
			player->player->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
		else
		{
			if(game.flags & c_g_f_CASE)
				player->player->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
			else
			{
				//по уровням
				if(game.gamemap == mapList) // первая карта
					player->player->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
				else // не первая карта
					if(!player->player->charact.spawned && player->player->items[ITEM_HEALTH] <= 0)
						player->player->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
			}
		}
	}
	if(0 <= sp->items[ITEM_ARMOR] && sp->items[ITEM_ARMOR] <= playerinfo->items[ITEM_ARMOR] )
		player->player->items[ITEM_ARMOR] = sp->items[ITEM_ARMOR];
	else
	{
		if(player->player->charact.status == c_p_BOSS || player->player->charact.status == c_p_ENEMY)
			player->player->items[ITEM_ARMOR] = playerinfo->items[ITEM_ARMOR];
		else
			if(game.flags & c_g_f_CASE) player->player->items[ITEM_ARMOR] = playerinfo->items[ITEM_ARMOR];
	};
	player->player->charact.frags = 0;
	player->player->charact.spawned = true;
	player->player->bull            = NULL;
	player->player->move.speed      = 0;
	player->player->move.go         = false;
	player->dir          = DIR_UP;
	player->player->attack          = false;
	player->player->reloadtime_d    = 0;
	player->player->soundId_move    = 0;

	switch(player->player->charact.status)
	{
		case c_p_BOSS : player->player->Iflag = image_get(IMG_FLAG_USA);break;
		case c_p_ENEMY: player->player->Iflag = image_get(IMG_FLAG_WHITE);break;
		case c_p_P0   : player->player->Iflag = image_get(IMG_FLAG_RUS);break;
		case c_p_P1   : player->player->Iflag = image_get(IMG_FLAG_RUS);break;
	}

};


/**
 * получить любой спавн-поинт для игрока
 */
mobj_t * player_spawn_get()
{

	int count = 0;
	//считаем количество спавн-поинтов
	mobj_t * mobj;
	for(mobj = map.mobjs; mobj; mobj = mobj->next)
	{
		if(mobj->type == MOBJ_SPAWN && mobj->spawn.type == SPAWN_PLAYER) count++;
	};
	//выбираем случайным образом
	count = xrand(count);

	for(mobj = map.mobjs; mobj; mobj = mobj->next)
	{
		if(mobj->type == MOBJ_SPAWN && mobj->spawn.type == SPAWN_PLAYER)
		{
			if(count == 0) return mobj;
			count--;
		}
	};
	return NULL;
}


/**
 * появление/восстановление игрока на карте
 * @return = 0 -спавнинг прошел успешно
 * @return = 1 -игрок является монстром, ошибка спавнинга
 */
int player_respawn(mobj_t * player)
{
	if(
			player->player->charact.status != c_p_P0 &&
			player->player->charact.status != c_p_P1
	) return 1;//игрок является монстром, ошибка спавнинга

	mobj_t * spawn = player_spawn_get();

	player->pos.x = spawn->pos.x;
	player->pos.y = spawn->pos.y;
	player_spawn_init(player, &spawn->spawn);
	return 0;
}

mobj_t * player_create_player(const mobj_t * spawn)
{
	if(spawn->type != MOBJ_SPAWN) return NULL;

	const mobj_spawn_t * sp = &spawn->spawn;

	mobj_t * player = mobj_new(MOBJ_PLAYER, spawn->pos.x, spawn->pos.y, spawn->dir);

	player->player = Z_malloc(sizeof(mobj_player_t));

	int status;

	if(sp->type == SPAWN_ENEMY) status = c_p_ENEMY;
	if(sp->type == SPAWN_BOSS ) status = c_p_BOSS;
	if(sp->type == SPAWN_PLAYER) status = c_p_P0;

	player->player->charact.status     = status;

	if(status == c_p_ENEMY || status == c_p_BOSS)
		ctrl_AI_init(&player->player->brain);

	player_spawn_init(player, sp);

	return player;
}

/**
 * появление/восстановление игрока на карте
 * @return = 0 - спавнинг монстров прошел успешно
 * @return = 1 - слишком много игроков
 */
int player_create_enemy(const mobj_t * spawn)
{
	if(spawn->type != MOBJ_SPAWN) return -1;

	const mobj_spawn_t * sp = &spawn->spawn;

	if(sp->type != SPAWN_ENEMY && sp->type == SPAWN_BOSS ) return -1;

	mobj_t * player = mobj_new(MOBJ_PLAYER, spawn->pos.x, spawn->pos.y, spawn->dir);

	player->player = Z_malloc(sizeof(mobj_player_t));

	int status;

	if(sp->type == SPAWN_ENEMY) status = c_p_ENEMY;
	if(sp->type == SPAWN_BOSS ) status = c_p_BOSS;
	player->player->charact.status     = status;

	ctrl_AI_init(&player->player->brain);

	player_spawn_init(player, sp);


	return 0;
}

/*
 * вывод информации об игроке
 */
void player_draw_status(camera_t * cam, mobj_t * player)
{
	playerinfo_t * playerinfo = &playerinfo_table[player->player->level];
	int ref_y = VIDEO_SCREEN_H - 16 * 2;

	font_color_set3i(COLOR_1);
	// gr2D_settext(cam->x,cam_y,0,'('+realtostr(player->move.orig.x,8,4)+';'+realtostr(player->move.orig.y,8,4)+')');
	// gr2D_settext(cam->x,cam_y,0,"PING %d", player->time.delta);

	gr2D_setimage0(cam->x + 16 * 0     , ref_y, game.i_health);
	gr2D_setimage0(cam->x + 16 * 6     , ref_y, game.i_armor);

	video_printf(cam->x + 16 * 0 + 16, ref_y, orient_horiz, "%d", player->player->items[ITEM_HEALTH]);
	video_printf(cam->x + 16 * 6 + 16, ref_y, orient_horiz, "%d", player->player->items[ITEM_ARMOR]);
	video_printf(cam->x + 16 * 0 + 16, ref_y + 8, orient_horiz, "%d", playerinfo->items[ITEM_HEALTH]);
	video_printf(cam->x + 16 * 6 + 16, ref_y + 8, orient_horiz, "%d", playerinfo->items[ITEM_ARMOR]);

	/* вторая строка */
	ref_y += 16;

	gr2D_setimage1(cam->x + 16 * 0, ref_y, player->img, 0, 0, c_p_MDL_box,c_p_MDL_box);
	gr2D_setimage0(cam->x + 16 * 4, ref_y, image_get(wtable[0].icon));
	gr2D_setimage0(cam->x + 16 * 6, ref_y, image_get(wtable[1].icon));
	gr2D_setimage0(cam->x + 16 * 8, ref_y, image_get(wtable[2].icon));

	video_printf(cam->x + 16 * 4 + 16, ref_y + 4, orient_horiz, "@"); // player->items[ITEM_AMMO_ARTILLERY]
	if(player->player->items[ITEM_AMMO_MISSILE] >= 0)
		video_printf(cam->x + 16 * 6 + 16, ref_y + 4, orient_horiz, "%d", player->player->items[ITEM_AMMO_MISSILE]);
	if(player->player->items[ITEM_AMMO_MINE] >= 0)
		video_printf(cam->x + 16 * 8 + 16, ref_y + 4, orient_horiz , "%d", player->player->items[ITEM_AMMO_MINE]);
	video_printf(cam->x + 16 * 0 + 16, ref_y +  4, orient_horiz, "%d", player->player->items[ITEM_SCORES]);

}

/*
 * инициализация класса танка
 */
void player_class_init(mobj_t * player)
{
	player->player->Fbase = 0;                                                       //№ кадра(база)
	int level = player->player->items[ITEM_SCORES] / ITEM_SCOREPERCLASS;
	if(level > PLAYER_LEVEL5) level = PLAYER_LEVEL5;
	if(player->player->charact.status == c_p_BOSS) level = PLAYER_LEVEL_BOSS;
	player->player->level = level;
	playerinfo_t * playerinfo = &playerinfo_table[level];

	int i;
	for(i = 0; i< __ITEM_NUM; i++)
	{
		if(playerinfo->items[i] == ITEM_AMOUNT_NA)
			player->player->items[i] = playerinfo->items[i];
		else if(playerinfo->items[i] == ITEM_AMOUNT_INF)
			player->player->items[i] = playerinfo->items[i];
		else
		{
			if(player->player->items[i] < 0) player->player->items[i] = 0;
			if( player->player->charact.status==c_p_ENEMY || player->player->charact.status==c_p_BOSS )
			{
				player->player->items[i] = playerinfo->items[i];
			}
		}
	}

	player->img = image_get(playerinfo->imageindex);

}

/*
 * вычисление повреждения наносимого игроку
 */
void player_getdamage(mobj_t * player, mobj_t * explode, bool self, float radius)
{
	int damage_full;
	int armor;

	//weapon_info_t * weapinfo = &wtable[explode->explode.type];
	explodeinfo_t * explode_info = &explodeinfo_table[explode->explode.type];

	if( player->player->items[ITEM_HEALTH] > 0 && radius <= explode_info->radius )
	{
		if(self)
			damage_full = explode_info->selfdamage*(1-radius/explode_info->radius);
		else
			damage_full = explode_info->damage    *(1-radius/explode_info->radius);

		int damage_armor = damage_full*2/3;
		int damage_health = damage_full - damage_armor;

		armor = player->player->items[ITEM_ARMOR] - damage_armor;
		player->player->items[ITEM_HEALTH] = player->player->items[ITEM_HEALTH] - damage_health;
		if(armor < 0)
		{
			player->player->items[ITEM_HEALTH] = player->player->items[ITEM_HEALTH] + armor;
			player->player->items[ITEM_ARMOR] = 0;
		}
		else
			player->player->items[ITEM_ARMOR] = armor;
		if(player->player->items[ITEM_HEALTH] <= 0)
		{
			if(!self)
			{
				//атакующему добавим очки
				explode->explode.owner->player->items[ITEM_SCORES] += c_score_pertank;
				explode->explode.owner->player->charact.fragstotal++;
				explode->explode.owner->player->charact.frags++;
			}
			else
			{
				//атакующий умер от своей пули
				explode->explode.owner->player->items[ITEM_SCORES] = 0;
				explode->explode.owner->player->charact.fragstotal--;
				explode->explode.owner->player->charact.frags--;
			}
			player_class_init(explode->explode.owner);
		}
	}
}

