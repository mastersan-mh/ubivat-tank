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
#include "server.h"

typedef struct
{
	char name[G_GAMESAVE_NAME_SIZE];
	char mapfilename[MAP_FILENAME_SIZE];
	uint16_t flags;

} ATTR_PACKED gamesave_data_header_t;

typedef struct
{
	uint32_t fragstotal;
	uint32_t frags;
	uint32_t scores;
	int16_t health;
	int16_t armor;
	int16_t ammo1;
	int16_t ammo2;
	int16_t ammo3;
} ATTR_PACKED gamesave_data_player_t;

/* сохранение иры */
typedef struct
{
	bool exist;
	//внутреннее имя записи
	char name[G_GAMESAVE_NAME_SIZE];
	//флаги настройки игры
	uint16_t flags;
} gamesave_descr_t;

/* чтение сохранённой иры */
typedef struct
{
	int fd;
	/* имя файла карты */
	char mapfilename[G_GAMESAVE_MAPFILENAME_SIZE];
	//флаги настройки игры
	uint16_t flags;
} gamesave_load_context_t;

extern gamesave_descr_t gamesaves[G_GAMESAVES_NUM];

extern void g_gamesave_cacheinfos(void);
extern int g_gamesave_save(int isave);

extern void g_gamesave_load_close(gamesave_load_context_t * ctx);
extern void g_gamesave_load_read(gamesave_load_context_t * ctx);

extern int g_gamesave_load_open(int isave, gamesave_load_context_t * ctx);

//extern int g_gamesave_load(int isave);


#endif /* SRC_G_GAMESAVE_H_ */
