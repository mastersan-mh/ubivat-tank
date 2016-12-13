/*
 * mobjs.h
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_MOBJS_H_
#define SRC_MOBJS_H_

#include "vec.h"
#include "types.h"
#include "client.h"
#include "Z_mem.h"
#include "model.h"
#include "img.h"


#define MOBJ_FUNCTION_INIT(x) \
	void (x)(mobj_t * this, void * thisdata, const mobj_t * parent, const void * args)

#define MOBJ_FUNCTION_DONE(x) \
	void (x)(mobj_t * this, void * thisdata)

#define MOBJ_FUNCTION_DONE_DEFAULT NULL
#define MOBJ_FUNCTION_HANDLE_DEFAULT NULL

#define MOBJ_ERASE(mobj) (mobj)->erase = true

typedef enum
{
	MOBJ_SPAWN_PLAYER,
	MOBJ_SPAWN_ENEMY,
	MOBJ_SPAWN_BOSS,
	MOBJ_ITEM_SCORES,
	MOBJ_ITEM_HEALTH,
	MOBJ_ITEM_ARMOR,
	MOBJ_ITEM_AMMO_MISSILE,
	MOBJ_ITEM_AMMO_MINE,
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

typedef void (*endframe_f)(struct mobj_s * this, char * actionname);

typedef struct
{
	char * name;
	unsigned int startframe;
	unsigned int endframe;
	endframe_f endframef;
} ent_modelaction_t;

typedef struct
{
	const model_t * model;
	const vec_t modelscale;
	const vec2_t translation;
	/* количество действий */
	const unsigned int actions_num;
	/* действия */
	const ent_modelaction_t * actions;
} entmodel_t;

/* структура для проигрывания кардов моделей, связанных с объектом */
typedef struct
{
//	/* воспроизводимое действие */
	const ent_modelaction_t * action;
	/* номер кадра */
	float frame;
} ent_modelplayer_t;

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
	/* объект показывать и обрабатывать */
	bool show;

	//изображение объекта
	item_img_t * img;

	const struct mobj_register_s * info;
	/* структура для проигрывания кардов моделей, связанных с объектом */
	ent_modelplayer_t * modelplayers;

	void * data;

} mobj_t;

typedef struct mobj_register_s
{
	char * name;
	size_t datasize;
	void (*mobjinit)(mobj_t * this, void * thisdata, const mobj_t * parent, const void * args);
	void (*mobjdone)(mobj_t * this, void * thisdata);

	void (*handle)(mobj_t * this);

	void * (*connect)(const mobj_t * this);

	void * (*client_store)(client_storedata_t * storedata, const void * data);
	void (*client_restore)(void * data, const client_storedata_t * storedata, const void * userstoredata);

	/* размер массива моделей */
	unsigned int entmodels_num;
	/* массив моделей, связанных с объектом*/
	entmodel_t * entmodels;

}mobj_reginfo_t;

void mobj_register(const mobj_reginfo_t * info);
const mobj_reginfo_t * mobj_reginfo_get(const char * name);


extern void mobjs_handle();

extern void mobjs_render(camera_t * cam);

mobj_t * mobj_new(mobj_type_t mobj_type, vec_t x, vec_t y, direction_t dir, const mobj_t * parent, const void * args);
void mobj_model_start_play(mobj_t * mobj, unsigned int imodel, char * actionname);

void mobjs_erase();

#endif /* SRC_MOBJS_H_ */
