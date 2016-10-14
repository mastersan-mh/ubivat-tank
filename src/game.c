/**
 *        Ubivat Tank
 *        игра
 * by Master San
 */


#include <defs.h>
#include <game.h>
#include <map.h>
#include <weap.h>
#include <plr.h>
#include <_gr2D.h>
#include <_gr2Don.h>
#include <x10_str.h>
#include <x10_time.h>
#include <x10_kbrd.h>
#include <x10_str.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

//состояние игры
game_t game;

/********запись конфига********/
void game_cfg_save()
{
	int fd;
	char * config = Z_malloc(strlen(BASEDIR)+strlen(c_CFGname) + 1);
	strcpy(config, BASEDIR);
	strcat(config, c_CFGname);
	fd = open(config, O_CREAT | O_WRONLY);
	write(fd, game.controlP0, sizeof(game.controlP0));
	write(fd, game.controlP1, sizeof(game.controlP1));
	close(fd);
};
/********новый конфиг********/
void game_CFG_new()
{
	//игрок 0
	game.controlP0[0]  = KP0_UPARROW_1   ;//вперед
	game.controlP0[1]  = KP0_UPARROW_0   ;//
	game.controlP0[2]  = KP0_DOWNARROW_1 ;//назад
	game.controlP0[3]  = KP0_DOWNARROW_0 ;//
	game.controlP0[4]  = KP0_LEFTARROW_1 ;//влево
	game.controlP0[5]  = KP0_LEFTARROW_0 ;//
	game.controlP0[6]  = KP0_RIGHTARROW_1;//вправо
	game.controlP0[7]  = KP0_RIGHTARROW_0;//
	game.controlP0[8]  = KP0_SLASH1_1    ;//пульки
	game.controlP0[9]  = KP0_SLASH1_0    ;//
	game.controlP0[10] = KP0_POINT_1     ;//ракета
	game.controlP0[11] = KP0_POINT_0     ;//
	game.controlP0[12] = KP0_SEMIPOINT_1 ;//мина
	game.controlP0[13] = KP0_SEMIPOINT_0 ;//
	//игрок 1
	game.controlP1[0]  = KP0_R_1         ;
	game.controlP1[1]  = KP0_R_0         ;
	game.controlP1[2]  = KP0_F_1         ;
	game.controlP1[3]  = KP0_F_0         ;
	game.controlP1[4]  = KP0_D_1         ;
	game.controlP1[5]  = KP0_D_0         ;
	game.controlP1[6]  = KP0_G_1         ;
	game.controlP1[7]  = KP0_G_0         ;
	game.controlP1[8]  = KP0_W_1         ;
	game.controlP1[9]  = KP0_W_0         ;
	game.controlP1[10] = KP0_Q_1         ;
	game.controlP1[11] = KP0_Q_0         ;
	game.controlP1[12] = KP0_TAB_1       ;
	game.controlP1[13] = KP0_TAB_0       ;
	game_cfg_save();
}
/********чтение конфига********/
void game_cfg_load()
{
	int fd;
	char * path = Z_malloc(strlen(BASEDIR)+strlen(c_CFGname) + 1);
	strcpy(path, BASEDIR);
	strcat(path, c_CFGname);
	fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		game_CFG_new();
		return;
	}
	read(fd, game.controlP0, sizeof(game.controlP0));
	read(fd, game.controlP1, sizeof(game.controlP1));
	close(fd);
	Z_free(path);
}
/********чтение файла палитры********/
//game_PAL_get=0 -успешно
//game_PAL_get=1 -файл не найден
//game_PAL_get=2 -ошибка чтения файла
bool game_PAL_get()
{
	int fd;
	char * path = Z_malloc(strlen(BASEDIR)+strlen(c_PALname) + 1);
	strcpy(path, BASEDIR);
	strcat(path, c_PALname);
	fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		Z_free(path);
		return false;
	}
	if(read(fd, gr2D.PAL, 768) != 768)
	{
		Z_free(path);
		return false;
	}
	close(fd);
	Z_free(path);
	return true;
}
/********переустановка времени у всех объектов на карте********/
void game_time_reset()
{
	player_t * player = playerList;
	while(player)
	{
		player->time.delta      = 0;
		player->time.last_delta = 0;
		time_Sget();
		player->time.t0         = time.s*100+time.hs;
		player->time.t1         = player->time.t0;
		player = player->next;
	}
	bull_t * bull = bullList;
	while(bull)
	{
		bull->delta_s = 0;                                                   //изменение расстояния
		bull->time.delta      = 0;
		bull->time.last_delta = 0;
		time_Sget();
		bull->time.t0  = time.s*100+time.hs;                                 //системное время в сотых долях секунд
		bull->time.t1         = bull->time.t0;
		bull = bull->next;
	}
	explode_t * explode = explList;
	while(explode)
	{
		explode->time.delta      = 0;
		explode->time.last_delta = 0;
		time_Sget();
		explode->time.t0         = time.s*100+time.hs;
		explode->time.t1         = explode->time.t0;
		explode = explode->next;
	}
}
/**
 * @description чтение заголовка записи
 * @return true | false
 */
static bool game_record_load_info(const char * savename, gamesave_t * rec)
{
	int fd;
	char * path = Z_malloc(strlen(BASEDIR)+strlen(c_CFGname) + 1);
	strcpy(rec->_file, savename);
	strcpy(path, BASEDIR);
	strcat(path, SAVESDIR);
	strcat(path, savename);
	strcat(path, c_SAVext);
	fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		return false;
	}
	memset(rec, 0, sizeof(*rec));
	//имя файла карты
	read(fd, rec->Hmap , 9 );rec->Hmap [8]  = 0;
	//внутреннее имя записи
	read(fd, rec->Hname, 17);rec->Hname[16] = 0;
	//флаги состояния игры
	read(fd, &rec->flags, 1 );
	close(fd);
	return true;
};
/*
 * формируем листинг записей
 */
void game_record_getsaves()
{
	int i;
	char _file[9];

	memset(game.saveslist, 0, sizeof(game.saveslist[0])*GAME_SAVESNUM);
	for(i = 0; i < GAME_SAVESNUM; i++)
	{
		game.saveslist[i].Hname[0] = 0xFF;
		sprintf(_file, "UT_S%02x", i);
		game_record_load_info(_file, &(game.saveslist[i]));
	};
};

/**
 * запись игрока
 * @return = true | false
 */
static bool game_record_save_player(int fd, player_t * player)
{
	switch(player->charact.status)
	{
	case c_p_P0:
	case c_p_P1:
		write(fd, c_MAP_s_player, 13);
		break;
	default:
		return false;
	};
	write(fd, &player->charact.scores    , 4);
	write(fd, &player->charact.status    , 1);
	write(fd, &player->charact.health    , 2);
	write(fd, &player->charact.armor     , 2);
	write(fd, &player->charact.fragstotal, 4);
	write(fd, &player->charact.frags     , 4);
	write(fd, &player->w.ammo            , 6);
	return true;
};

/**
 * чтение игрока
 * @return true | false
 */
static bool game_record_load_player(int fd, player_t * player)
{
	char class_string[33];
	map_file_class_get(fd, class_string);
	if(strcmp(class_string,c_MAP_s_player) != 0)
	{
		return false;
	}
	read(fd, &player->charact.scores    , 4);
	read(fd, &player->charact.status    , 1);
	read(fd, &player->charact.health    , 2);
	read(fd, &player->charact.armor     , 2);
	read(fd, &player->charact.fragstotal, 4);
	read(fd, &player->charact.frags     , 4);
	read(fd, &player->w.ammo            , 6);
	player_class_init(player);
	return true;
};
/**
 * сохраниние записи
 * @return true| false
 */
bool game_record_save(gamesave_t * rec)
{
	int fd;
	char * path;

	//если папка "SAVES" отсутствует, тогда создадим ее
	path = Z_malloc(
		strlen(BASEDIR)+
		strlen(SAVESDIR)+
		1
		);

	strcpy(path, BASEDIR);
	strcat(path, SAVESDIR);
	DIR * dir = opendir(path);
	if(!dir) mkdir(path, 0755);
	else closedir(dir);

	Z_free(path);

	path = Z_malloc(
		strlen(BASEDIR)+
		strlen(SAVESDIR)+
		strlen(rec->_file)+
		strlen(c_SAVext)+
		1
		);
	strcpy(path, BASEDIR);
	strcat(path, SAVESDIR);
	strcat(path, rec->_file);
	strcat(path, c_SAVext);
	strcpy(rec->Hmap, map._file);

	fd = open(path, O_CREAT | O_WRONLY);
	//имя файла карты
	write(fd, rec->Hmap , 9);
	//внутреннее имя записи
	write(fd, rec->Hname,17);
	//флаги настройки игры
	write(fd, &rec->flags, 1);
	//сохраним 0-го игрока
	game_record_save_player(fd, game.P0);
	//сохраним 1-го игрока
	if(game.P1) game_record_save_player(fd, game.P1);
	close(fd);
	Z_free(path);
	return true;
};
/*
 * чтение сохранённой игры
 * @return = 0 - успешно
 * @return = 1 - запись отсутствует
 * @return = 2 - карта отсутствует в списке карт
 * @return = 3 - ошибка чтения карты
 */
int game_record_load(gamesave_t * rec)
{
	int fd_rec;

	if(rec->Hname[0] == 0xFF)
		return 1;
	if(!(rec->flags & c_g_f_CASE))
	{
		game.gamemap = mapList;
		while(game.gamemap)
		{
			if(strcmp(rec->Hmap, game.gamemap->map)==0)
				return 2;
			game.gamemap = game.gamemap->next;
		}
	};
	//закроем отктытую карту
	map_close();
	//прочитаем карту
	game.error = map_load(rec->Hmap);
	if(game.error != 0) return 3;
	char * path = Z_malloc(
		sizeof(BASEDIR)+
		sizeof(SAVESDIR)+
		sizeof(rec->_file)+
		sizeof(c_SAVext)+
		1
	);

	strcpy(path, BASEDIR);
	strcat(path, SAVESDIR);
	strcat(path, rec->_file);
	strcat(path, c_SAVext);
	fd_rec = open(path, O_RDONLY);
	//имя файла карты
	read(fd_rec, rec->Hmap , 9);rec->Hmap [ 8] = 0;
	//внутреннее имя записи
	read(fd_rec, rec->Hname,17);rec->Hname[16] = 0;
	//флаги настройки игры
	read(fd_rec, &rec->flags, 1);game.flags = rec->flags;
	//создаем игру и спавним всех игроков
	game.error = game_create();
	//читаем первого игрока
	game_record_load_player(fd_rec, game.P0);
	//читаем второго игрока
	if(rec->flags & c_g_f_2PLAYERS)
		game_record_load_player(fd_rec, game.P1);
	close(fd_rec);
	Z_free(path);
	return 0;
}
/*
 * создане игры
 *
 * @return = 0 - игра создана успешно
 * @return = 1 - игра уже создана
 * @return = 2 - ошибка создания серверного игрока
 * @return = 3 - ошибка создания серверного игрока
 */
int game_create()
{
	int ret;
	if(game.created) return 1;
	if((game.flags & c_g_f_2PLAYERS) == 0)
	{
		game.P0cam.orig.x = 0;
		game.P0cam.orig.y = 0;
		game.P0cam.x      = 0;
		game.P0cam.y      = 0;
		game.P0cam.sx     = 320;
		game.P0cam.sy     = 184;
		ret = player_connect(c_p_P0);
		if(ret)return ret;
	}
	else
	{
		game.P0cam.orig.x = 0;
		game.P0cam.orig.y = 0;
		game.P0cam.x      = 160+1;
		game.P0cam.y      = 0;
		game.P0cam.sx     = 160-1;
		game.P0cam.sy     = 184;
		game.P1cam.orig.x = 0;
		game.P1cam.orig.y = 0;
		game.P1cam.x      = 0;
		game.P1cam.y      = 0;
		game.P1cam.sx     = 160-1;
		game.P1cam.sy     = 184;
		ret = player_connect(c_p_P0);
		if(ret)return ret;
		ret = player_connect(c_p_P1);
		if(ret)return ret;
	};
	//спавним всех игроков и монстров
	player_spawn_all();
	game.created    = true;
	game.ingame     = true;
	game.allowpress = true;
	return 0;
}
/*
 * прерывание игры
 */
void game_abort()
{
	game.gamemap = mapList;
	//дисконнект всех игроков
	player_disconnect_all();
	explode_removeall();
	bull_removeall();
	//закроем карту
	map_close();
	game.created    = false;
	game.ingame     = false;
	game._win_      = false;
	game.menu       = c_m_main;
	game.allowpress = false;
}

/*
 * сообщения о фатальных ошибках
 */
void game_msg_error_fatal(int error)
{
#define ERR_NUM (4)
	static char *errList[ERR_NUM] =
	{
		"NO ERRORS.",
		"file not found.",
		"error file format.",
		"unknown."
	};
	if(error > ERR_NUM) error = ERR_NUM;
	printf("ERROR(%d): %s\n", error, errList[error]);
}

/*
 * сообщения об ошибках
 */
void game_msg_error(int error)
{
#define sx (256)
#define sy (32)
#define x (160 - (sx / 2))
#define y (100 - (sy / 2))
#define ERR_MAX 10
	static char *errList[ERR_MAX] = {
			"NO ERRORS.", // 0
			"Map not found.", // 1
			"Map load error.", // 2
			"Map load error.", // 3
			"Could not find spawn point.", // 4
			"Could not find spawn point.", // 5
			"Record not found.", // 11
			"Map have no found in maps list.", // 12
			"Map load error.", // 13
			"Unknown."
	};
	bool quit = false;

	while(!quit)
	{
		gr2D.color.current = 0;
		gr2D_BUFcurrentfill();
		gr2D.color.current = 26; gr2D_rectangle_f(x,y,sx,sy);
		gr2D.color.current = 30; gr2D_line_h(x     ,x+sx-1,y     );          //верхний борт
		gr2D.color.current = 22; gr2D_line_h(x     ,x+sx-1,y+sy-1);          //нижний  борт
		gr2D.color.current = 29; gr2D_line_v(x     ,y     ,y+sy-1);          //левый   борт
		gr2D.color.current = 23; gr2D_line_v(x+sx-1,y     ,y+sy-1);          //правый  борт
		gr2D.color.current =  4; gr2Don_settextZ(x+(sx / 2)-6*8,y+2,"ERROR: ");
		gr2D.color.current = 15;
		int e;
		if(error <= 5)e = error;
		else e = error - 5;
		if(e > ERR_MAX) e = ERR_MAX;

		gr2Don_settextZ(x+2, y+16, errList[e]);

		kbrd_readport();
		if(kbrd.port != 0)
		{
			if(game.allowpress && kbrd.port <= 127 )
			{
				do
				{
					kbrd_readport();
				} while(kbrd.port != KP0_ENTER_1);
				quit = true;
			};
			if( !game.allowpress && 127 < kbrd.port ) game.allowpress = true;
		}
	gr2D_BUFcurrent2screen();
	}
}

void game_message_send(const char * mess)
{
	game.mess = (char*)mess;
};



void game_halt(const char *error, ...)
{
	int __errno_ = errno;
	static char errmsg[MAX_MESSAGE_SIZE];
	va_list argptr;
	va_start(argptr, error);
#ifdef HAVE_VSNPRINTF
	vsnprintf(errmsg, MAX_MESSAGE_SIZE, error, argptr);
#else
	vsprintf(errmsg, error, argptr);
#endif
	va_end(argptr);

	if(__errno_)
	{
		fprintf(stdout, "%s: %s\n", errmsg, strerror(__errno_));
	}
	else
	{
		fprintf(stdout, "%s\n", errmsg);
	}
	exit(1);
}

