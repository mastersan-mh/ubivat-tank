/*
 * map.h
 *
 *  Created on: 9 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_MAP_H_
#define SRC_MAP_H_

#include "types.h"

#define MAP_FILENAME_SIZE (256)

//флаг присутствия стены
#define MAP_WALL_CLIP      0x10

/* присутствует зажим */
#define MAP_WALL_CLIPPED(wall) ((wall) & MAP_WALL_CLIP)

#define MAP_WALL_TEXTURE(wall) ((wall) & 0x0F)

//броня0
#define MAP_WALL_W0        0x01
//броня1
#define MAP_WALL_W1        0x02
//кирпич
#define MAP_WALL_brick     0x04
//вода
#define MAP_WALL_water     0x08

#define MAP_NAME_SIZE (17)
#define MAP_BRIEF_SIZE (129)

// размер карты OX
#define MAP_SX 66
// размер карты OY
#define MAP_SY 50
// размер блока стены карты
#define MAP_WALLBLOCKSIZE (8)

typedef enum
{
	MAPDATA_MOBJ_UNKNOWN     = -1,
	MAPDATA_MOBJ_SPAWN_PLAYER,
	MAPDATA_MOBJ_SPAWN_ENEMY ,
	MAPDATA_MOBJ_SPAWN_BOSS  ,
	MAPDATA_MOBJ_ITEM_HEALTH ,
	MAPDATA_MOBJ_ITEM_ARMOR  ,
	MAPDATA_MOBJ_ITEM_STAR   ,
	MAPDATA_MOBJ_ITEM_ROCKET ,
	MAPDATA_MOBJ_ITEM_MINE   ,
	MAPDATA_MOBJ_OBJ_EXIT    ,
	MAPDATA_MOBJ_OBJ_MESS    ,
	MAPDATA_MOBJ_NUM
} mapdata_entity_type_t;


extern char * map_class_names[MAPDATA_MOBJ_NUM];

/* заголовок карты */
typedef struct
{
	char sign[3];
	char mapname[MAP_NAME_SIZE];
	char mapbrief[MAP_BRIEF_SIZE];
} ATTR_PACKED map_data_header_t;

/* координаты */
typedef struct
{
	uint16_t x;
	uint16_t y;
} ATTR_PACKED map_data_position_t;

/* объекты */
typedef struct
{
	//позиция
	map_data_position_t pos;
	//очки(-1 не используется)
	int32_t scores;
	//здоровье у танка
	int16_t health;
	//броня у танка
	int16_t armor;
} ATTR_PACKED map_data_spawn_t;

typedef struct
{
	//позиция
	map_data_position_t pos;
	// количество
	int16_t amount;
} ATTR_PACKED map_data_item_t;

#define MAB_OBJ_MESAGE_SIZE (65)
typedef struct
{
	//позиция
	map_data_position_t pos;
	//сообщение
	char message[MAB_OBJ_MESAGE_SIZE];
} ATTR_PACKED map_data_obj_t;


typedef union
{
	map_data_position_t pos;
	map_data_spawn_t spawn;
	map_data_item_t item;
	map_data_obj_t obj;
} ATTR_PACKED map_data_entity_t;

typedef struct maplist_s
{
	struct maplist_s * prev;
	struct maplist_s * next;
	char * filename;
	char * mapname;
} maplist_t;


/*
 * карта
 */
typedef struct
{
	// имя файла карты
	char * filename;
	// название карты
	char * mapname;
	// краткое описание
	char * mapbrief;
	// 66 X 50 = 3300 матрица карты
	char map[MAP_SY][MAP_SX];
} map_t;

extern maplist_t * mapList;

extern int map_error_get(void);

extern maplist_t * map_find(const char * name);

extern void map_init(void);


extern void map_clip_find(
    map_t * map,
	const vec2_t orig,
	vec_t BOX,
	char mask,
	bool * Ul,
	bool * Ur,
	bool * Dl,
	bool * Dr,
	bool * Lu,
	bool * Ld,
	bool * Ru,
	bool * Rd
);

extern void map_clip_find_near(
    map_t * map,
    const vec2_t origin, vec_t box, direction_t dir, char mask, vec_t DISTmax, vec_t * dist);
extern void map_clip_find_near_wall(
    map_t * map,
    const vec2_t origin, direction_t dir, vec_t * dist, char * wall);

extern mapdata_entity_type_t map_file_class_get(int fd);
extern map_t * map_load(const char * mapname);
extern void map_free(map_t * map);
extern void map_render(const map_t * map, camera_t * cam);

extern void maplist_add(const char * filename,const char * mapname);
extern void maplist_free(void);
extern void maplist_load(void);


#endif /* SRC_MAP_H_ */
