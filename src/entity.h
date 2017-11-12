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
#include "vars.h"

/* common */
#define ENTITY_COMMON_STRUCT \
        /* объект живой. влияет на победу игроков и на взаимодействие (touch). */ \
        BOOL alive; \
        /* позиция */ \
        VECTOR2 origin_prev; \
        VECTOR2 origin; \
        /* направление взгляда/движения */ \
        DIRECTION dir; \
        /* статистика */ \
        /* пройденое расстояние */ \
        VECTOR1 stat_traveled_distance;

#define ENTITY_COMMON_VARS \
        VAR_DESCR( VARTYPE_BOOL     , entity_common_t, alive  ), \
        VAR_DESCR( VARTYPE_VECTOR2  , entity_common_t, origin_prev), \
        VAR_DESCR( VARTYPE_VECTOR2  , entity_common_t, origin     ), \
        VAR_DESCR( VARTYPE_DIRECTION, entity_common_t, dir        ), \
        VAR_DESCR( VARTYPE_VECTOR1  , entity_common_t, stat_traveled_distance)


typedef struct entity_common_s
{
    ENTITY_COMMON_STRUCT;
} entity_common_t;

#define ENTITY_NAME_SIZE (64)

#define origin_x origin[0]
#define origin_y origin[1]

#define ENTITY_HALFBODYBOX(ENTITY) (((entity_t *)ENTITY)->info->bodybox * 0.5)

/*
 * цикл по объектам одного определённого типа
 */
#define ENTITIES_FOREACH(entity_name, entity) \
        for(entity = entity_getfirst(entity_name); entity; entity = entity->next)

#define ENTITY_FUNCTION_TOUCH(x) \
        void x (struct entity_s * this, struct entity_s * that)

#define ENTITY_FUNCTION_INIT(x) \
        void x (void /* entity_t */ * this, void * thisdata, void /* entity_t */ * parent)

#define ENTITY_FUNCTION_DONE(x) \
        void x (void /* entity_t */ * this, void * thisdata)

#define ENTITY_FUNCTION_SPAWN(x) \
        void x (void /* entity_t */ * this, void * thisdata)

#define ENTITY_FUNCTION_HANDLE(x) \
        void x (void /* entity_t */ * this, void * thisdata)

#define ENTITY_FUNCTION_PLAYER_SPAWN(x) \
        entity_t * x (void /* entity_t */ * this, void * storage)

#define ENTITY_FUNCTION_NONE NULL

#define ENTITY_FUNCTION_ACTION(x) \
        void x (entity_t * this, void * thisdata, const char * action)


#define ENTITY_ERASE(ENTITY) entity_erase((ENTITY))

/* entity является объектом типа entity_name */
#define ENTITY_IS(entity, entity_name) \
        ( strncmp( ((entity_t*)entity)->info->name, (entity_name), ENTITY_NAME_SIZE ) == 0 )

#define ENTITY_IS_SPAWNED(ENTITY) \
        entity_is_spawned((ENTITY))

#define ENTITY_UNSPAWN(ENTITY) \
        entity_unspawn((ENTITY))

#define ENTITY_SHOW(ENTITY) \
        entity_show((ENTITY))

#define ENTITY_HIDE(ENTITY) \
        entity_hide((ENTITY))

#define ENTITYINFO_VARS(TYPE, xvars) \
        .vars_size = sizeof(TYPE), \
        .vars_descr_num = ARRAYSIZE(xvars), \
        .vars_descr = xvars

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
 * .erased == false
 */
typedef struct entity_s
{
    struct entity_s * prev;
    struct entity_s * next;

    const struct entityinfo_s * info;

    /* объект, за которым следит камера данного объекта, обычно равен this */
    struct entity_s * cam_entity;

    /* родитель объекта. Если родитель уничтожатеся, родителем становится "дедушка" */
    struct entity_s * parent;

    /* структура для проигрывания кардов моделей, связанных с объектом */
    struct ent_modelplayer_s * modelplayers;

    /* удалить объект */
    bool erase;
    /* объект "существует" */
    bool exist;
    /* объект "заморожен" */
    bool freezed; /* temporary unused and not tested */
    /*
    Объект находится на карте, с ним можно взаимодействовать, он отрисовывается.
    Если функция info->spawn() не задана, объект всегда spawned.
    При этом, если принудительно сделать объект не spawned, он никогда не появится,
    потому-что info->handle() никогда не вызовется.
    Ели функция info->spawn() задана, тогда изначально spawned = false
    */
    bool spawned;
    /* объект разрешено показывать. влияет только на рендер. */
    bool allow_draw;

    /* vars */
    void * common; /* entity_common_t */
} entity_t;

typedef struct
{
    char * name;
    unsigned int startframe;
    unsigned int endframe;
    void (*endframef)(void * this, unsigned int imodel, char * actionname);
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

    /* entity flags */
    int flags;
    /* для простоты все объекты квадратные */
    vec_t bodybox;

    /* размер буфера переменных */
    size_t vars_size;
    /* массив описателей переменных */
    size_t vars_descr_num;
    var_descr_t * vars_descr;

    /* размер массива моделей */
    size_t entmodels_num;
    /* массив моделей, связанных с объектом*/
    entitymodel_t * entmodels;

    void (*init)(void /* entity_t */ * this, void * thisdata, void /* entity_t */ * parent);
    void (*done)(void /* entity_t */ * this, void * thisdata);

    void (*spawn)(void /* entity_t */ * this, void * thisdata);

    void (*handle)(void /* entity_t */ * this, void * thisdata);

    /* соприкосновения объекта с другими объектами */
    size_t entitytouchs_num;
    entitytouch_t * entitytouchs;

    ENTITY_FUNCTION_PLAYER_SPAWN((*player_spawn));

    /* массив действий, допустимых для entity */
    unsigned int actions_num;
    entityaction_t * actions;

}entityinfo_t;

extern const entityinfo_t * entityinfo_get(const char * name);
extern void entity_register(const entityinfo_t * info);

extern entity_t * entity_getfirst(const char * name);

extern void entities_handle(void);
extern entity_t * entity_player_spawn_random(void * storage);

extern void entities_render(camera_t * cam);

extern void entity_respawn(entity_t * entity, const void * vars);

extern void entity_erase(void * entity);
extern bool entity_is_spawned(void * entity);
extern void entity_unspawn(void * entity);
extern void entity_show(void * entity);
extern void entity_hide(void * entity);
extern void * entity_parent(void * entity);
extern void * entity_vars(void * entity);
extern void entity_cam_set(void * entity, void * cam_entity);
extern void entity_cam_reset(void * entity);

extern entity_t * entity_new(const char * name, entity_t * parent, const var_value_t * vars_values, size_t vars_values_num);

extern void entities_erase(void);

extern void entity_model_play_start(entity_t * entity, unsigned int imodel, char * actionname);
extern void entity_model_play_pause(entity_t * entity, unsigned int imodel);
extern void entity_model_play_pause_all(entity_t * entity);

extern void entity_restore(entity_t * entity, const void * vars);

extern int entity_model_set(entity_t * entity, unsigned int imodel, const char * modelname);

#endif /* SRC_ENTITY_H_ */
