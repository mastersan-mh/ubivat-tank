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

player_t * playerList = NULL;


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

int player_spawn_player(player_t * player);

void player_moveUp_ON()
{
	if(!game.P0)return;
	game.P0->move.dir = DIR_UP;
	game.P0->move.go = true;
}

void player_moveUp_OFF()
{
	if(!game.P0)return;
	if(game.P0->move.dir != DIR_UP) return;
	game.P0->move.go = false;
}

void player_moveDown_ON()
{
	if(!game.P0)return;
	game.P0->move.dir = DIR_DOWN;
	game.P0->move.go = true;
}

void player_moveDown_OFF()
{
	if(!game.P0)return;
	if(game.P0->move.dir != DIR_DOWN) return;
	game.P0->move.go = false;
}

void player_moveLeft_ON()
{
	if(!game.P0)return;
	game.P0->move.dir = DIR_LEFT;
	game.P0->move.go = true;
}

void player_moveLeft_OFF()
{
	if(!game.P0)return;
	if(game.P0->move.dir != DIR_LEFT) return;
	game.P0->move.go = false;
}

void player_moveRight_ON()
{
	if(!game.P0)return;
	game.P0->move.dir = DIR_RIGHT;
	game.P0->move.go = true;
}

void player_moveRight_OFF()
{
	if(!game.P0)return;
	if(game.P0->move.dir != DIR_RIGHT) return;
	game.P0->move.go = false;
}

void player_attack_weapon1_ON()
{
	game.P0->weap = WEAP_ARTILLERY;
	game.P0->attack = true;
}
void player_attack_weapon1_OFF()
{
	game.P0->attack = false;
}

void player_attack_weapon2_ON()
{
	game.P0->weap = WEAP_MISSILE;
	game.P0->attack = true;
}
void player_attack_weapon2_OFF()
{
	game.P0->attack = false;
}

void player_attack_weapon3_ON()
{
	game.P0->weap = WEAP_MINE;
	game.P0->attack = true;
}
void player_attack_weapon3_OFF()
{
	game.P0->attack = false;
}

void player2_moveUp_ON()
{
	if(!game.P1)return;
	game.P1->move.dir = DIR_UP;
	game.P1->move.go = true;
}

void player2_moveUp_OFF()
{
	if(!game.P1)return;
	if(game.P1->move.dir != DIR_UP) return;
	game.P1->move.go = false;
}

void player2_moveDown_ON()
{
	if(!game.P1)return;
	game.P1->move.dir = DIR_DOWN;
	game.P1->move.go = true;
}

void player2_moveDown_OFF()
{
	if(!game.P1)return;
	if(game.P1->move.dir != DIR_DOWN) return;
	game.P1->move.go = false;
}

void player2_moveLeft_ON()
{
	if(!game.P1)return;
	game.P1->move.dir = DIR_LEFT;
	game.P1->move.go = true;
}

void player2_moveLeft_OFF()
{
	if(!game.P1)return;
	if(game.P1->move.dir != DIR_LEFT) return;
	game.P1->move.go = false;
}

void player2_moveRight_ON()
{
	if(!game.P1)return;
	game.P1->move.dir = DIR_RIGHT;
	game.P1->move.go = true;
}

void player2_moveRight_OFF()
{
	if(!game.P1)return;
	if(game.P1->move.dir != DIR_RIGHT) return;
	game.P1->move.go = false;
}

void player2_attack_weapon1_ON()
{
	if(!game.P1)return;
	game.P1->weap = WEAP_ARTILLERY;
	game.P1->attack = true;
}
void player2_attack_weapon1_OFF()
{
	if(!game.P1)return;
	game.P1->attack = false;
}

void player2_attack_weapon2_ON()
{
	if(!game.P1)return;
	game.P1->weap = WEAP_MISSILE;
	game.P1->attack = true;
}
void player2_attack_weapon2_OFF()
{
	if(!game.P1)return;
	game.P1->attack = false;
}

void player2_attack_weapon3_ON()
{
	if(!game.P1)return;
	game.P1->weap = WEAP_MINE;
	game.P1->attack = true;
}
void player2_attack_weapon3_OFF()
{
	if(!game.P1)return;
	game.P1->attack = false;
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
void player_item_get(player_t * player)
{
	if(!
			(
					(player->items[ITEM_HEALTH] > 0) &&
					((player->charact.status==c_p_P0)||(player->charact.status==c_p_P1))
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
				(mobj->pos.x - c_i_MDL_box/2 < player->move.pos.x + c_p_MDL_box/2 )&&
				(player->move.pos.x - c_p_MDL_box/2 <= mobj->pos.x + c_i_MDL_box/2)&&
				(mobj->pos.y - c_i_MDL_box/2 <= player->move.pos.y + c_p_MDL_box/2)&&
				(player->move.pos.y - c_p_MDL_box/2 < mobj->pos.y + c_i_MDL_box/2 )&&
				(mobj->type == MOBJ_ITEM)
		)
		{
			playerinfo_t *playerinfo = &playerinfo_table[player->level];
			itemtype_t itemtype = mobj->item.type;

			if(
					playerinfo->items[itemtype] != ITEM_AMOUNT_INF &&
					playerinfo->items[itemtype] != ITEM_AMOUNT_NA &&
					((player->items[itemtype] < playerinfo->items[itemtype]) || item->amount < 0)
			)
			{
				item->exist = false;
				player->items[itemtype] += item->amount;
				check_value_int(&player->items[itemtype], 0, playerinfo->items[itemtype]);
			};


			if(itemtype == ITEM_SCORES)
			{
				player_class_init(player);
				if(5 <= player->items[ITEM_SCORES] / ITEM_SCOREPERCLASS)
				{
					if(player->items[ITEM_HEALTH] < playerinfo->items[ITEM_HEALTH])
						player->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
					if(player->items[ITEM_ARMOR] < player->items[ITEM_ARMOR])
						player->items[ITEM_ARMOR] = playerinfo->items[ITEM_ARMOR];
				}
			}
		}
	} //end for
}

/*
 * проверка объектов на карте
 */
void player_obj_check(player_t * player)
{
	if(
			!((player->charact.status==c_p_P0)||(player->charact.status==c_p_P1))
	) return;

	mobj_t * mobj;
	for(mobj = map.mobjs; mobj; mobj = mobj->next)
	{
		if(
				(mobj->pos.x-c_o_MDL_box/2 <= player->move.pos.x + c_p_MDL_box / 2)&&
				(player->move.pos.x-c_p_MDL_box/2 <= mobj->pos.x + c_o_MDL_box / 2)&&
				(mobj->pos.y-c_o_MDL_box/2 <= player->move.pos.y + c_p_MDL_box / 2)&&
				(player->move.pos.y-c_p_MDL_box/2 <= mobj->pos.y + c_o_MDL_box / 2)
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
static void player_draw(camera_t * cam, player_t * player)
{
	if(player->items[ITEM_HEALTH] > 0)
	{ //если игрок жив
		if(
				(cam->pos.x-cam->sx/2<=player->move.pos.x+(c_p_MDL_box/2))&&(player->move.pos.x-(c_p_MDL_box/2)<=cam->pos.x+cam->sx/2)&&
				(cam->pos.y-cam->sy/2<=player->move.pos.y+(c_p_MDL_box/2))&&(player->move.pos.y-(c_p_MDL_box/2)<=cam->pos.y+cam->sy/2)
		)
		{
			gr2D_setimage1(
					round(cam->x+player->move.pos.x-(cam->pos.x-cam->sx/2))+c_p_MDL_pos,
					round(cam->y-player->move.pos.y+(cam->pos.y+cam->sy/2))+c_p_MDL_pos,
					player->Ibase,
					0,
					c_p_MDL_box*((player->move.dir * 4)+round(player->Fbase)),
					c_p_MDL_box,
					c_p_MDL_box
			);
			gr2D_setimage0(
					round(cam->x+player->move.pos.x-(cam->pos.x-cam->sx/2))+c_p_MDL_pos,
					round(cam->y-player->move.pos.y+(cam->pos.y+cam->sy/2))+c_p_MDL_pos,
					player->Iflag
			);
		};
	};
};

void player_draw_all(camera_t * cam)
{
	player_t * player = playerList;
	while(player)
	{
		player_draw(cam, player);
		player = player->next;
	}
}


/*
 * передвижение игрока
 */
static void player_move(player_t * player, int dir, coord_t * speed)
{
	pos_t * orig = &player->move.pos;
	coord_t dway = (*speed) * dtimed1000;
	coord_t halfbox = c_p_MDL_box/2;
	coord_t dist;

	map_clip_find_near(orig, c_p_MDL_box, dir, 0xF0, c_p_MDL_box, &dist);//найдем препятствия
	if(dist < dway + halfbox) dway = dist - halfbox;

	switch(dir)
	{
	case DIR_UP   : orig->y += dway; break;
	case DIR_DOWN : orig->y -= dway; break;
	case DIR_LEFT : orig->x -= dway; break;
	case DIR_RIGHT: orig->x += dway; break;
	}
};
/*
 * управление игроком
 */
static void player_handle(player_t * player)
{
	pos_t Sorig;
	coord_t L,R,U,D;
	coord_t speed_s;

	playerinfo_t * playerinfo = &playerinfo_table[player->level];
	if(player->items[ITEM_HEALTH] <= 0)
	{
		if(player->soundId_move)
		{
			sound_play_stop(player->soundId_move);
			player->soundId_move = 0;
		}

		//если игрок мертв
		if(player->charact.spawned)
		{
			explode_new(player->move.pos.x,player->move.pos.y, EXPLODE_MISSILE, player);
			player->charact.spawned = false;
			player->items[ITEM_ARMOR] = 0;
			player->items[ITEM_AMMO_MISSILE] = 0;
			player->items[ITEM_AMMO_MINE] = 0;
		};
		if(player->charact.status == c_p_BOSS) game._win_ = true;
	}
	else
	{
		//игрок жив
		if(player->bull)
		{
			player->bull->dir = player->move.dir;
			player->move.go = false;
		};
		if(player->move.go)
		{
			//игрок едет
			player->move.speed += PLAYER_ACCEL * dtime;
			if(player->move.speed > playerinfo->speed) player->move.speed = playerinfo->speed;

			player->Fbase = player->Fbase + PLAYER_FPS_RUN * dtimed1000;
			if(player->Fbase < 0 || player->Fbase > 3) player->Fbase = 0;
			if(!player->soundId_move)
			{
				player->soundId_move = sound_play_start(SOUND_PLAYER_TANKMOVE, -1);
			}

		}
		else
		{
			//игрок останавливается
			player->move.speed -= PLAYER_DECEL * dtime;
		};
		if(player->move.speed < 0)
		{
			if(player->soundId_move)
			{
				sound_play_stop(player->soundId_move);
				player->soundId_move = 0;
			}
			player->move.speed = 0;
		}
		player_move(player, player->move.dir, &player->move.speed);

		speed_s = playerinfo->speed / 4;

		//стрейф
		switch(player->move.dir){
		case DIR_UP:
		case DIR_DOWN:
			Sorig = player->move.pos;
			Sorig.x = Sorig.x-c_p_MDL_box/4;
			map_clip_find_near(&Sorig,c_p_MDL_box/2,player->move.dir,0xF0,c_p_MDL_box/2+2, &L);
			Sorig = player->move.pos;
			Sorig.x = Sorig.x+c_p_MDL_box/4;
			map_clip_find_near(&Sorig,c_p_MDL_box/2,player->move.dir,0xF0,c_p_MDL_box/2+2, &R);
			if((c_p_MDL_box/2<L) && (R-1<=c_p_MDL_box/2)) player_move(player,DIR_LEFT, &speed_s);//strafe left
			if((c_p_MDL_box/2<R) && (L-1<=c_p_MDL_box/2)) player_move(player,DIR_RIGHT, &speed_s);//strafe right
			break;
		case DIR_LEFT:
		case DIR_RIGHT:
			Sorig = player->move.pos;
			Sorig.y = Sorig.y-c_p_MDL_box/4;
			map_clip_find_near(&Sorig,c_p_MDL_box/2,player->move.dir,0xF0,c_p_MDL_box/2+2, &D);
			Sorig = player->move.pos;
			Sorig.y = Sorig.y+c_p_MDL_box/4;
			map_clip_find_near(&Sorig,c_p_MDL_box/2,player->move.dir,0xF0,c_p_MDL_box/2+2, &U);
			if((c_p_MDL_box/2<U)&&(D-1<=c_p_MDL_box/2)) player_move(player,DIR_UP, &speed_s);//strafe up
			if((c_p_MDL_box/2<D)&&(U-1<=c_p_MDL_box/2)) player_move(player,DIR_DOWN, &speed_s);//strafe down
			break;
		}
	}
//стрельба
	if(!player->attack)
	{
		//игрок не атакует
		if(player->reloadtime_d>0) player->reloadtime_d -= dtime;//учитываем время на перезарядку
	}
	else
	{
		//игрок атакует
		if(player->items[ITEM_HEALTH] <= 0)
		{
			if(
				((game.flags & c_g_f_2PLAYERS) != 0) &&
				((player->charact.status == c_p_P0)||(player->charact.status == c_p_P1))
				)
			{
				player_spawn_player(player);
				player->reloadtime_d = c_p_WEAP_reloadtime;
			};
		}
		else
		{
			if(player->reloadtime_d>0) player->reloadtime_d -= dtime;//учитываем время на перезарядку
			else
			{
				if(!player->bull)
				{
					int item;
					switch(player->weap)
					{
					case WEAP_ARTILLERY: item = ITEM_AMMO_ARTILLERY; break;
					case WEAP_MISSILE  : item = ITEM_AMMO_MISSILE; break;
					case WEAP_MINE     : item = ITEM_AMMO_MINE; break;
					default: item = ITEM_AMMO_ARTILLERY;
					}

					//если не стреляем управляемой ракетой
					if(
							(playerinfo->items[item] == ITEM_AMOUNT_INF)||            //если пуль бесконечно много
							(player->items[item] > 0)
					)
					{
						// пули не кончились
						player->reloadtime_d = c_p_WEAP_reloadtime;
						mobj_bulltype_t bulltype = mobj_weapon_type_to_bull_type(player->weap);
						mobj_t * bull = bull_new(
							player->move.pos.x,
							player->move.pos.y,
							bulltype,
							player->move.dir,
							player
							);                                                       //создаем пулю
						if(bulltype == BULL_MISSILE) player->bull = bull;
						//присоединяем изображение пули
						switch(player->weap)
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
								(player->charact.status==c_p_P0 || player->charact.status == c_p_P1) // игрок не монстр(у монстрюков пули не кончаются)
						)
							player->items[item]--;
					}
				}
			}
		}
	}
	if(player->reloadtime_d < 0) player->reloadtime_d = 0;
	//подбираем предметы
	player_item_get (player);
	//проверяем объекты
	player_obj_check(player);
}
/*
 * поиск игрока
 */
player_t * player_find(int status)
{
	player_t * player;

	player = playerList;
	while((player)&&(player->charact.status != status))
		player = player->next;
	return player;
};
/**
 * подключение игрока к игре
 * @return = 0 подключение удалось
 * @return = 1 слишком много игроков
 * @return = 2 игрок P0/P1/BOSS уже присоединен
 */
int player_connect(int status)
{
	player_t * player = NULL;
	int error = 0;
	bool match;
	int id = 0;
	if(status != c_p_ENEMY && status != c_p_BOSS) player = player_find(status);
	//ирок P0/P1 присоединен
	if(player) error = 2;
	else
	{
		//ирок P0/P1 не присоединен
		if(playerList)
		{
			//список игроков создан
			error = 1;
			match = false;
			//пытаемся соединить игрока с сервером,
			while( id < c_p_Pmax && error == 1)
			{
				// пока не исчерпан лимит на кол-во игроков
				player = playerList;
				//ищем, чтобы небыло совпадения идентификаторов
				while( !match && player )
				{
					if(player->charact.id == id) match = true;
					else player = player->next;
				};
				if(!match) error = 0;                                         //все номера свободны, коннект удался
				else
				{
					match = false;
					id = id+1;
				}
			}
		}
	}
	//нет ошибок, инициализируем игрока
	if(!error)
	{
		//создаем игрока
		player = Z_malloc(sizeof(*player));
		player->next = playerList;
		playerList = player;
		player->charact.id         = id;
		player->charact.status     = status;
		ctrl_AI_init(&player->brain);
	};
	if( !error || error == 2 )
	{
		switch(player->charact.status)
		{
		case c_p_BOSS : player->Iflag = image_get(IMG_FLAG_USA);break;
		case c_p_ENEMY: player->Iflag = image_get(IMG_FLAG_WHITE);break;
		case c_p_P0   :
			game.P0 = player;
			player->charact.frags = 0;
			player->Iflag = image_get(IMG_FLAG_RUS);
			break;
		case c_p_P1   :
			game.P1 = player;
			player->charact.frags = 0;
			player->Iflag = image_get(IMG_FLAG_RUS);
			break;
		}
		player->charact.spawned = false;
	}
	return error;
}
/*
 * отключение игрока от игры
 *
 * @return = 0 успешно
 * @return = 1 игроков нет
 * @return = 2 игрок отсутствует
 */
static int player_disconnect(player_t ** player)
{
	player_t * p;

	if(!playerList)
	{
		*player = NULL;
		return 1;
	}
	if(playerList == *player)
	{
		playerList = playerList->next;
		sound_play_stop((*player)->soundId_move);
		ctrl_AI_done(&((*player)->brain));
		Z_free(*player);
		*player = playerList;
	}
	else
	{
		p = playerList;
		while(p && p->next != *player) p = p->next;
		if(!p) return 2;
		p->next = (*player)->next;
		sound_play_stop((*player)->soundId_move);
		ctrl_AI_done(&((*player)->brain));
		Z_free(*player);
		*player = p->next;
	}
	return 0;
}
/*
 * отключение всех монстров от игры
 */
void player_disconnect_monsters()
{
	player_t * player;

	player = playerList;
	while(player)
	{
		if(
				(player->charact.status == c_p_ENEMY)||
				(player->charact.status == c_p_BOSS )
		) player_disconnect(&player);
		else player = player->next;
	};
};
/*
 * отключение всех игроков от игры
 */
void player_disconnect_all()
{
	player_t * player;
	game.P0 = NULL;
	game.P1 = NULL;
	while(playerList)
	{
		player = playerList;
		sound_play_stop(player->soundId_move);
		playerList = playerList->next;
		Z_free(player);
	};
};
/*
 * спавним игрока
 */
void player_spawn(player_t * player, mobj_t * mobj)
{
	mobj_spawn_t * sp = &mobj->spawn;
	if(0 <= sp->items[ITEM_SCORES] && sp->items[ITEM_SCORES] <= c_score_max)
		player->items[ITEM_SCORES] = sp->items[ITEM_SCORES];
	player_class_init(player);
	playerinfo_t * playerinfo = &playerinfo_table[player->level];

	if(0 <= sp->items[ITEM_HEALTH] && sp->items[ITEM_HEALTH] <= playerinfo->items[ITEM_HEALTH])
		player->items[ITEM_HEALTH] = sp->items[ITEM_HEALTH];
	else
	{
		if(player->charact.status == c_p_BOSS || player->charact.status == c_p_ENEMY)
			player->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
		else
		{
			if(game.flags & c_g_f_CASE)
				player->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
			else
			{
				//по уровням
				if(game.gamemap == mapList) // первая карта
					player->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
				else // не первая карта
					if(!player->charact.spawned && player->items[ITEM_HEALTH] <= 0)
						player->items[ITEM_HEALTH] = playerinfo->items[ITEM_HEALTH];
			}
		}
	}
	if(0 <= sp->items[ITEM_ARMOR] && sp->items[ITEM_ARMOR] <= playerinfo->items[ITEM_ARMOR] )
		player->items[ITEM_ARMOR] = sp->items[ITEM_ARMOR];
	else
	{
		if(player->charact.status == c_p_BOSS || player->charact.status == c_p_ENEMY)
			player->items[ITEM_ARMOR] = playerinfo->items[ITEM_ARMOR];
		else
			if(game.flags & c_g_f_CASE) player->items[ITEM_ARMOR] = playerinfo->items[ITEM_ARMOR];
	};
	player->charact.spawned   = true;
	player->bull              = NULL;
	player->move.speed        = 0;
	player->move.go           = false;
	player->move.dir          = DIR_UP;
	player->attack            = false;
	player->reloadtime_d    = 0;
	player->soundId_move      = 0;
};
/*
 * появление/восстановление игрока на карте
 * @return = 0 -спавнинг прошел успешно
 * @return = 1 -игрок является монстром, ошибка спавнинга
 */
int player_spawn_player(player_t * player)
{
	if(
			player->charact.status != c_p_P0 &&
			player->charact.status != c_p_P1
	) return 1;//игрок является монстром, ошибка спавнинга

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
			if(count == 0) break;
			count--;
		}
	};

	player->move.pos.x = mobj->pos.x;
	player->move.pos.y = mobj->pos.y;
	player_spawn(player, mobj);
	return 0;
}

/*
 * появление/восстановление игрока на карте
 * @return = 0 -спавнинг монстров прошел успешно
 * @return = 1 -слишком много игроков
 */
int player_spawn_enemy()
{
	int ret = 0;

	mobj_t * mobj = map.mobjs;
	while(mobj && ret != 1)
	{
		//пока не привысили лимит на игроков
		if( mobj->type == MOBJ_SPAWN && (mobj->spawn.type == SPAWN_ENEMY || mobj->spawn.type == SPAWN_BOSS ))
		{
			//это monster спавн
			if(mobj->spawn.type == SPAWN_ENEMY) ret = player_connect(c_p_ENEMY);
			if(mobj->spawn.type == SPAWN_BOSS) ret = player_connect(c_p_BOSS );
			if(ret != 1)
			{
				playerList->move.pos.x = mobj->pos.x;
				playerList->move.pos.y = mobj->pos.y;
				player_spawn(playerList, mobj);
			}
		}
		mobj = mobj->next;
	}
	return ret;
}
/*
 * спавним всех игроков и монстров
 */
void player_spawn_all()
{
	player_t * player;
	player = playerList;
	while(player)
	{
		player_spawn_player(player);
		player = player->next;
	};
	player_spawn_enemy();
}


void players_control()
{
	player_t * player = playerList;
	while(player)
	{
		switch(player->charact.status)
		{
		case c_p_BOSS:
		case c_p_ENEMY:
			think_enemy(player);
			break;
		case c_p_P0:
			think_human(0, player);
			break;
		case c_p_P1:
			think_human(1, player);
			break;
		}
		player_handle(player);
		player = player->next;
	}
}



/*
 * вывод информации об игроке
 */
void player_draw_status(camera_t * cam, player_t * player)
{
	playerinfo_t * playerinfo = &playerinfo_table[player->level];
	int ref_y = VIDEO_SCREEN_H - 16 * 2;

	font_color_set3i(COLOR_1);
	// gr2D_settext(cam->x,cam_y,0,'('+realtostr(player->move.orig.x,8,4)+';'+realtostr(player->move.orig.y,8,4)+')');
	// gr2D_settext(cam->x,cam_y,0,"PING %d", player->time.delta);

	gr2D_setimage0(cam->x + 16 * 0     , ref_y, game.i_health);
	gr2D_setimage0(cam->x + 16 * 6     , ref_y, game.i_armor);

	video_printf(cam->x + 16 * 0 + 16, ref_y, orient_horiz, "%d", player->items[ITEM_HEALTH]);
	video_printf(cam->x + 16 * 6 + 16, ref_y, orient_horiz, "%d", player->items[ITEM_ARMOR]);
	video_printf(cam->x + 16 * 0 + 16, ref_y + 8, orient_horiz, "%d", playerinfo->items[ITEM_HEALTH]);
	video_printf(cam->x + 16 * 6 + 16, ref_y + 8, orient_horiz, "%d", playerinfo->items[ITEM_ARMOR]);

	/* вторая строка */
	ref_y += 16;

	gr2D_setimage1(cam->x + 16 * 0, ref_y, player->Ibase, 0, 0, c_p_MDL_box,c_p_MDL_box);
	gr2D_setimage0(cam->x + 16 * 4, ref_y, image_get(wtable[0].icon));
	gr2D_setimage0(cam->x + 16 * 6, ref_y, image_get(wtable[1].icon));
	gr2D_setimage0(cam->x + 16 * 8, ref_y, image_get(wtable[2].icon));

	video_printf(cam->x + 16 * 4 + 16, ref_y + 4, orient_horiz, "@"); // player->items[ITEM_AMMO_ARTILLERY]
	if(player->items[ITEM_AMMO_MISSILE] >= 0)
		video_printf(cam->x + 16 * 6 + 16, ref_y + 4, orient_horiz, "%d", player->items[ITEM_AMMO_MISSILE]);
	if(player->items[ITEM_AMMO_MINE] >= 0)
		video_printf(cam->x + 16 * 8 + 16, ref_y + 4, orient_horiz , "%d", player->items[ITEM_AMMO_MINE]);
	video_printf(cam->x + 16 * 0 + 16, ref_y +  4, orient_horiz, "%d", player->items[ITEM_SCORES]);

}

/*
 * инициализация класса танка
 */
void player_class_init(player_t * player)
{
	player->Fbase = 0;                                                       //№ кадра(база)
	int level = player->items[ITEM_SCORES] / ITEM_SCOREPERCLASS;
	if(level > PLAYER_LEVEL5) level = PLAYER_LEVEL5;
	if(player->charact.status == c_p_BOSS) level = PLAYER_LEVEL_BOSS;
	player->level = level;
	playerinfo_t * playerinfo = &playerinfo_table[level];

	int i;
	for(i = 0; i< __ITEM_NUM; i++)
	{
		if(playerinfo->items[i] == ITEM_AMOUNT_NA)
			player->items[i] = playerinfo->items[i];
		else if(playerinfo->items[i] == ITEM_AMOUNT_INF)
			player->items[i] = playerinfo->items[i];
		else
		{
			if(player->items[i] < 0) player->items[i] = 0;
			if( player->charact.status==c_p_ENEMY || player->charact.status==c_p_BOSS )
			{
				player->items[i] = playerinfo->items[i];
			}
		}
	}

	player->Ibase = image_get(playerinfo->imageindex);

}

/*
 * вычисление повреждения наносимого игроку
 */
void player_getdamage(player_t * player, mobj_t * explode, bool self, float radius)
{
	int damage_full;
	int armor;

	//weapon_info_t * weapinfo = &wtable[explode->explode.type];
	explodeinfo_t * explode_info = &explodeinfo_table[explode->explode.type];

	if( player->items[ITEM_HEALTH] > 0 && radius <= explode_info->radius )
	{
		if(self)
			damage_full = explode_info->selfdamage*(1-radius/explode_info->radius);
		else
			damage_full = explode_info->damage    *(1-radius/explode_info->radius);

		int damage_armor = damage_full*2/3;
		int damage_health = damage_full - damage_armor;

		armor = player->items[ITEM_ARMOR] - damage_armor;
		player->items[ITEM_HEALTH] = player->items[ITEM_HEALTH] - damage_health;
		if(armor < 0)
		{
			player->items[ITEM_HEALTH] = player->items[ITEM_HEALTH] + armor;
			player->items[ITEM_ARMOR] = 0;
		}
		else
			player->items[ITEM_ARMOR] = armor;
		if(player->items[ITEM_HEALTH] <= 0)
		{
			if(!self)
			{
				//атакующему добавим очки
				explode->explode.owner->items[ITEM_SCORES] += c_score_pertank;
				explode->explode.owner->charact.fragstotal++;
				explode->explode.owner->charact.frags++;
			}
			else
			{
				//атакующий умер от своей пули
				explode->explode.owner->items[ITEM_SCORES] = 0;
				explode->explode.owner->charact.fragstotal--;
				explode->explode.owner->charact.frags--;
			}
			player_class_init(explode->explode.owner);
		}
	}
}

