/*
 *        Ubivat Tank
 *        обработка игрока
 *
 * by Master San
 */

#include <defs.h>
#include <img.h>
#include <map.h>
#include <weap.h>
#include <plr.h>
#include <game.h>
#include <ctrl.h>
#include <_gr2D.h>
#include <_gr2Don.h>
#include <x10_time.h>
#include <x10_str.h>
#include <x10_kbrd.h>

#include <string.h>

player_t * playerList = NULL;

static char codebuf[17];
static int codecount = 0;
static long codetimer = 0;

int player_spawn_player(player_t * player);

//////////////////////////////////////////////////////////////////////
//набор и проверка кодов (добавление 10.05.2006)
//////////////////////////////////////////////////////////////////////
void player_checkcode()
{
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
		item = map.HEADitem;
		while(item)
		{
			if(item->present)
			{                                    //если предмет есть
				if(
						(item->data.orig.x-c_i_MDL_box/2< player->move.orig.x+c_p_MDL_box/2)&&
						(player->move.orig.x-c_p_MDL_box/2<=item->data.orig.x+c_i_MDL_box/2)&&
						(item->data.orig.y-c_i_MDL_box/2<=player->move.orig.y+c_p_MDL_box/2)&&
						(player->move.orig.y-c_p_MDL_box/2< item->data.orig.y+c_i_MDL_box/2)
				)
				{
					switch(item->data.class)
					{
					case c_i_health:
						if((player->charact.health<player->charact.healthmax)||(item->data.amount<0))
						{
							item->present = false;
							if(player->charact.healthmax < player->charact.health + item->data.amount)
								player->charact.health = player->charact.healthmax;
							else
								player->charact.health += item->data.amount;
						};
						break;
					case c_i_armor:
						if((player->charact.armor<player->charact.armormax)||(item->data.amount<0)) {
							item->present = false;
							if(player->charact.armormax<player->charact.armor+item->data.amount)
								player->charact.armor = player->charact.armormax;
							else
								player->charact.armor+= item->data.amount;
							if(player->charact.armor<0) player->charact.armor = 0;
						};
						break;
					case c_i_star  :
						if((player->charact.scores<c_score_max)||(item->data.amount<0))
						{
							item->present = false;
							if(c_score_max<player->charact.scores+item->data.amount)
								player->charact.scores = c_score_max;
							else
								player->charact.scores+= item->data.amount;
							if(player->charact.scores<0) player->charact.scores = 0;
						};
						player_class_init(player);
						if(5<=player->charact.scores / c_score_perclass)
						{
							if(player->charact.health<player->charact.healthmax) player->charact.health = player->charact.healthmax;
							if(player->charact.armor <player->charact.armormax ) player->charact.armor  = player->charact.armormax ;
						};
						break;
					case c_i_rocket:
						if((player->w.ammo[1] != c_p_WEAP_notused)&&((player->w.ammo[1]<wtable[1].ammo)||(item->data.amount<0)))
						{
							item->present = false;
							if(wtable[1].ammo<player->w.ammo[1]+item->data.amount)
								player->w.ammo[1] = wtable[1].ammo;
							else
								player->w.ammo[1]+= item->data.amount;
							if(player->w.ammo[1]<0) player->w.ammo[1] = 0;
						};
						break;
					case c_i_mine  :
						if((player->w.ammo[2] != c_p_WEAP_notused)&&((player->w.ammo[2]<wtable[2].ammo)||(item->data.amount<0)))
						{
							item->present = false;
							if(wtable[2].ammo<player->w.ammo[2]+item->data.amount)
								player->w.ammo[2] = wtable[2].ammo;
							else
								player->w.ammo[2]+= item->data.amount;
							if(player->w.ammo[2]<0) player->w.ammo[2] = 0;
						};
						break;
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
	if((player->charact.status=c_p_P0)||(player->charact.status=c_p_P1))
	{
		obj = map.HEADobj;
		while(obj)
		{
			if(
					(obj->data.orig.x-c_o_MDL_box/2 <= player->move.orig.x + c_p_MDL_box / 2)&&
					(player->move.orig.x-c_p_MDL_box/2 <= obj->data.orig.x + c_o_MDL_box / 2)&&
					(obj->data.orig.y-c_o_MDL_box/2 <= player->move.orig.y + c_p_MDL_box / 2)&&
					(player->move.orig.y-c_p_MDL_box/2 <= obj->data.orig.y + c_o_MDL_box / 2)
			)
			{
				switch (obj->data.class)
				{
				case c_o_exit: game._win_ = true;break;
				case c_o_mess: break;
				}
				//отправим сообщение игроку
				game_message_send(obj->data.message);
			};
		obj = obj->next;
		};
	};
};
/*
 * рисование игрока
 */
void player_draw(camera_t * cam, player_t * player, bool play)
{
	if(player->charact.health>0)
	{ //если игрок жив
		if(player->move.go)
		{
			if(play) {
				player->Fbase = player->Fbase+c_p_fpsRUN*player->time.delta/100;
				if((player->Fbase<0)||(player->Fbase>3)) player->Fbase = 0;
			};
		};
		if(
				(cam->orig.x-cam->sx/2<=player->move.orig.x+(c_p_MDL_box/2))&&(player->move.orig.x-(c_p_MDL_box/2)<=cam->orig.x+cam->sx/2)&&
				(cam->orig.y-cam->sy/2<=player->move.orig.y+(c_p_MDL_box/2))&&(player->move.orig.y-(c_p_MDL_box/2)<=cam->orig.y+cam->sy/2)
		)
		{
			gr2D.WIN.x0 = cam->x;gr2D.WIN.x1 = cam->x+cam->sx-1;
			gr2D.WIN.y0 = cam->y;gr2D.WIN.y1 = cam->y+cam->sy-1;
			gr2D_setimage1(
					round(cam->x+player->move.orig.x-(cam->orig.x-cam->sx/2))+c_p_MDL_pos,
					round(cam->y-player->move.orig.y+(cam->orig.y+cam->sy/2))+c_p_MDL_pos,
					player->Ibase->IMG.sx,
					player->Ibase->IMG.sy,
					0,
					c_p_MDL_box*((player->move.dir * 4)+round(player->Fbase)),
					c_p_MDL_box,
					c_p_MDL_box,
					player->Ibase->IMG.pic
			);//база
			gr2D_setimage0(
					round(cam->x+player->move.orig.x-(cam->orig.x-cam->sx/2))+c_p_MDL_pos,
					round(cam->y-player->move.orig.y+(cam->orig.y+cam->sy/2))+c_p_MDL_pos,
					player->Iflag->IMG.sx,
					player->Iflag->IMG.sy,
					player->Iflag->IMG.pic
			);//флаг
			gr2D.WIN.x0 = 0;
			gr2D.WIN.x1 = gr2D_SCR_sx-1;
			gr2D.WIN.y0 = 0;
			gr2D.WIN.y1 = gr2D_SCR_sy-1;
		};
	};
};
/*
 * передвижение игрока
 */
static void player_move(player_t * player, int dir, long * speed)
{
	pos_t orig;
	int c = 0;
	float dist;

	orig = player->move.orig;
	switch(dir)
	{
	case c_DIR_up:
		do{
			orig.y = player->move.orig.y+(*speed)*player->time.delta/1000;
			map_clip_find_near(&orig,c_p_MDL_box,c_DIR_up,0xF0,c_p_MDL_box/2+2, &dist);//найдем препятствия
			if(dist<=c_p_MDL_box/2) (*speed) = (*speed) >> 2;
			c+= 1;
		}while(!( (c_p_MDL_box/2<dist)||(c=5) ));
		break;
	case c_DIR_dn:
		do{
			orig.y = player->move.orig.y-(*speed)*player->time.delta/1000;
			map_clip_find_near(&orig,c_p_MDL_box,c_DIR_dn,0xF0,c_p_MDL_box/2+2, &dist);//найдем препятствия
			if(dist<=c_p_MDL_box/2) (*speed) = (*speed) >> 2;
			c+= 1;
		}while(!( (c_p_MDL_box/2<dist)||(c=5) ));
		break;
	case c_DIR_lf:
		do{
			orig.x = player->move.orig.x-(*speed)*player->time.delta/1000;
			map_clip_find_near(&orig,c_p_MDL_box,c_DIR_lf,0xF0,c_p_MDL_box/2+2, &dist);//найдем препятствия
			if(dist<=c_p_MDL_box/2) (*speed) = (*speed) >> 2;
			c+= 1;
		}while(!( (c_p_MDL_box/2<dist)||(c=5) ));
		break;
	case c_DIR_rt:
		do{
			orig.x = player->move.orig.x+(*speed)*player->time.delta/1000;
			map_clip_find_near(&orig,c_p_MDL_box,c_DIR_rt,0xF0,c_p_MDL_box/2+2, &dist);//найдем препятствия
			if(dist<=c_p_MDL_box/2) (*speed) = (*speed) >> 2;
			c+= 1;
		}while(!( (c_p_MDL_box/2<dist)||(c=5) ));
		break;
	}
	player->move.orig = orig;
};
/*
 * управление игроком
 */
void player_control(player_t * player)
{
	pos_t Sorig;
	float L,R,U,D;
	long speed_s;

	time_Sget();
	player->time.t1 = time.s*100+time.hs;              //системное время в сотых долях секунд
	if(player->time.t0>player->time.t1)
	{
		player->time.delta = player->time.last_delta;       //t0 должно быть меньше t1
	}
	else
	{
		player->time.delta = player->time.t1-player->time.t0;
	};
	player->time.last_delta = player->time.delta;
	time_Sget();
	player->time.t0 = time.s*100+time.hs;               //системное время в сотых долях секунд
	if(player->charact.health<=0) {                     //если игрок мертв
		if(player->charact.spawned)
		{
			bull_add();
			bullList->player = player;
			bullList->_weap_ = 1;
			explode_add(bullList,player->move.orig.x,player->move.orig.y);
			bull_remove(&bullList);
			player->charact.spawned = false;
			player->charact.armor = 0;
			if(0<player->w.ammo[1]) player->w.ammo[1] = 0;
			if(0<player->w.ammo[2]) player->w.ammo[2] = 0;
		};
		if(player->charact.status == c_p_BOSS) game._win_ = true;
	}
	else
	{                                                           //если игрок жив
		if(player->bull)
		{
			player->bull->dir = player->move.dir;
			player->move.go = false;
		};
		if(!player->move.go)
		{                                     //игрок останавливается
			player->move.speed = player->move.speed-c_p_accel*player->time.delta;
			if(player->move.speed<0) player->move.speed = 0;
		}
		else {                                                          //игрок идет
			player->move.speed = player->move.speed+c_p_accel*player->time.delta;
			if(player->charact.speed<player->move.speed) player->move.speed = player->charact.speed;
			if(player->move.speed<0) player->move.speed = 0;
		};
		player_move(player,player->move.dir, &player->move.speed);
		speed_s = player->charact.speed >> 2;
		//стрейф
		switch(player->move.dir){
		case c_DIR_up:
		case c_DIR_dn:
			Sorig = player->move.orig;
			Sorig.x = Sorig.x-c_p_MDL_box/4;
			map_clip_find_near(&Sorig,c_p_MDL_box/2,player->move.dir,0xF0,c_p_MDL_box/2+2, &L);
			Sorig = player->move.orig;
			Sorig.x = Sorig.x+c_p_MDL_box/4;
			map_clip_find_near(&Sorig,c_p_MDL_box/2,player->move.dir,0xF0,c_p_MDL_box/2+2, &R);
			if((c_p_MDL_box/2<L) && (R-1<=c_p_MDL_box/2)) player_move(player,c_DIR_lf, &speed_s);//strafe left
			if((c_p_MDL_box/2<R) && (L-1<=c_p_MDL_box/2)) player_move(player,c_DIR_rt, &speed_s);//strafe right
			break;
		case c_DIR_lf:
		case c_DIR_rt:
			Sorig = player->move.orig;
			Sorig.y = Sorig.y-c_p_MDL_box/4;
			map_clip_find_near(&Sorig,c_p_MDL_box/2,player->move.dir,0xF0,c_p_MDL_box/2+2, &D);
			Sorig = player->move.orig;
			Sorig.y = Sorig.y+c_p_MDL_box/4;
			map_clip_find_near(&Sorig,c_p_MDL_box/2,player->move.dir,0xF0,c_p_MDL_box/2+2, &U);
			if((c_p_MDL_box/2<U)&&(D-1<=c_p_MDL_box/2)) player_move(player,c_DIR_up, &speed_s);//strafe up
			if((c_p_MDL_box/2<D)&&(U-1<=c_p_MDL_box/2)) player_move(player,c_DIR_dn, &speed_s);//strafe down
			break;
		}
	}
//стрельба
	if(player->w.attack == 0)
	{                                       //игрок не атакует
		if(player->w.reloadtime_d>0) player->w.reloadtime_d -= player->time.delta;//учитываем время на перезарядку
	}
	else {                                                           //игрок атакует
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
			if(player->w.reloadtime_d>0) player->w.reloadtime_d-= player->time.delta;//учитываем время на перезарядку
			else
			{
				if(!player->bull)
				{                                      //если не стреляем управляемой ракетой
					if(
							(wtable[player->w.attack-1].ammo == c_WEAP_indefinit)||            //если пуль бесконечно много
							(player->w.ammo[player->w.attack-1]>0)
					)
					{                   // или пули не кончились
						player->w.reloadtime_d = c_p_WEAP_reloadtime;
						bull_add();                                                       //создаем пулю
						bullList->orig.x   = player->move.orig.x;                     //координаты
						bullList->orig.y   = player->move.orig.y;                     //координаты
						bullList->player   = player;                                  //игрок, выпустивший пулю
						bullList->_weap_   = player->w.attack-1;                      //тип пули(оружие, из которого выпущена пуля)
						if(bullList->_weap_==1) player->bull = bullList;
						bullList->dir    = player->move.dir;                          //направление движения
						bullList->delta_s  = 0;                                     //изменение расстояния
						bullList->time.t0  = time.s*100+time.hs;                    //системное время в сотых долях секунд
						bullList->frame    = 0;
						switch(player->w.attack)
						{                                          //присоединяем изображение пули
						case 1:bullList->image = IMG_connect(game.HEADimg, "B_BULL"  );break;
						case 2:bullList->image = IMG_connect(game.HEADimg, "B_ROCKET");break;
						case 3:bullList->image = IMG_connect(game.HEADimg, "B_MINE"  );break;
						};
						if(
								(wtable[player->w.attack-1].ammo>0) && //если пули у оружия не бесконечны и
								((player->charact.status=c_p_P0)||(player->charact.status=c_p_P1)) // игрок не монстр(у монстрюков пули не кончаются)
						)
							player->w.ammo[player->w.attack-1]-= 1;
					}
				}
			}
		}
	}
	if(player->w.reloadtime_d<0) player->w.reloadtime_d = 0;
	player_item_get (player);                                              //подбираем предметы
	player_obj_check(player);                                              //проверяем объекты
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
	bool flag;
	int id = 0;
	if(
			(status != c_p_ENEMY)&&(status != c_p_BOSS)
	) player = player_find(status);
	if(player) error = 2;                                           //ирок P0/P1 присоединен
	else
	{                                                           //ирок P0/P1 не присоединен
		if(playerList)
		{                                  //список игроков создан
			error = 1;
			flag  = false;
			while((id<c_p_Pmax)&&(error=1))
			{                             //пытаемся соединить игрока с сервером,
				player = playerList;                                            // пока не исчерпан лимит на кол-во игроков
				while((!flag)&&(player))
				{                             //ищем, чтобы небыло совпадения идентификаторов
					if(player->charact.id==id) flag = true;                            //совпадение есть
					else player = player->next;                                           //совпадения нет, проверим следующий эл-т
				};
				if(!flag) error = 0;                                         //все номера свободны, коннект удался
				else {
					flag = false;
					id = id+1;
				}
			}
		}
	}
	if(!error)
	{                                                //нет ошибок, инициализируем игрока
		player = Z_malloc(sizeof(*player));                       //создаем игрока
		player->next      = playerList;              //включим игрока в
		playerList = player           ;             // общий список
		player->charact.scores = 0;
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
	if((!error)||(error==2))
	{
		if(player->charact.status==c_p_BOSS) player->Iflag = IMG_connect(game.HEADimg,"F_USA");//флаг
		else
			if(player->charact.status==c_p_ENEMY) player->Iflag = IMG_connect(game.HEADimg,"F_WHITE");//флаг
			else {
				if(player->charact.status==c_p_P0) game.P0 = player;
				if(player->charact.status==c_p_P1) game.P1 = player;
				player->charact.frags = 0;
				player->Iflag = IMG_connect(game.HEADimg,"F_RUS");//флаг
			};
		player->charact.spawned   = false;
	};
	return error;
};
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
void player_spawn(player_t * player, TDATAspawn * DATA)
{
	if((-1<DATA->scores)&&(DATA->scores<=c_score_max)) player->charact.scores = DATA->scores;
	player_class_init(player);
	if((-1<DATA->health)&&(DATA->health<=player->charact.healthmax)) player->charact.health = DATA->health;
	else
	{
		if(
				(player->charact.status==c_p_BOSS)||
				(player->charact.status==c_p_ENEMY)
		) player->charact.health = player->charact.healthmax;
		else
		{
			if((game.flags & c_g_f_CASE)==c_g_f_CASE) player->charact.health = player->charact.healthmax;//по выбору
			else {                                                         //по уровням
				if(game.gamemap == mapList) player->charact.health = player->charact.healthmax;//первая карта
				else {                                                        //не первая карта
					if((!player->charact.spawned)&&(player->charact.health<=0)) player->charact.health = player->charact.healthmax;
				};
			}
		};
	};
	if((-1<DATA->armor )&&(DATA->armor <=player->charact.armormax )) player->charact.armor = DATA->armor;
	else
	{
		if(
				(player->charact.status=c_p_BOSS)||
				(player->charact.status=c_p_ENEMY)
		) player->charact.armor = player->charact.armormax;
		else
		{
			if((game.flags & c_g_f_CASE) == c_g_f_CASE) player->charact.armor = player->charact.armormax;
		};
	};
	player->charact.spawned   = true;
	player->bull              = NULL;
	player->move.speed        = 0;
	player->move.go           = false;
	player->move.dir          = 0;
	player->w.attack          = false;
	player->w.reloadtime_d    = 0;
	player->time.delta      = 0;
	player->time.last_delta = 0;
	time_Sget();
	player->time.t0         = time.s*100+time.hs;
	player->time.t1         = player->time.t0;
};
/*
 * появление/восстановление игрока на карте
 * @return = 0 -спавнинг прошел успешно
 * @return = 1 -игрок является монстром, ошибка спавнинга
 */
int player_spawn_player(player_t * player)
{
	spawn_t * spawn;
	int count;
	int spawn_amount;
	int spawn_number;
	bool flag = false;

	if(
			(player->charact.status != c_p_P0)&&(player->charact.status != c_p_P1)
	) return 1;//игрок является монстром, ошибка спавнинга
	else
	{
		spawn_amount = 0;
		spawn = map.HEADspawn;
		while(spawn){ spawn = spawn->next;spawn_amount++;};     //считаем количество спавн-поинтов
		do
		{                                                              //ищем подходящий спавн поинт
			spawn_number = trunc(xrand(spawn_amount));//=[0..spawn_amount-1]    //выбираем случайным образом
			spawn = map.HEADspawn;
			for(count = 0; count < spawn_number; count ++) spawn = spawn->next;
			if(spawn->data.class==c_s_PLAYER) flag = true;                      //это PLAYER спавн
		}while(!flag);
		player->move.orig.x = spawn->data.orig.x;
		player->move.orig.y = spawn->data.orig.y;
		player_spawn(player, &spawn->data);
		return 0;
	}
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

	spawn = map.HEADspawn;
	while(spawn && error != 1)
	{                                //пока не привысили лимит на игроков
		if(
				(spawn->data.class==c_s_ENEMY)
				||(spawn->data.class==c_s_BOSS )
		)
		{                            //это monster спавн
			if(spawn->data.class==c_s_ENEMY) error = player_connect(c_p_ENEMY);
			if(spawn->data.class==c_s_BOSS ) error = player_connect(c_p_BOSS );
			if(error != 1)
			{
				playerList->move.orig.x = spawn->data.orig.x;
				playerList->move.orig.y = spawn->data.orig.y;
				player_spawn(playerList, &spawn->data);
			};
		};
		spawn = spawn->next;
	};
	return error;
};
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



/*
 * вывод информации об игроке
 */
void player_draw_status(camera_t * cam, player_t *player)
{
	char s[32];
	gr2D.color.current = 1;
	// gr2D_settext(cam->x,cam->y,0,'('+realtostr(player->move.orig.x,8,4)+';'+realtostr(player->move.orig.y,8,4)+')');
	// gr2D_settext(cam->x,cam->y,0,'PING '+wordtostr(player->time.delta));
	gr2Don_settext(cam->x+32*2+16, cam->y+cam->sy+4, 0,inttostr(player->w.ammo[0], s));
	if(player->w.ammo[1]>-1)
		gr2Don_settext(cam->x+32*3+16,cam->y+cam->sy+4,0,inttostr(player->w.ammo[1], s));
	if(player->w.ammo[2]>-1)
		gr2Don_settext(cam->x+32*4+16,cam->y+cam->sy+4,0,inttostr(player->w.ammo[2], s));
	gr2Don_settext(cam->x+32*0+16,cam->y+cam->sy-16,0,inttostr(player->charact.health, s));
	gr2Don_settext(cam->x+32*0+16,cam->y+cam->sy-8,0,inttostr(player->charact.healthmax, s));
	gr2Don_settext(cam->x+32*3+16,cam->y+cam->sy-16,0,inttostr(player->charact.armor, s));
	gr2Don_settext(cam->x+32*3+16,cam->y+cam->sy-8,0,inttostr(player->charact.armormax, s));
	gr2Don_settext(cam->x+32*0+16,cam->y+cam->sy+4,0,inttostr(player->charact.scores, s));
	gr2D_setimage0(
		cam->x + 32 * 0,
		cam->y + cam->sy - 16,
		game.i_health->IMG.sx, game.i_health->IMG.sy, game.i_health->IMG.pic
	);
	gr2D_setimage0(cam->x+32*3,cam->y+cam->sy-16,
		game.i_armor->IMG.sx,game.i_armor->IMG.sy,game.i_armor->IMG.pic
	);
	gr2D_setimage1(
		cam->x+32*0,cam->y+cam->sy,
		player->Ibase->IMG.sx,
		player->Ibase->IMG.sy,
		0,0,c_p_MDL_box,c_p_MDL_box,
		player->Ibase->IMG.pic
	);
	gr2D_setimage0(cam->x+32*2,cam->y+cam->sy,
		wtable[0].icon->IMG.sx,
		wtable[0].icon->IMG.sy,
		wtable[0].icon->IMG.pic
	);
	gr2D_setimage0(cam->x+32*3,cam->y+cam->sy,
		wtable[1].icon->IMG.sx,wtable[1].icon->IMG.sy,wtable[1].icon->IMG.pic
	);
	gr2D_setimage0(cam->x+32*4,cam->y+cam->sy,
		wtable[2].icon->IMG.sx,wtable[2].icon->IMG.sy,wtable[2].icon->IMG.pic
	);
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
			player->charact.speed = 300;                                          //максимальная скорость при ходьбе
			player->w.ammo[0] = 99;
			player->w.ammo[1] = c_p_WEAP_notused;
			player->w.ammo[2] = c_p_WEAP_notused;
			player->Ibase = IMG_connect(game.HEADimg,"TANK0");                    //база
			break;
		case 1:
			player->charact.healthmax = 100;                                      //здоровье max
			player->charact.armormax  = 050;                                      //броня max
			player->charact.speed = 375;                                          //максимальная скорость при ходьбе
			player->w.ammo[0] = 99;
			player->w.ammo[1] = c_p_WEAP_notused;
			player->w.ammo[2] = c_p_WEAP_notused;
			player->Ibase = IMG_connect(game.HEADimg,"TANK1");                    //база
			break;
		case 2:
			player->charact.healthmax = 100;                                      //здоровье max
			player->charact.armormax  = 100;                                      //броня max
			player->charact.speed = 450;                                          //максимальная скорость при ходьбе
			player->w.ammo[0] = 99;
			if((player->charact.status==c_p_ENEMY)||(player->charact.status==c_p_BOSS ))
				player->w.ammo[1] = 99;
			else if(player->w.ammo[1]<0) player->w.ammo[1] = 0;
			player->w.ammo[2] = c_p_WEAP_notused;
			player->Ibase = IMG_connect(game.HEADimg,"TANK2");                    //база
			break;
		case 3:
			player->charact.healthmax = 200;                                      //здоровье max
			player->charact.armormax  = 150;                                      //броня max
			player->charact.speed = 525;                                          //максимальная скорость при ходьбе
			player->w.ammo[0] = 99;
			if((player->charact.status==c_p_ENEMY)||(player->charact.status==c_p_BOSS ))
				player->w.ammo[1] = 99;
			else
				if(player->w.ammo[1]<0) player->w.ammo[1] = 0;
			player->w.ammo[2] = c_p_WEAP_notused;
			player->Ibase = IMG_connect(game.HEADimg,"TANK3");                    //база
			break;
		case 4:
			player->charact.healthmax = 200;                                      //здоровье max
			player->charact.armormax  = 200;                                      //броня max
			player->charact.speed = 600;                                          //максимальная скорость при ходьбе
			player->w.ammo[0] = 99;
			if((player->charact.status==c_p_ENEMY)||(player->charact.status==c_p_BOSS ))
				player->w.ammo[1] = 99;
			else
				if(player->w.ammo[1]<0) player->w.ammo[1] = 0;
			if((player->charact.status==c_p_ENEMY)||(player->charact.status==c_p_BOSS ))
				player->w.ammo[2] = 99;
			else
				if(player->w.ammo[2]<0) player->w.ammo[2] = 0;
			player->Ibase = IMG_connect(game.HEADimg,"TANK4");                    //база
			break;
	};
	if(player->charact.status==c_p_BOSS) {
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
		if(armor<0)
		{
			player->charact.health = player->charact.health+roundf(armor);
			player->charact.armor = 0;
		}
		else player->charact.armor = roundf(armor);
		if(player->charact.health<=0) {
			if(!self)
			{
				explode->player->charact.scores     += c_score_pertank;               //атакующему добавим очки
				explode->player->charact.fragstotal ++;
				explode->player->charact.frags      ++;
			}
			else
			{
				//атакующий умер от своей пули
				explode->player->charact.scores     = 0;
				explode->player->charact.fragstotal --;
				explode->player->charact.frags      --;
			};
			player_class_init(explode->player);
		}
	}
}

