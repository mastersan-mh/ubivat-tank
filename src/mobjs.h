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
#include "weap.h"
#include "items.h"
//#include "player.h"

typedef enum
{
	MOBJ_SPAWN,
	MOBJ_ITEM,
	MOBJ_MESSAGE,
	MOBJ_PLAYER,
	MOBJ_ENEMY,
	MOBJ_BULL,
	MOBJ_EXPLODE,
	MOBJ_EXIT,
	__MOBJ_NUM
} mobj_type_t;


typedef enum
{
	BULL_ARTILLERY,
	BULL_MISSILE,
	BULL_MINE,
	__BULL_NUM
} mobj_bulltype_t;

typedef enum
{
	EXPLODE_ARTILLERY,
	EXPLODE_MISSILE,
	EXPLODE_MINE,
	__EXPLODE_NUM
} mobj_explodetype_t;

typedef enum direction_e
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
	int items[__ITEM_NUM];
} mobj_spawn_t;

/*
 * предметы
 */
typedef struct
{
	itemtype_t type;
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
	struct mobj_s * owner;
	//тип пули
	mobj_bulltype_t type;
	//изменение расстояния
	vec_t delta_s;
	//время
	float frame;
} mobj_bull_t;

/*
 * взрыв
 */
typedef struct
{
	//игрок, выпустивший пулю
	mobj_explodetype_t type;
	struct mobj_s * owner;
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
	struct mobj_s * next;
	// удалить объект
	bool erase;
	//класс
	mobj_type_t type;
	//позиция
	vec2_t pos;
	/* направление взгляда/движения */
	direction_t dir;
	//изображение объекта
	item_img_t * img;

	struct mobj_register_s * info;
	union
	{
		mobj_spawn_t   spawn;
		mobj_item_t    item;
		mobj_message_t mesage;
		struct mobj_player_s * player;
		mobj_bull_t    bull;
		mobj_explode_t explode;
		mobj_exit_t    exit;
	};


} mobj_t;


typedef void* (*mobjinit_t)(const mobj_t * mobj);

typedef struct mobj_register_s
{
	char * name;
	void * (*mobjinit)(const mobj_t * mobj);

	void * (*connect)(const mobj_t * mobj);
	void * (*client_store)(const mobj_t * mobj);
	void * (*client_restore)(const mobj_t * mobj);

}mobj_register_t;

void mobjinfo_register(const mobj_register_t * info);
const mobj_register_t * mobjinjfo_get(const char * name);


mobj_bulltype_t mobj_weapon_type_to_bull_type(weapontype_t type);
mobj_explodetype_t mobj_bull_type_to_explode_type(mobj_bulltype_t bull_type);

extern void mobjs_handle();

extern void mobjs_draw(camera_t * cam);

mobj_t * mobj_new(mobj_type_t mobj_type, vec_t x, vec_t y, direction_t dir);

void mobjs_erase_all();

#endif /* SRC_MOBJS_H_ */
