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
#include "common/common_bstree.h"
#include "vars.h"

#define ENTITY_NAME_SIZE (64)

#define origin_x origin[0]
#define origin_y origin[1]

#define ENTITY_HALFBODYBOX(entity) ((entity)->info->bodybox * 0.5)

/*
 * цикл по объектам одного определённого типа
 */
#define ENTITIES_FOREACH(entity_name, entity) \
        for(entity = entity_getfirst(entity_name); entity; entity = entity->next)

#define ENTITY_FUNCTION_TOUCH(x) \
        void x (struct entity_s * this, struct entity_s * that)

#define ENTITY_FUNCTION_INIT(x) \
        void x (entity_t * this, void * thisdata, const entity_t * parent)

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

#define ENTITYINFO_TOUCHS(xtouchs) \
        .entitytouchs_num = ARRAYSIZE(xtouchs), \
        .entitytouchs = xtouchs

#define ENTITYINFO_ACTIONS(xactions) \
        .actions_num = ARRAYSIZE(xactions), \
        .actions = xactions

#define ENTITYINFO_ENTMODELS(xentmodels) \
        .entmodels_num = ARRAYSIZE(xentmodels), \
        .entmodels = xentmodels

enum
{
    ENTITYFLAG_SOLIDWALL   = 0x01
    //	ENTITYFLAG_SOLIDENTITY = 0x02
};

/*
 * список объектов одного типа, у которых
 * .erased == false и allow_handle == true
 */
typedef struct entity_s
{
    struct entity_s * prev;
    struct entity_s * next;

    /* объект, за которым следит камера данного объекта, обычно равен this */
    struct entity_s * cam_entity;

    /* родитель объекта. Если родитель уничтожатеся, родителем становится "дедушка" */
    struct entity_s * parent;
    /* переменные */
    var_t * vars; /* vardata_t */
    /* удалить объект */
    bool erase;
    //позиция
    vec2_t origin_prev;
    vec2_t origin;
    /* направление взгляда/движения */
    direction_t dir;
    /* объект показывать и обрабатывать */
    bool allow_handle;
    /* объект живой. влияет на победу игроков и на взаимодействие (touch). */
    bool alive;
    /* объект разрешено показывать. влияет только на рендер. */
    bool allow_draw;
    /*
	Объект находится на карте, с ним можно взаимодействовать, он отрисовывается.
	Ели функция info->spawn() не задана, объект всегда spawned.
	При этом, если принудительно сделать объект не spawned, он никогда не появится,
	потому-что info->handle() никогда не вызовется.
	Ели функция info->spawn() задана, тогда изначально spawned = false
     */
    bool spawned;

    /* статистика */
    /* пройденое расстояние */
    vec_t stat_traveled_distance;

    const struct entityinfo_s * info;
    /* структура для проигрывания кардов моделей, связанных с объектом */
    struct ent_modelplayer_s * modelplayers;

    /* extended data */
    void * edata;
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
} entitymodel_t;

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
    /* тип переменной */
    vartype_t type;
    /* имя переменной */
    char * name;
} entityvarinfo_t;

typedef struct
{
    const char * entityname;
    void (*touch)(struct entity_s * this, struct entity_s * that);
} entitytouch_t;


typedef struct entityaction_s
{
    char * action;
    void (*action_f)(struct entity_s * this, void * thisdata, const char * action);
} entityaction_t;

typedef struct entityinfo_s
{
    char * name;
    size_t edatasize;

    /* entity flags */
    int flags;
    /* для простоты все объекты квадратные */
    vec_t bodybox;

    /* массив дополнительных переменных */
    size_t vars_num;
    entityvarinfo_t * vars;

    /* размер массива моделей */
    size_t entmodels_num;
    /* массив моделей, связанных с объектом*/
    entitymodel_t * entmodels;

    void (*init)(entity_t * this, void * thisdata, const entity_t * parent);
    void (*done)(entity_t * this, void * thisdata);

    void (*spawn)(entity_t * this, void * thisdata);

    void (*handle)(entity_t * this, void * thisdata);

    /* соприкосновения объекта с другими объектами */
    size_t entitytouchs_num;
    entitytouch_t * entitytouchs;

    entity_t * (*client_join)(const entity_t * this);

    void * (*client_store)(const void * this_edata);
    void (*client_restore)(entity_t * this, void * thisdata, const void * userstoredata);

    /* массив действий, допустимых для entity */
    unsigned int actions_num;
    entityaction_t * actions;

}entityinfo_t;

extern const entityinfo_t * entityinfo_get(const char * name);
extern void entity_register(const entityinfo_t * info);

extern entity_t * entity_getfirst(const char * name);

extern void entities_handle(void);
extern entity_t * entries_client_join(void);

extern void entities_render(camera_t * cam);

extern entity_t * entity_new(const char * name, vec_t x, vec_t y, direction_t dir, const entity_t * parent);
extern void entities_erase(void);

extern vardata_t * entity_vardata_get(const entity_t * entity, const char * varname, vartype_t vartype);

#define ENTITY_VARIABLE_INTEGER(entity, varname) \
        entity_vardata_get((entity), (varname), VARTYPE_INTEGER)->value.i64
#define ENTITY_VARIABLE_FLOAT(entity, varname) \
        entity_vardata_get((entity), (varname), VARTYPE_FLOAT)->value.f
#define ENTITY_VARIABLE_STRING(entity, varname) \
        entity_vardata_get((entity), (varname), VARTYPE_STRING)->value.string

extern void entity_model_play_start(entity_t * entity, unsigned int imodel, char * actionname);
extern void entity_model_play_pause(entity_t * entity, unsigned int imodel);
extern void entity_model_play_pause_all(entity_t * entity);

extern int entity_model_set(entity_t * entity, unsigned int imodel, const char * modelname);

#endif /* SRC_ENTITY_H_ */
