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

#include <string.h>
#include <think.h>
#include <types.h>

player_t * playerList = NULL;

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
	game.P0->w.attack = WEAP_BULL;
}
void player_attack_weapon1_OFF()
{
	game.P0->w.attack = WEAP_NONE;
}

void player_attack_weapon2_ON()
{
	game.P0->w.attack = WEAP_ROCKET;
}
void player_attack_weapon2_OFF()
{
	game.P0->w.attack = WEAP_NONE;
}

void player_attack_weapon3_ON()
{
	game.P0->w.attack = WEAP_MINE;
}
void player_attack_weapon3_OFF()
{
	game.P0->w.attack = WEAP_NONE;
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
	game.P1->w.attack = WEAP_BULL;
}
void player2_attack_weapon1_OFF()
{
	if(!game.P1)return;
	game.P1->w.attack = WEAP_NONE;
}

void player2_attack_weapon2_ON()
{
	if(!game.P1)return;
	game.P1->w.attack = WEAP_ROCKET;
}
void player2_attack_weapon2_OFF()
{
	if(!game.P1)return;
	game.P1->w.attack = WEAP_NONE;
}

void player2_attack_weapon3_ON()
{
	if(!game.P1)return;
	game.P1->w.attack = WEAP_MINE;
}
void player2_attack_weapon3_OFF()
{
	if(!game.P1)return;
	game.P1->w.attack = WEAP_NONE;
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
/*
 * подбирание предметов игроком
 */
void player_item_get(player_t * player)
{
	item_t * item;

	if(
			(player->charact.health>0) &&                                        //игрок живой и
			((player->charact.status==c_p_P0)||(player->charact.status==c_p_P1))
	)
	{// не монстр(монстрюки оружие не подбирают)
		item = map.items;
		while(item)
		{
			if(item->exist)
			{
				//если предмет есть
				if(
						(item->orig.x-c_i_MDL_box/2< player->move.pos.x+c_p_MDL_box/2)&&
						(player->move.pos.x-c_p_MDL_box/2<=item->orig.x+c_i_MDL_box/2)&&
						(item->orig.y-c_i_MDL_box/2<=player->move.pos.y+c_p_MDL_box/2)&&
						(player->move.pos.y-c_p_MDL_box/2< item->orig.y+c_i_MDL_box/2)
				)
				{
					switch(item->class)
					{
					case MAP_ITEM_HEALTH:
						if((player->charact.health<player->charact.healthmax)||(item->amount<0))
						{
							item->exist = false;
							if(player->charact.healthmax < player->charact.health + item->amount)
								player->charact.health = player->charact.healthmax;
							else
								player->charact.health += item->amount;
						};
						break;
					case MAP_ITEM_ARMOR:
						if((player->charact.armor<player->charact.armormax)||(item->amount<0)) {
							item->exist = false;
							if(player->charact.armormax<player->charact.armor+item->amount)
								player->charact.armor = player->charact.armormax;
							else
								player->charact.armor+= item->amount;
							if(player->charact.armor<0) player->charact.armor = 0;
						};
						break;
					case MAP_ITEM_STAR  :
						if((player->charact.scores<c_score_max)||(item->amount<0))
						{
							item->exist = false;
							if(c_score_max<player->charact.scores+item->amount)
								player->charact.scores = c_score_max;
							else
								player->charact.scores+= item->amount;
							if(player->charact.scores<0) player->charact.scores = 0;
						};
						player_class_init(player);
						if(5<=player->charact.scores / c_score_perclass)
						{
							if(player->charact.health<player->charact.healthmax) player->charact.health = player->charact.healthmax;
							if(player->charact.armor <player->charact.armormax ) player->charact.armor  = player->charact.armormax ;
						};
						break;
					case MAP_ITEM_ROCKET:
						if((player->w.ammo[1] != PLAYER_WEAP_NOTACCESSIBLE)&&((player->w.ammo[1]<wtable[1].ammo)||(item->amount<0)))
						{
							item->exist = false;
							if(wtable[1].ammo<player->w.ammo[1]+item->amount)
								player->w.ammo[1] = wtable[1].ammo;
							else
								player->w.ammo[1]+= item->amount;
							if(player->w.ammo[1]<0) player->w.ammo[1] = 0;
						};
						break;
					case MAP_ITEM_MINE  :
						if((player->w.ammo[2] != PLAYER_WEAP_NOTACCESSIBLE)&&((player->w.ammo[2]<wtable[2].ammo)||(item->amount<0)))
						{
							item->exist = false;
							if(wtable[2].ammo<player->w.ammo[2]+item->amount)
								player->w.ammo[2] = wtable[2].ammo;
							else
								player->w.ammo[2]+= item->amount;
							if(player->w.ammo[2]<0) player->w.ammo[2] = 0;
						};
						break;
					default: ;
					}
				}
			}
			item = item->next;
		}
	}
}

/*
 * проверка объектов на карте
 */
void player_obj_check(player_t * player)
{
	obj_t * obj;
	if((player->charact.status==c_p_P0)||(player->charact.status==c_p_P1))
	{
		obj = map.objs;
		while(obj)
		{
			if(
					(obj->orig.x-c_o_MDL_box/2 <= player->move.pos.x + c_p_MDL_box / 2)&&
					(player->move.pos.x-c_p_MDL_box/2 <= obj->orig.x + c_o_MDL_box / 2)&&
					(obj->orig.y-c_o_MDL_box/2 <= player->move.pos.y + c_p_MDL_box / 2)&&
					(player->move.pos.y-c_p_MDL_box/2 <= obj->orig.y + c_o_MDL_box / 2)
			)
			{
				switch (obj->class)
				{
				case MAP_OBJ_EXIT: game._win_ = true;break;
				case MAP_OBJ_MESS: break;
				default: ;
				}
				//отправим сообщение игроку
				game_message_send(obj->message);
			};
		obj = obj->next;
		};
	};
};
/*
 * рисование игрока
 */
static void player_draw(camera_t * cam, player_t * player)
{
	if(player->charact.health>0)
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

	if(player->charact.health<=0)
	{
		//если игрок мертв
		if(player->charact.spawned)
		{
			bull_add();
			bullList->player = player;
			bullList->_weap_ = 1;
			explode_add(bullList,player->move.pos.x,player->move.pos.y);
			bull_remove(&bullList);
			player->charact.spawned = false;
			player->charact.armor = 0;
			if(0<player->w.ammo[1]) player->w.ammo[1] = 0;
			if(0<player->w.ammo[2]) player->w.ammo[2] = 0;
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
			if(player->charact.speed < player->move.speed) player->move.speed = player->charact.speed;

			player->Fbase = player->Fbase + PLAYER_FPS_RUN * dtimed1000;
			if(player->Fbase < 0 || player->Fbase > 3) player->Fbase = 0;

		}
		else
		{
			//игрок останавливается
			player->move.speed -= PLAYER_DECEL * dtime;
		};
		if(player->move.speed < 0) player->move.speed = 0;
		player_move(player, player->move.dir, &player->move.speed);

		speed_s = player->charact.speed / 4;

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
	if(player->w.attack == 0)
	{
		//игрок не атакует
		if(player->w.reloadtime_d>0) player->w.reloadtime_d -= dtime;//учитываем время на перезарядку
	}
	else
	{
		//игрок атакует
		if(player->charact.health<=0)
		{
			if(
				((game.flags & c_g_f_2PLAYERS) != 0) &&
				((player->charact.status == c_p_P0)||(player->charact.status == c_p_P1))
				)
			{
				player_spawn_player(player);
				player->w.reloadtime_d = c_p_WEAP_reloadtime;
			};
		}
		else
		{
			if(player->w.reloadtime_d>0) player->w.reloadtime_d -= dtime;//учитываем время на перезарядку
			else
			{
				if(!player->bull)
				{
					//если не стреляем управляемой ракетой
					if(
							(wtable[player->w.attack-1].ammo == c_WEAP_indefinit)||            //если пуль бесконечно много
							(player->w.ammo[player->w.attack-1]>0)
					)
					{
						// пули не кончились
						player->w.reloadtime_d = c_p_WEAP_reloadtime;
						bull_add();                                                       //создаем пулю
						bullList->pos.x   = player->move.pos.x;                     //координаты
						bullList->pos.y   = player->move.pos.y;                     //координаты
						bullList->player   = player;                                  //игрок, выпустивший пулю
						bullList->_weap_   = player->w.attack-1;                      //тип пули(оружие, из которого выпущена пуля)
						if(bullList->_weap_ == 1) player->bull = bullList;
						bullList->dir    = player->move.dir;                          //направление движения
						bullList->delta_s  = 0;                                     //изменение расстояния
						bullList->frame    = 0;
						//присоединяем изображение пули
						switch(player->w.attack)
						{
						case 1:bullList->image = IMG_connect("B_BULL"  );break;
						case 2:bullList->image = IMG_connect("B_ROCKET");break;
						case 3:bullList->image = IMG_connect("B_MINE"  );break;
						default: bullList->image = NULL;
						};
						if(
								wtable[player->w.attack-1].ammo > 0 && //если пули у оружия не бесконечны и
								(player->charact.status==c_p_P0 || player->charact.status == c_p_P1) // игрок не монстр(у монстрюков пули не кончаются)
						)
							player->w.ammo[player->w.attack-1]--;
					}
				}
			}
		}
	}
	if(player->w.reloadtime_d < 0) player->w.reloadtime_d = 0;
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
		player->charact.scores     = 0;
		player->charact.id         = id;
		player->charact.status     = status;
		player->charact.health     = 0;
		player->charact.armor      = 0;
		player->charact.fragstotal = 0;
		player->charact.frags      = 0;
		player->w.ammo[0] = 0;
		player->w.ammo[1] = 0;
		player->w.ammo[2] = 0;
		ctrl_AI_init(&player->brain);
	};
	if( !error || error == 2 )
	{
		switch(player->charact.status)
		{
		case c_p_BOSS : player->Iflag = IMG_connect("F_USA");break;
		case c_p_ENEMY: player->Iflag = IMG_connect("F_WHITE");break;
		case c_p_P0   :
			game.P0 = player;
			player->charact.frags = 0;
			player->Iflag = IMG_connect("F_RUS");
			break;
		case c_p_P1   :
			game.P1 = player;
			player->charact.frags = 0;
			player->Iflag = IMG_connect("F_RUS");
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
	if(playerList == *player) {
		playerList = playerList->next;
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
		playerList = playerList->next;
		Z_free(player);
	};
};
/*
 * спавним игрока
 */
void player_spawn(player_t * player, spawn_t * spawn)
{
	if(-1 < spawn->scores && spawn->scores <= c_score_max) player->charact.scores = spawn->scores;
	player_class_init(player);
	if(-1 < spawn->health && spawn->health <= player->charact.healthmax) player->charact.health = spawn->health;
	else
	{
		if(player->charact.status == c_p_BOSS || player->charact.status == c_p_ENEMY)
			player->charact.health = player->charact.healthmax;
		else
		{
			if(game.flags & c_g_f_CASE) player->charact.health = player->charact.healthmax;//по выбору
			else
			{
				//по уровням
				if(game.gamemap == mapList) player->charact.health = player->charact.healthmax;//первая карта
				else //не первая карта
					if(!player->charact.spawned && player->charact.health <= 0)
						player->charact.health = player->charact.healthmax;
			}
		}
	}
	if(-1 < spawn->armor && spawn->armor <= player->charact.armormax )
		player->charact.armor = spawn->armor;
	else
	{
		if(player->charact.status == c_p_BOSS || player->charact.status == c_p_ENEMY)
			player->charact.armor = player->charact.armormax;
		else
			if(game.flags & c_g_f_CASE) player->charact.armor = player->charact.armormax;
	};
	player->charact.spawned   = true;
	player->bull              = NULL;
	player->move.speed        = 0;
	player->move.go           = false;
	player->move.dir          = 0;
	player->w.attack          = false;
	player->w.reloadtime_d    = 0;
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

	spawn_t * spawn;
	int count = 0;
	spawn = map.spawns;
	//считаем количество спавн-поинтов
	while(spawn)
	{
		if(spawn->class == MAP_SPAWN_PLAYER) count++;
		spawn = spawn->next;
	};
	//выбираем случайным образом
	count = xrand(count);

	spawn = map.spawns;
	while(spawn)
	{
		if(spawn->class == MAP_SPAWN_PLAYER)
		{
			if(count == 0) break;
			count--;
		}
		spawn = spawn->next;
	};

	player->move.pos.x = spawn->orig.x;
	player->move.pos.y = spawn->orig.y;
	player_spawn(player, spawn);
	return 0;
}

/*
 * появление/восстановление игрока на карте
 * @return = 0 -спавнинг монстров прошел успешно
 * @return = 1 -слишком много игроков
 */
int player_spawn_enemy()
{
	spawn_t * spawn;
	int error = 0;

	spawn = map.spawns;
	while(spawn && error != 1)
	{
		//пока не привысили лимит на игроков
		if( spawn->class == MAP_SPAWN_ENEMY || spawn->class == MAP_SPAWN_BOSS )
		{
			//это monster спавн
			if(spawn->class == MAP_SPAWN_ENEMY) error = player_connect(c_p_ENEMY);
			if(spawn->class == MAP_SPAWN_BOSS ) error = player_connect(c_p_BOSS );
			if(error != 1)
			{
				playerList->move.pos.x = spawn->orig.x;
				playerList->move.pos.y = spawn->orig.y;
				player_spawn(playerList, spawn);
			}
		}
		spawn = spawn->next;
	}
	return error;
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
	int ref_y = VIDEO_SCREEN_H - 16 * 2;

	gr2D.color.current = 1;
	// gr2D_settext(cam->x,cam_y,0,'('+realtostr(player->move.orig.x,8,4)+';'+realtostr(player->move.orig.y,8,4)+')');
	// gr2D_settext(cam->x,cam_y,0,"PING %d", player->time.delta);

	gr2D_setimage0(cam->x + 16 * 0     , ref_y, game.i_health);
	gr2D_setimage0(cam->x + 16 * 6     , ref_y, game.i_armor);

	video_printf(cam->x + 16 * 0 + 16, ref_y, orient_horiz, "%d", player->charact.health);
	video_printf(cam->x + 16 * 6 + 16, ref_y, orient_horiz, "%d", player->charact.armor);
	video_printf(cam->x + 16 * 0 + 16, ref_y + 8, orient_horiz, "%d", player->charact.healthmax);
	video_printf(cam->x + 16 * 6 + 16, ref_y + 8, orient_horiz, "%d", player->charact.armormax);

	/* вторая строка */
	ref_y += 16;

	gr2D_setimage1(cam->x + 16 * 0, ref_y, player->Ibase, 0, 0, c_p_MDL_box,c_p_MDL_box);
	gr2D_setimage0(cam->x + 16 * 4, ref_y, wtable[0].icon);
	gr2D_setimage0(cam->x + 16 * 6, ref_y, wtable[1].icon);
	gr2D_setimage0(cam->x + 16 * 8, ref_y, wtable[2].icon);

	video_printf(cam->x + 16 * 4 + 16, ref_y + 4, orient_horiz, "%d", player->w.ammo[0]);
	if(player->w.ammo[1]>-1)
		video_printf(cam->x + 16 * 6 + 16, ref_y + 4, orient_horiz, "%d", player->w.ammo[1]);
	if(player->w.ammo[2]>-1)
		video_printf(cam->x + 16 * 8 + 16, ref_y + 4, orient_horiz , "%d", player->w.ammo[2]);
	video_printf(cam->x + 16 * 0 + 16, ref_y +  4, orient_horiz, "%d", player->charact.scores);

}

/*
 * инициализация класса танка
 */
void player_class_init(player_t * player)
{
	long level;

	player->Fbase = 0;                                                       //№ кадра(база)
	level = player->charact.scores / c_score_perclass;
	if(level>4) level = 4;

	switch(level)
	{
		case 0:                                                             //нулевой
			player->charact.healthmax = 100;                                      //здоровье max
			player->charact.armormax  = 000;                                      //броня max
			player->charact.speed = 40/2 * SPEEDSCALE;                                          //максимальная скорость при ходьбе
			player->w.ammo[0] = 99;
			player->w.ammo[1] = PLAYER_WEAP_NOTACCESSIBLE;
			player->w.ammo[2] = PLAYER_WEAP_NOTACCESSIBLE;
			player->Ibase = IMG_connect("TANK0");                    //база
			break;
		case 1:
			player->charact.healthmax = 100;                                      //здоровье max
			player->charact.armormax  = 050;                                      //броня max
			player->charact.speed = 50/2 * SPEEDSCALE;                                          //максимальная скорость при ходьбе
			player->w.ammo[0] = 99;
			player->w.ammo[1] = PLAYER_WEAP_NOTACCESSIBLE;
			player->w.ammo[2] = PLAYER_WEAP_NOTACCESSIBLE;
			player->Ibase = IMG_connect("TANK1");                    //база
			break;
		case 2:
			player->charact.healthmax = 100;                                      //здоровье max
			player->charact.armormax  = 100;                                      //броня max
			player->charact.speed = 60/2 * SPEEDSCALE;                                          //максимальная скорость при ходьбе
			player->w.ammo[0] = 99;
			if((player->charact.status==c_p_ENEMY)||(player->charact.status==c_p_BOSS ))
				player->w.ammo[1] = 99;
			else if(player->w.ammo[1]<0) player->w.ammo[1] = 0;
			player->w.ammo[2] = PLAYER_WEAP_NOTACCESSIBLE;
			player->Ibase = IMG_connect("TANK2");                    //база
			break;
		case 3:
			player->charact.healthmax = 200;                                      //здоровье max
			player->charact.armormax  = 150;                                      //броня max
			player->charact.speed = 70/2* SPEEDSCALE;                                          //максимальная скорость при ходьбе
			player->w.ammo[0] = 99;
			if((player->charact.status==c_p_ENEMY)||(player->charact.status==c_p_BOSS ))
				player->w.ammo[1] = 99;
			else
				if(player->w.ammo[1]<0) player->w.ammo[1] = 0;
			player->w.ammo[2] = PLAYER_WEAP_NOTACCESSIBLE;
			player->Ibase = IMG_connect("TANK3");                    //база
			break;
		case 4:
			player->charact.healthmax = 200;                                      //здоровье max
			player->charact.armormax  = 200;                                      //броня max
			player->charact.speed = 90/2 * SPEEDSCALE;                                          //максимальная скорость при ходьбе
			player->w.ammo[0] = 99;

			//player->w.ammo[1] = 90;
			//player->w.ammo[2] = 90;

			if( player->charact.status == c_p_ENEMY || player->charact.status == c_p_BOSS )
				player->w.ammo[1] = 99;
			else
				if(player->w.ammo[1]<0) player->w.ammo[1] = 0;
			if(player->charact.status == c_p_ENEMY || player->charact.status == c_p_BOSS )
				player->w.ammo[2] = 99;
			else
				if(player->w.ammo[2] < 0) player->w.ammo[2] = 0;
			player->Ibase = IMG_connect("TANK4");
			break;
	};
	if(player->charact.status == c_p_BOSS)
	{
		player->charact.healthmax = 5000;                                      //здоровье max
		player->charact.armormax  = 5000;                                      //броня max
	}
}

/*
 * вычисление повреждения наносимого игроку
 */
void player_getdamage(player_t * player, explode_t * explode, bool self, float radius)
{
	float damage_full;
	float armor;

	if(
			(player->charact.health>0) && (radius<=wtable[explode->_weap_].radius)
	)
	{
		if(self)
			damage_full = wtable[explode->_weap_].selfdamage*(1-radius/wtable[explode->_weap_].radius);
		else
			damage_full = wtable[explode->_weap_].damage    *(1-radius/wtable[explode->_weap_].radius);
		armor = player->charact.armor-damage_full*2/3;
		player->charact.health = player->charact.health-roundf(damage_full/3);
		if(armor < 0)
		{
			player->charact.health = player->charact.health+roundf(armor);
			player->charact.armor = 0;
		}
		else player->charact.armor = roundf(armor);
		if(player->charact.health <= 0)
		{
			if(!self)
			{
				//атакующему добавим очки
				explode->player->charact.scores += c_score_pertank;
				explode->player->charact.fragstotal++;
				explode->player->charact.frags++;
			}
			else
			{
				//атакующий умер от своей пули
				explode->player->charact.scores = 0;
				explode->player->charact.fragstotal--;
				explode->player->charact.frags--;
			}
			player_class_init(explode->player);
		}
	}
}

