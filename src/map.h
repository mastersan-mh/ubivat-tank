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

typedef enum
{
	MAP_SPAWN_PLAYER,
	MAP_SPAWN_ENEMY ,
	MAP_SPAWN_BOSS  ,
	MAP_ITEM_HEALTH ,
	MAP_ITEM_ARMOR  ,
	MAP_ITEM_STAR   ,
	MAP_ITEM_ROCKET ,
	MAP_ITEM_MINE   ,
	MAP_OBJ_EXIT    ,
	MAP_OBJ_MESS    ,
	MAP_UNKNOWN     ,
	__MAP_NUM
} mobj_type_t;

extern char * map_class_names[__MAP_NUM];

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
	map_data_position_t orig;
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
	map_data_position_t orig;
	// количество
	int16_t amount;
} ATTR_PACKED map_data_item_t;

#define MAB_OBJ_MESAGE_SIZE (65)
typedef struct
{
	//позиция
	map_data_position_t orig;
	//сообщение
	char message[MAB_OBJ_MESAGE_SIZE];
} ATTR_PACKED map_data_obj_t;



typedef struct
{
	coord_t x;
	coord_t y;
}map_position_t;

typedef struct maplist_s
{
	struct maplist_s * prev;
	struct maplist_s * next;
	char * map;
	char * name;
} maplist_t;

/*
 * точка респавнинга
 */
typedef struct spawn_s
{
	struct spawn_s * next;
	//класс
	mobj_type_t class;
	//позиция
	map_position_t orig;
	//очки(-1 не используется)
	long scores;
	//здоровье у танка
	int health;
	//броня у танка
	int armor;
}spawn_t;

/*
 * предметы
 */
typedef struct item_s
{
	struct item_s * next;
	//класс
	mobj_type_t class;
	// координаты
	map_position_t orig;
	// количество
	int amount;
	// флаг присутствия
	bool exist;
	// изображение предмета
	item_img_t * img;
}item_t;

typedef struct Tobj
{
	struct Tobj *next;
	//предмет
	mobj_type_t class;
	//координаты
	map_position_t orig;
	//сообщение
	char * message;
	//изображение объекта
	item_img_t * img;
} obj_t;
/********************************************************************/
/*
 * карта
 */
typedef struct
{
	bool loaded;
	//имя файла карты
	char * _file;
	//название карты
	char * name;
	//краткое описание
	char * brief;
	//точки респавнинга
	spawn_t * spawns;
	//предметы
	item_t * items;
	//обьекты
	obj_t * objs;
	//66 X 50 = 3300 матрица карты
	char map[MAP_SY][MAP_SX];
} map_t;

extern map_t map;

extern maplist_t * mapList;

int map_error_get();

void map_init();

void map_clip_find(
		pos_t * orig,
		float BOX,
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

void map_clip_find_near(pos_t * orig, coord_t box, int dir, char mask, coord_t DISTmax, coord_t * dist);
void map_clip_find_near_wall(pos_t * orig, int dir, coord_t * dist, char * wall);

extern mobj_type_t map_file_class_get(int fd);
int map_load(const char * mapname);
void map_clear();
void map_draw(camera_t * cam);
void map_list_add(const char * map,const char * name);
void map_list_removeall();
void map_load_list();


#endif /* SRC_MAP_H_ */
