/*
 * map.h
 *
 *  Created on: 9 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_MAP_H_
#define SRC_MAP_H_

/*
 * координаты
 */
typedef struct
{
	int x;
	int y;
}TMAPorig;

typedef struct maplist_s
{
	struct maplist_s * prev;
	struct maplist_s * next;
	char * map;
	char * name;
} maplist_t;

/*
 * настройки
 */
typedef struct
{
	//класс
	char class;
	//позиция
	TMAPorig orig;
	//очки(-1 не используется)
	long scores;
	//здоровье у танка
	int health;
	//броня у танка
	int armor;
}TDATAspawn;
/*
 * точка респавнинга
 */
typedef struct spawn_s
{
	struct spawn_s * next;
	//параметры
	TDATAspawn data;
}spawn_t;
/********************************************************************/
/*
 * предметы
 */
typedef struct
{
	// предмет
	int class;
	// координаты
	TMAPorig orig;
	// количество
	int amount;
} TDATAitem;
typedef struct item_s
{
	struct item_s * next;
	// парамеры
	TDATAitem data;
	// флаг присутствия
	bool present;
	// изображение предмета
	item_img_t * img;
}item_t;
/********************************************************************/
typedef struct
{
	//предмет
	char class;
	//координаты
	TMAPorig orig;
	//сообщение
	TstrZ64 message;
} obj_data_t;
typedef struct Tobj
{
	struct Tobj *next;
	//настройки
	obj_data_t data;
	//изображение объекта
	item_img_t * img;
} obj_t;
/********************************************************************/
/*
 * карта
 */
typedef struct
{
	//имя файла карты
	TstrZ8 _file;
	//название карты
	TstrZ16 name;
	//краткое описание
	TstrZ128 brief;
	//точки респавнинга
	spawn_t * HEADspawn;
	//предметы
	item_t * HEADitem;
	//обьекты
	obj_t * HEADobj;
	//66 X 50 = 3300 матрица карты
	char map[c_MAP_sy][c_MAP_sx];
} map_t;

extern map_t map;

extern maplist_t * mapList;

void map_init();

int map_spawn_checkspawnpoints();

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

void map_clip_find_near(pos_t * orig, float box, int dir, char mask, float DISTmax, float * dist);
void map_clip_find_near_wall(pos_t * orig, int dir, float * dist, char * wall);

char * map_file_class_get(int fd, char * class);
int map_load(const char *mapname);
void map_close();
void map_draw(camera_t * cam);
void map_list_add(const char * map,const char * name);
void map_list_removeall();
void map_load_list();


#endif /* SRC_MAP_H_ */
