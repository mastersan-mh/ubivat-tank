/*
 * mobjs.h
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_MOBJS_H_
#define SRC_MOBJS_H_

#include "types.h"
#include "client.h"
#include "img.h"
#include "items.h"

typedef enum
{
	MOBJ_SPAWN,
	MOBJ_ITEM,
	MOBJ_MESSAGE,
	MOBJ_PLAYER,
	MOBJ_ENEMY,
	MOBJ_BOSS,
	MOBJ_BULL_ARTILLERY,
	MOBJ_BULL_MISSILE,
	MOBJ_BULL_MINE,
	MOBJ_EXPLODE_ARTILLERY,
	MOBJ_EXPLODE_MISSILE,
	MOBJ_EXPLODE_MINE,
	MOBJ_EXIT,
	__MOBJ_NUM
} mobj_type_t;

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

	const struct mobj_register_s * info;
	union
	{
		void * data;
		mobj_spawn_t   spawn;
		mobj_item_t    item;
		mobj_message_t mesage;
		mobj_exit_t    exit;
	};


} mobj_t;


typedef void* (*mobjinit_t)(const mobj_t * mobj);



typedef struct mobj_register_s
{
	char * name;
	void * (*mobjinit)(mobj_t * this, mobj_t * parent);
	void (*mobjdone)(mobj_t * this);

	void (*handle)(mobj_t * this);

	void * (*connect)(const mobj_t * this);

	void (*client_store)(client_storedata_t * storedata, const void * data);
	void (*client_restore)(void * data, const client_storedata_t * storedata);

}mobj_reginfo_t;

void mobj_register(const mobj_reginfo_t * info);
const mobj_reginfo_t * mobj_reginfo_get(const char * name);


extern void mobjs_handle();

extern void mobjs_draw(camera_t * cam);

mobj_t * mobj_new(mobj_type_t mobj_type, vec_t x, vec_t y, direction_t dir, mobj_t * const parent);

void mobjs_erase_all();

#endif /* SRC_MOBJS_H_ */
