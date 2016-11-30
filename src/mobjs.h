/*
 * mobjs.h
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_MOBJS_H_
#define SRC_MOBJS_H_

#include "types.h"
#include "img.h"

typedef enum
{
	MOBJ_SPAWN,
	MOBJ_ITEM,
	MOBJ_MESSAGE,
	MOBJ_BULL,
	MOBJ_EXPLODE,
	MOBJ_EXIT,
	__MOBJ_NUM
} mobj_type_t;

typedef enum
{
	EXPLODE_ARTILLERY,
	EXPLODE_MISSILE,
	EXPLODE_MINE
} mobj_explode_type_t;

typedef struct
{
	coord_t x;
	coord_t y;
}mobj_position_t;

typedef enum
{
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT
} direction_t;

/*
 * точка респавнинга
 */
typedef struct
{
	enum
	{
		SPAWN_PLAYER,
		SPAWN_ENEMY,
		SPAWN_BOSS
	} type;
	//очки(-1 не используется)
	long scores;
	//здоровье у танка
	int health;
	//броня у танка
	int armor;
} mobj_spawn_t;

/*
 * предметы
 */
typedef struct
{
	enum
	{
		ITEM_HEALTH,
		ITEM_ARMOR ,
		ITEM_STAR  ,
		ITEM_ROCKET,
		ITEM_MINE
	} type;
	// количество
	int amount;
	// флаг присутствия
	bool exist;
} mobj_item_t;

typedef struct
{
	//сообщение
	char * message;
} mobj_message_t;

/*
 * пуля
 */
typedef struct
{
	//игрок, выпустивший пулю
	struct player_s * owner;
	//тип пули(оружие, из которого выпущена пуля)
	int _weap_;
	//изменение расстояния
	coord_t delta_s;
	//время
	float frame;
} mobj_bull_t;

/*
 * взрыв
 */
typedef struct
{
	//игрок, выпустивший пулю
	mobj_explode_type_t type;
	struct player_s * owner;
	int state;
	float frame;
} mobj_explode_t;

typedef struct
{
	//сообщение
	char * message;
} mobj_exit_t;

typedef struct mobj_s
{
	struct mobj_s *next;
	// удалить объект
	bool erase;
	//класс
	mobj_type_t type;
	//позиция
	mobj_position_t pos;
	/* направление взгляда/движения */
	direction_t dir;
	//изображение объекта
	item_img_t * img;
	union
	{
		mobj_spawn_t   spawn;
		mobj_item_t    item;
		mobj_message_t mesage;
		mobj_bull_t    bull;
		mobj_explode_t explode;
		mobj_exit_t    exit;
	};


} mobj_t;


mobj_explode_type_t bull_type_to_explode_type(int bull_type);

extern void mobjs_handle();

extern void mobjs_draw(camera_t * cam);

mobj_t * mobj_new(mobj_type_t mobj_type, coord_t x, coord_t y, direction_t dir);

void mobjs_erase_all();

#endif /* SRC_MOBJS_H_ */
