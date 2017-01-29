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
#include "Z_mem.h"
#include "model.h"
#include "img.h"
#include "server.h"
#include "common_bstree.h"

#define ENTITY_NAME_SIZE (64)
#define ENTITY_VARNAME_SIZE (64)

/*
 * цикл по объектам одного определённого типа
 */
#define ENTITIES_FOREACH(entity_name, entity) \
	for(entity = entity_getfirst(entity_name); entity; entity = entity->next)

#define ENTITY_FUNCTION_INIT(x) \
	void x (entity_t * this, void * thisdata, const entity_t * parent, const void * args)

#define ENTITY_FUNCTION_DONE(x) \
	void x (entity_t * this, void * thisdata)

#define ENTITY_FUNCTION_SPAWN(x) \
	void x (entity_t * this, void * thisdata)

#define ENTITY_FUNCTION_HANDLE(x) \
	void x (entity_t * this, void * thisdata)

#define ENTITY_FUNCTION_CLIENT_SPAWN(x) \
	entity_t * x (const entity_t * this)

#define ENTITY_FUNCTION_NONE NULL

#define ENTITY_FUNCTION_ACTION(x) \
	void x (entity_t * this, void * thisdata, const char * action)


#define ENTITY_ERASE(ent) (ent)->erase = true

/* entity является объектом типа entity_name */
#define ENTITY_IS(entity, entity_name) \
		( strncmp( (entity)->info->name, (entity_name), ENTITY_NAME_SIZE ) == 0 )

#define ENTITY_ALLOW_HANDLE_SET(ent, bool_value) \
		(ent)->allow_handle = (bool_value)

#define ENTITY_ALLOW_DRAW_SET(ent, bool_value) \
		(ent)->allow_draw = (bool_value)

#define ENTITY_IS_SPAWNED(ent) \
		((ent)->spawned == true)

#define ENTITY_UNSPAWN(ent) \
		(ent)->spawned = false

#define ENTITYINFO_VARS(xvars) \
		.vars_num = ARRAYSIZE(xvars), \
		.vars = xvars

#define ENTITYINFO_ACTIONS(xactions) \
		.actions_num = ARRAYSIZE(xactions), \
		.actions = xactions

#define ENTITYINFO_ENTMODELS(xentmodels) \
		.entmodels_num = ARRAYSIZE(xentmodels), \
		.entmodels = xentmodels

/* переменная entity */
typedef struct entityvardata_s
{
	size_t index; /* номер переменной в списке entityvardata_t.vars */
	entityvartype_t type;
	entityvarvalue_t value;
} entityvardata_t;

/*
 * список объектов одного типа, у которых
 * .erased == false и allow_handle == true
 */
typedef struct entity_s
{
	struct entity_s * prev;
	struct entity_s * next;
	/* родитель объекта. Если родитель уничтожатеся, родителем становится "дедушка" */
	struct entity_s * parent;
	/* переменные */
	node_t * vars; /* entityvardata_t */
	/* удалить объект */
	bool erase;
	//позиция
	vec2_t pos;
	/* объект живой */
	bool alive;
	/* направление взгляда/движения */
	direction_t dir;
	/* объект показывать и обрабатывать */
	bool allow_handle;
	/* объеккт разрешено показывать */
	bool allow_draw;
	/*
	объект находится на карте, с ним можно взаимодействовать, от отрисовывается.
	Ели функция info->spawn() не задана, объект всегда spawned.
	При этом, если насильно сделать объект не spawned, он никогда не появится,
	потому-что info->handle() никогда не вызовтся.
	Ели функция info->spawn() задана, тогда изначально spawned = false
	 */
	bool spawned;

	/* статистика */
	/* пройденое расстояние */
	vec_t stat_traveled_distance;

	const struct entityinfo_s * info;
	/* структура для проигрывания кардов моделей, связанных с объектом */
	struct ent_modelplayer_s * modelplayers;

	void * data;
} entity_t;

typedef struct
{
	char * name;
	unsigned int startframe;
	unsigned int endframe;
	void (*endframef)(entity_t * this, unsigned int imodel, char * actionname);
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

/* структура для проигрывания кадров моделей, связанных с объектом */
typedef struct ent_modelplayer_s
{
	const model_t * model;
//	/* воспроизводимое действие */
	const ent_modelaction_t * action;
	/* номер кадра */
	float frame;
} ent_modelplayer_t;

/* описатель дополнительной переменной entity, используются в handler, передаются клиенту */
typedef struct
{
	/* имя переменной */
	char * name;
	/* тип переменной */
	entityvartype_t type;
} entityvarinfo_t;

typedef struct entityaction_s
{
	char * action;
	void (*action_f)(struct entity_s * this, void * thisdata, const char * action);
} entityaction_t;

typedef struct entityinfo_s
{
	char * name;
	size_t datasize;

	/* массив дополнительных переменных */
	unsigned int vars_num;
	entityvarinfo_t * vars;

	void (*init)(entity_t * this, void * thisdata, const entity_t * parent, const void * args);
	void (*done)(entity_t * this, void * thisdata);

	void (*spawn)(entity_t * this, void * thisdata);

	void (*handle)(entity_t * this, void * thisdata);

	entity_t * (*client_join)(const entity_t * this);

	void * (*client_store)(const void * thisdata);
	void (*client_restore)(entity_t * this, void * thisdata, const void * userstoredata);

	/* массив действий, допустимых для entity */
	unsigned int actions_num;
	entityaction_t * actions;

	/* размер массива моделей */
	unsigned int entmodels_num;
	/* массив моделей, связанных с объектом*/
	entmodel_t * entmodels;

}entityinfo_t;

extern void entity_register(const entityinfo_t * info);

extern entity_t * entity_getfirst(const char * name);

extern void entities_handle(void);
extern entity_t * entries_client_join(void);

extern void entities_render(camera_t * cam);

extern entity_t * entity_new(const char * name, vec_t x, vec_t y, direction_t dir, const entity_t * parent, const void * args);
extern void entities_erase(void);

extern entityvardata_t * entity_vardata_get(const entity_t * entity, const char * varname, entityvartype_t vartype);

#define ENTITY_VARIABLE_INTEGER(entity, varname) \
	entity_vardata_get((entity), (varname), ENTITYVARTYPE_INTEGER)->value.i64
#define ENTITY_VARIABLE_FLOAT(entity, varname) \
	entity_vardata_get((entity), (varname), ENTITYVARTYPE_FLOAT)->value.f
#define ENTITY_VARIABLE_STRING(entity, varname) \
	entity_vardata_get((entity), (varname), ENTITYVARTYPE_STRING)->value.string
#define ENTITY_VARIABLE_STRING_DUP(text) \
	Z_strdup((text))
#define ENTITY_VARIABLE_STRING_ERASE(text) \
	do { Z_free((text)); (text) = NULL; } while(0)

extern void entity_model_play_start(entity_t * entity, unsigned int imodel, char * actionname);
extern void entity_model_play_pause(entity_t * entity, unsigned int imodel);
extern void entity_model_play_pause_all(entity_t * entity);

extern int entity_model_set(entity_t * entity, unsigned int imodel, char * modelname);

#endif /* SRC_ENTITY_H_ */
