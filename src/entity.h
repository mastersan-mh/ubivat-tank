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

typedef struct {char binary[1];} * ENTITY;

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

#define ENTITY_HALFBODYBOX(ENTITY) (entity_info_bodybox(ENTITY) * 0.5)

/*
 * цикл по объектам одного определённого типа
 */
#define ENTITIES_FOREACH(entity_name, entity) \
        for(entity = entity_first(entity_name); entity; entity = entity_next(entity))

#define ENTITY_FUNCTION_TOUCH(x) \
        void x (ENTITY this, ENTITY that)

#define ENTITY_FUNCTION_INIT(x) \
        void x (ENTITY this, ENTITY parent)

#define ENTITY_FUNCTION_DONE(x) \
        void x (ENTITY this)

#define ENTITY_FUNCTION_SPAWN(x) \
        void x (ENTITY this)

#define ENTITY_FUNCTION_HANDLE(x) \
        void x (ENTITY this)

#define ENTITY_FUNCTION_PLAYER_SPAWN(x) \
        void * x (ENTITY this, void * storage)

#define ENTITY_FUNCTION_NONE NULL

#define ENTITY_FUNCTION_ACTION(x) \
        void x (ENTITY this, const char * action)


#define ENTITY_ERASE(ENTITY) entity_erase((ENTITY))

#define entity_is_SPAWNED(ENTITY) \
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


typedef struct
{
    char * name;
    unsigned int startframe;
    void (*startframef)(ENTITY this, unsigned int imodel, char * actionname);
    unsigned int endframe;
    void (*endframef)(ENTITY this, unsigned int imodel, char * actionname);
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
    void (*touch)(ENTITY this, ENTITY that);
} entitytouch_t;


typedef struct entityaction_s
{
    char * action;
    void (*action_f)(ENTITY this, const char * action);
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

    void (*init)(ENTITY this, ENTITY parent);
    void (*done)(ENTITY this);

    void (*spawn)(ENTITY this);

    void (*handle)(ENTITY this);

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

extern ENTITY entity_first(const char * name);
extern ENTITY entity_next(ENTITY entity);

extern BOOL entity_is(const ENTITY entity, const char *entity_name);
extern const char * entity_info_name(const ENTITY entity);
VECTOR1 entity_info_bodybox(const ENTITY entity);
extern void entity_erase(ENTITY entity);
extern bool entity_is_spawned(ENTITY entity);
extern void entity_unspawn(ENTITY entity);
extern void entity_show(ENTITY entity);
extern void entity_hide(ENTITY entity);
extern ENTITY entity_parent(ENTITY entity);
extern void * entity_vars(ENTITY entity);
extern void entity_cam_set(ENTITY entity, ENTITY cam_entity);
extern void entity_cam_reset(ENTITY entity);

extern ENTITY entity_new(const char * name, ENTITY parent);

extern void entity_model_play_start(ENTITY entity, unsigned int imodel, char * actionname);
extern void entity_model_play_pause(ENTITY entity, unsigned int imodel);
extern void entity_model_play_pause_all(ENTITY entity);

extern void entity_restore(ENTITY entity, const void * vars);

extern int entity_model_set(ENTITY entity, unsigned int imodel, const char * modelname);

#endif /* SRC_ENTITY_H_ */
