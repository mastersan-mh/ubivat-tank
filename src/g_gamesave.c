/*
 * g_record.c
 *
 *  Created on: 20 янв. 2017 г.
 *      Author: mastersan
 */


#include "common_list2.h"

#include "types.h"
#include "server.h"
#include "client.h"
#include "ent_player.h"

#include <fcntl.h>
#include <g_gamesave.h>
#include <unistd.h>

extern host_client_t * hclients;
extern server_state_t sv_state;

/* список записей */
gamesave_descr_t gamesaves[G_GAMESAVES_NUM];


static char * make_filename(char * filename, int i)
{
	sprintf(filename, "/ut_s%02d.sav", i);
	return filename;
}


/**
 * @description чтение заголовка записи
 * @return true | false
 */
static int g_gamesave_cacheinfo(const char * savename, gamesave_descr_t * rec)
{
	int fd;
	char * path = Z_malloc(strlen(game_dir_saves) + strlen(savename) + 1);
	strcpy(path, game_dir_saves);
	strcat(path, savename);
	fd = open(path, O_RDONLY);
	Z_free(path);
	if(fd < 0)
		return -1;
	memset(rec, 0, sizeof(*rec));
	gamesave_data_header_t header;
	ssize_t count = read(fd, &header, sizeof(header));
	if(count != sizeof(header))
	{
		close(fd);
		return -1;
	}
	strncpy(rec->name, header.name, 15);
	rec->flags = header.flags;
	close(fd);
	rec->exist = true;
	return 0;
};


/**
 * запись игрока
 * @return = 0 | -1
 */
static int g_gamesave_save_player(int fd, entity_t * player)
{
	write(fd, map_class_names[MAPDATA_MOBJ_SPAWN_PLAYER], strlen(map_class_names[MAPDATA_MOBJ_SPAWN_PLAYER])+1);
	player_t * pl = player->data;
	gamesave_data_player_t savedata =
	{
		.fragstotal = pl->fragstotal,
		.frags      = pl->frags,
		.scores     = pl->items[ITEM_SCORES],
		.health     = pl->items[ITEM_HEALTH],
		.armor      = pl->items[ITEM_ARMOR],
		.ammo1      = pl->items[ITEM_AMMO_ARTILLERY],
		.ammo2      = pl->items[ITEM_AMMO_MISSILE],
		.ammo3      = pl->items[ITEM_AMMO_MINE]
	};
	write(fd, &savedata, sizeof(savedata));
	return 0;
};

/**
 * чтение игрока
 * @return true | false
 */
static int g_gamesave_load_player(int fd, server_clientsaveent_t * clientsaveent)
{
	mapdata_entity_type_t mapdata_mobj_type = map_file_class_get(fd);
	if(mapdata_mobj_type != MAPDATA_MOBJ_SPAWN_PLAYER)
		return -1;
	gamesave_data_player_t savedata;
	ssize_t c = read(fd, &savedata, sizeof(savedata));
	if(c != sizeof(savedata))return false;
	clientsaveent->storedata.fragstotal = savedata.fragstotal;
	clientsaveent->storedata.frags      = savedata.frags;
	clientsaveent->storedata.scores     = savedata.scores;
	clientsaveent->userstoredata.item_SCORES = savedata.scores;
	clientsaveent->userstoredata.item_HEALTH = savedata.health;
	clientsaveent->userstoredata.item_ARMOR  = savedata.armor;
	clientsaveent->userstoredata.item_AMMO_ARTILLERY = savedata.ammo1;
	clientsaveent->userstoredata.item_AMMO_MISSILE   = savedata.ammo2;
	clientsaveent->userstoredata.item_AMMO_MINE      = savedata.ammo3;
	clientsaveent->valid = true;
	return 0;
};

/*
 * формируем листинг записей
 */
void g_gamesave_cacheinfos(void)
{
	int i;
	char filename[16];

	memset(gamesaves, 0, sizeof(gamesaves));
	for(i = 0; i < G_GAMESAVES_NUM; i++)
	{
		g_gamesave_cacheinfo(make_filename(filename, i), &(gamesaves[i]));
	}
}

/**
 * сохраниние записи
 * @return true| false
 */
int g_gamesave_save(int isave)
{
	gamesave_descr_t * gamesave = &gamesaves[isave];
	//strncpy(gamesave->name, name, G_GAMESAVE_NAME_SIZE);
	int fd;
	char filename[16];
	make_filename(filename, isave);

	check_directory(game_dir_saves);

	char * path;
	path = Z_malloc(strlen(game_dir_saves) + strlen(filename) + 1);
	strcpy(path, game_dir_saves);
	strcat(path, filename);
	ssize_t count;
	//int ret = unlink(path);
	fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	Z_free(path);
	if(fd <= 0)
		return -1;
	gamesave_data_header_t header;
	strncpy(header.name, gamesave->name, G_GAMESAVE_NAME_SIZE);
	strncpy(header.mapfilename, map._file, MAP_FILENAME_SIZE);
	gamesave->flags = sv_state.flags;
	header.flags = gamesave->flags;
	count = write(fd, &header, sizeof(header));
	if(count != sizeof(header))
	{
		close(fd);
		return false;
	}

	host_client_t * client;
	LIST2_FOREACHR(hclients, client)
	{
		g_gamesave_save_player(fd, client->entity);
	}

	close(fd);
	return 0;
};


void g_gamesave_load_close(gamesave_load_context_t * ctx)
{
	if(ctx->fd >= 0)
	{
		close(ctx->fd);
	}
}

/*
 * чтение сохранённой игры
 * @return = 0 - успешно
 * @return = 1 - запись отсутствует
 * @return = 2 - карта отсутствует в списке карт
 * @return = 3 - ошибка чтения карты
 */
int g_gamesave_load_open(int isave, gamesave_load_context_t * ctx, server_clientsaveent_t server_clientsaveent[])
{
	ctx->fd = -1;
	gamesave_descr_t * gamesave = &gamesaves[isave];

	//закроем открытую карту
	map_clear();

	if(!gamesave->exist)
		return -1;
/*
	if(!(rec->flags & GAMEFLAG_CUSTOMGAME))
	{
		game.gamemap = map_find(rec->mapfilename);
		if(!game.gamemap) return 2;
	};
*/
	char filename[16];
	make_filename(filename, isave);
	char * path = Z_malloc(strlen(game_dir_saves) + strlen(filename) + 1);
	strcpy(path, game_dir_saves);
	strcat(path, filename);
	ctx->fd = open(path, O_RDONLY);
	Z_free(path);
	if(ctx->fd <= 0)
	{
		game_halt("gamesave load error");
		return -1;
	}

	gamesave_data_header_t header;
	ssize_t count = read(ctx->fd, &header, sizeof(header));
	if(count != sizeof(header))
	{
		g_gamesave_load_close(ctx);
		return -1;
	}

	strncpy(ctx->mapfilename, header.mapfilename, G_GAMESAVE_MAPFILENAME_SIZE);
	ctx->flags = header.flags;


	int player_num = (ctx->flags & GAMEFLAG_2PLAYERS) ? 2 : 1;

	int i;
	for(i = 0; i < player_num; i++)
	{
		g_gamesave_load_player(ctx->fd, &server_clientsaveent[i]);
	}
	return 0;

}