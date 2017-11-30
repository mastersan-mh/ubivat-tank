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
#define ENTITY_VARS_COMMON_STRUCT \
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

#define ENTITY_VARS_COMMON \
        VAR_DESCR( VARTYPE_BOOL     , entity_vars_common_t, alive  ), \
        VAR_DESCR( VARTYPE_VECTOR2  , entity_vars_common_t, origin_prev), \
        VAR_DESCR( VARTYPE_VECTOR2  , entity_vars_common_t, origin     ), \
        VAR_DESCR( VARTYPE_DIRECTION, entity_vars_common_t, dir        ), \
        VAR_DESCR( VARTYPE_VECTOR1  , entity_vars_common_t, stat_traveled_distance)


typedef struct
{
    ENTITY_VARS_COMMON_STRUCT;
} entity_vars_common_t;

#define ENTITY_CLASSNAME_SIZE (64)

#define origin_x origin[0]
#define origin_y origin[1]

#define ENTITIES_FOREACH(entity) \
        for((entity) = entity_first((entity_name)); !entity_end(entity) ; (entity) = entity_next((entity), (entity_name)))

/* цикл по объектам одного определённого типа */
#define ENTITIES_FOREACH_NAME(entity_name, entity) \
        for((entity) = entity_first_classname((entity_name)); !entity_end(entity) ; (entity) = entity_next_classname((entity), (entity_name)))

#define ENTITY_FUNCTION_TOUCH(x) \
        void x (ENTITY this, ENTITY other)

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

#define ENTITYINFO_VARS(TYPE, xvars) \
        .vars_size = sizeof(TYPE), \
        .vars_descr_num = ARRAYSIZE(xvars), \
        .vars_descr = xvars

#define ENTITYINFO_FRAMESSEQ(xframessequences) \
        .framessequences_num = ARRAYSIZE(xframessequences), \
        .framessequences = xframessequences

#define ENTITYINFO_FRAMESSEQ_NONE() \
        .framessequences_num = 0, \
        .framessequences = NULL

#define ENTITYINFO_ACTIONS(xactions) \
        .actions_num = ARRAYSIZE(xactions), \
        .actions = xactions

enum
{
    ENTITYFLAG_SOLIDWALL   = 0x01
    //	ENTITYFLAG_SOLIDENTITY = 0x02
};

/**
 * @breif Последовательности кадров
 */
typedef struct
{
    unsigned int imodel; /* индекс модели, для которой предназначена последовательность */
    char * seqname; /* имя последовательности */
    unsigned int firstframe; /* первый кадр */
    void (*firstframef)(ENTITY self, unsigned int imodel, const char * seqname);
    unsigned int lastframe; /* последний кадр */
    void (*lastframef)(ENTITY self, unsigned int imodel, const char * seqname);
} entity_framessequence_t;

/* структура для проигрывания последовательности кадров моделей, связанных с объектом */
typedef struct entity_modelplayer_s
{
    /* воспроизводимая последовательность */
    const entity_framessequence_t * play_frames_seq;
    /* номер кадра */
    float frame;
} entity_modelplayer_t;

typedef struct
{
    /* имя модели */
    char * name;
    model_t * model;
    FLOAT scale;
    vec2_t translation;
    entity_modelplayer_t player;
} entity_model_t;

typedef struct entityaction_s
{
    char * action;
    void (*action_f)(ENTITY self, const char * action);
} entity_action_t;

typedef struct entityinfo_s
{
    char * name_;

    /* размер буфера переменных */
    size_t vars_size;
    /* массив описателей переменных */
    size_t vars_descr_num;
    const var_descr_t * vars_descr;

    /* количество действий */
    size_t framessequences_num;
    /* действия */
    const entity_framessequence_t * framessequences;

    /* размер массива моделей */
    size_t models_num;

    void (*init)(ENTITY self, ENTITY parent);
    void (*done)(ENTITY self);

    void (*spawn)(ENTITY self);

    void (*handle)(ENTITY self);

    /* соприкосновения объекта с другими объектами */
    void (*touch)(ENTITY self, ENTITY other);

    ENTITY_FUNCTION_PLAYER_SPAWN((*player_spawn));

    /* массив действий, допустимых для entity, при управлении игроком */
    size_t actions_num;
    entity_action_t * actions;

}entityinfo_t;

extern const entityinfo_t * entityinfo_get(const char * name);
extern void entity_register(const entityinfo_t * info);

extern ENTITY entity_first_classname(const char * entity_name);
extern ENTITY entity_next_classname(ENTITY entity, const char * entity_name);
extern bool entity_end(ENTITY entity);

extern int entity_classname_cmp(const ENTITY entity, const char * classname);
extern const char * entity_classname_get(const ENTITY entity);
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
extern void entity_flags_set(ENTITY entity, int flags);
extern void entity_bodybox_set(ENTITY entity, FLOAT bodybox);
extern FLOAT entity_bodybox_get(const ENTITY entity);
extern int entity_model_set(
    ENTITY entity,
    unsigned int imodel,
    const char * modelname,
    FLOAT modelscale,
    VECTOR1 translation_x,
    VECTOR1 translation_y
);
extern void entity_model_play_start(ENTITY entity, unsigned int imodel, const char * seqname);
extern void entity_model_play_pause(ENTITY entity, unsigned int imodel);
extern void entity_model_play_pause_all(ENTITY entity);

extern void entity_restore(ENTITY entity, const void * vars);


#endif /* SRC_ENTITY_H_ */
