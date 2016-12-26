/*
 * entity.h
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENTITY_H_
#define SRC_ENTITY_H_

#include "vec.h"
#include "types.h"
#include "client.h"
#include "Z_mem.h"
#include "model.h"
#include "img.h"

/*
 * цикл по объектам одного определённого типа
 */
#define FOR_ENTITIES(entity_name, entity) \
	for(entity = entity_getfirst(entity_name); entity; entity = entity->next)

#define ENTITY_FUNCTION_INIT(x) \
	void x (entity_t * this, void * thisdata, const entity_t * parent, const void * args)

#define ENTITY_FUNCTION_DONE(x) \
	void x (entity_t * this, void * thisdata)

#define ENTITY_FUNCTION_CLIENT_SPAWN(x) \
	entity_t * x (const entity_t * this)


#define ENTITY_FUNCTION_DONE_DEFAULT NULL
#define ENTITY_FUNCTION_HANDLE_DEFAULT NULL


#define ENTITY_ERASE(ent) (ent)->erase = true

#define ENTITY_ALLOW_HANDLE_SET(ent, bool_value) \
		(ent)->allow_handle = (bool_value)

#define ENTITY_ALLOW_DRAW_SET(ent, bool_value) \
		(ent)->allow_draw = (bool_value)

typedef enum direction_e
{
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT
} direction_t;

typedef void (*endframe_f)(struct entlink_s * this, unsigned int imodel, char * actionname);

typedef struct
{
	char * name;
	unsigned int startframe;
	unsigned int endframe;
	endframe_f endframef;
} ent_modelaction_t;

typedef struct
{
	/* имя модели */
	char * modelname;
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
	const model_t * model;
//	/* воспроизводимое действие */
	const ent_modelaction_t * action;
	/* номер кадра */
	float frame;
} ent_modelplayer_t;

/*
 * список объектов одного типа, у которых
 * .erased == false и allow_handle == true
 */
typedef struct entlink_s
{
	struct entlink_s * prev;
	struct entlink_s * next;
	/* родитель объекта. Если родитель уничтожатеся, родителем становится "дедушка" */
	struct entlink_s * parent;
	/* удалить объект */
	bool erase;
	//позиция
	vec2_t pos;
	/* направление взгляда/движения */
	direction_t dir;
	/* объект показывать и обрабатывать */
	bool allow_handle;
	/* объеккт разрешено показывать */
	bool allow_draw;

	const struct entityinfo_s * info;
	/* структура для проигрывания кардов моделей, связанных с объектом */
	ent_modelplayer_t * modelplayers;

	void * data;
} entity_t;

typedef struct entityinfo_s
{
	char * name;
	size_t datasize;
	void (*init)(entity_t * this, void * thisdata, const entity_t * parent, const void * args);
	void (*done)(entity_t * this, void * thisdata);

	void (*handle)(entity_t * this);

	entity_t * (*client_spawn)(const entity_t * this);

	void * (*client_store)(client_storedata_t * storedata, const void * data);
	void (*client_restore)(void * data, const client_storedata_t * storedata, const void * userstoredata);

	/* размер массива моделей */
	unsigned int entmodels_num;
	/* массив моделей, связанных с объектом*/
	entmodel_t * entmodels;

}entityinfo_t;

extern void entity_register(const entityinfo_t * info);

extern entity_t * entity_getfirst(const char * name);

extern void entities_handle();
extern entity_t * entries_client_spawn();

extern void entities_render(camera_t * cam);

extern entity_t * entity_new(const char * name, vec_t x, vec_t y, direction_t dir, const entity_t * parent, const void * args);
extern void entities_erase();

extern void entity_model_play_start(entity_t * entity, unsigned int imodel, char * actionname);
extern void entity_model_play_pause(entity_t * entity, unsigned int imodel);
extern void entity_model_play_pause_all(entity_t * entity);

extern int entity_model_set(entity_t * entity, unsigned int imodel, char * modelname);

#endif /* SRC_ENTITY_H_ */
