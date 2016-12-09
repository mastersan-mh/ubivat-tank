/*
 * map.h
 *
 *  Created on: 9 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_MAP_H_
#define SRC_MAP_H_

#include <types.h>
#include <img.h>
#include "mobjs.h"

//броня0
#define c_m_w_w0        0x01
//броня1
#define c_m_w_w1        0x02
//кирпич
#define c_m_w_brick     0x04
//вода
#define c_m_water       0x08
//флаг присутствия стены
#define c_m_f_clip      0x10

#define MAP_NAME_SIZE (17)
#define MAP_BRIEF_SIZE (129)

// размер карты OX
#define MAP_SX 66
// размер карты OY
#define MAP_SY 50
// размер блока карты
#define MAP_BLOCKSIZE (8)

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
	__MAPDATA_MOBJ_NUM
} mapdata_mobj_type_t;


extern char * map_class_names[__MAPDATA_MOBJ_NUM];

/* заголовок карты */
typedef struct
{
	char sign[3];
	char name[MAP_NAME_SIZE];
	char brief[MAP_BRIEF_SIZE];
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
} ATTR_PACKED map_data_mobj_t;

typedef struct maplist_s
{
	struct maplist_s * prev;
	struct maplist_s * next;
	char * map;
	char * name;
} maplist_t;


/*
 * карта
 */
typedef struct
{
	bool loaded;
	// имя файла карты
	char * _file;
	// название карты
	char * name;
	// краткое описание
	char * brief;
	// объекты карты
	struct mobj_s * mobjs;
	// 66 X 50 = 3300 матрица карты
	char map[MAP_SY][MAP_SX];
} map_t;

extern map_t map;

extern maplist_t * mapList;

int map_error_get();

void map_init();


void map_clip_find(
	vec2_t * orig,
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

void map_clip_find_near(vec2_t * orig, vec_t box, int dir, char mask, vec_t DISTmax, vec_t * dist);
void map_clip_find_near_wall(vec2_t * orig, int dir, vec_t * dist, char * wall);
bool map_mobj_is_item(mobj_t * mobj);

extern mapdata_mobj_type_t map_file_class_get(int fd);
int map_load(const char * mapname);
void map_clear();
void map_draw(camera_t * cam);
void map_list_add(const char * map,const char * name);
void map_list_removeall();
void map_load_list();


#endif /* SRC_MAP_H_ */
