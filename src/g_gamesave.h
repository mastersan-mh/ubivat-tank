/*
 * sv_record.h
 *
 *  Created on: 19 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_G_GAMESAVE_H_
#define SRC_G_GAMESAVE_H_

#define G_GAMESAVE_NAME_SIZE (16)
#define G_GAMESAVE_MAPFILENAME_SIZE MAP_FILENAME_SIZE
#define G_GAMESAVES_NUM (8)

#include "types.h"
#include "map.h"
#include "server_private.h"

typedef struct
{
	char name[G_GAMESAVE_NAME_SIZE];
	char mapfilename[MAP_FILENAME_SIZE];
	uint16_t flag_localgame;
	uint16_t flag_allow_respawn;
	uint16_t players_num;

} ATTR_PACKED gamesave_data_header_t;

/* сохранение иры */
typedef struct
{
	bool exist;
	//внутреннее имя записи
	char name[G_GAMESAVE_NAME_SIZE];
	//флаги настройки игры
	server_gameflags_t flags;
	size_t players_num_total;
} gamesave_descr_t;

/* чтение сохранённой иры */
typedef struct
{
    int fd;
    char name[G_GAMESAVE_NAME_SIZE];
    /* имя файла карты */
    char mapfilename[G_GAMESAVE_MAPFILENAME_SIZE];
    //флаги настройки игры
    int flag_localgame;
    int flag_allow_respawn;
    size_t clients_num;
    size_t * clients_descr;  /* players_nums */
} gamesave_load_context_t;

extern gamesave_descr_t gamesaves[G_GAMESAVES_NUM];

extern int g_gamesave_save(int isave);

extern void g_gamesave_cacheinfos(void);

extern int g_gamesave_load_open(int isave, gamesave_load_context_t * ctx);
extern void g_gamesave_load_close(gamesave_load_context_t * ctx);
extern int g_gamesave_load_read_header(gamesave_load_context_t * ctx);
void g_gamesave_load_player(gamesave_load_context_t * ctx, server_player_vars_storage_t * storage);

//extern int g_gamesave_load(int isave);


#endif /* SRC_G_GAMESAVE_H_ */
