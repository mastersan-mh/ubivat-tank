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
        FLOAT stat_traveled_distance;

#define ENTITY_VARS_COMMON \
        VAR_DESCR( VARTYPE_BOOL     , entity_vars_common_t, alive  ), \
        VAR_DESCR( VARTYPE_VECTOR2  , entity_vars_common_t, origin_prev), \
        VAR_DESCR( VARTYPE_VECTOR2  , entity_vars_common_t, origin     ), \
        VAR_DESCR( VARTYPE_DIRECTION, entity_vars_common_t, dir        ), \
        VAR_DESCR( VARTYPE_FLOAT    , entity_vars_common_t, stat_traveled_distance)


typedef struct
{
    ENTITY_VARS_COMMON_STRUCT;
} entity_vars_common_t;

#define ENTITY_CLASSNAME_SIZE (64)

#define origin_x origin[0]
#define origin_y origin[1]

#define ENTITIES_FOREACH(entity) \
        for((entity) = entity_first(); !entity_end((entity)) ; (entity) = entity_next((entity)))

/* цикл по объектам одного определённого типа */
#define ENTITIES_FOREACH_CLASSNAME(entity, entity_classname) \
        for((entity) = entity_first_classname((entity_classname)); !entity_end((entity)) ; (entity) = entity_next_classname((entity), (entity_classname)))

#define ENTITY_FUNCTION_THINK(x) \
        void x (ENTITY self)

#define ENTITY_FUNCTION_TOUCH(x) \
        void x (ENTITY self, ENTITY other)

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
    unsigned int firstframe; /* первый кадр */
    void (*firstframef)(ENTITY self, unsigned int imodel);
    unsigned int lastframe; /* последний кадр */
    void (*lastframef)(ENTITY self, unsigned int imodel);
} entity_framessequence_t;

/* структура для проигрывания последовательности кадров моделей, связанных с объектом */
typedef struct entity_modelplayer_s
{
    /* воспроизводимая последовательность */
    const entity_framessequence_t * fseq;
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

typedef struct game_exports_entityinfo_s
{
    char * classname;

    /* размер буфера переменных */
    size_t vars_size;
    /* массив описателей переменных */
    size_t vars_descr_num;
    const var_descr_t * vars_descr;

    /* размер массива моделей */
    size_t models_num;
}game_exports_entityinfo_t;

bool entity_end(ENTITY entity);
ENTITY entity_first(void);
ENTITY entity_next(ENTITY entity);
ENTITY entity_first_classname(const char * entity_name);
ENTITY entity_next_classname(ENTITY entity, const char * entity_name);

void entity_vars_descr_get(ENTITY entity, const var_descr_t ** vars_descr, size_t * vars_descr_num);
int entity_classname_cmp(const ENTITY entity, const char * classname);
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

ENTITY entity_spawn(const char * name, ENTITY parent);

void entity_done_set(ENTITY entity, void (*done)(ENTITY self));
void entity_thinker_set(ENTITY entity, void (*think)(ENTITY self));
void entity_toucher_set(ENTITY entity, void (*touch)(ENTITY self, ENTITY other));

void entity_flags_set(ENTITY entity, int flags);
void entity_bodybox_set(ENTITY entity, FLOAT bodybox);
FLOAT entity_bodybox_get(const ENTITY entity);
int entity_model_set(
    ENTITY entity,
    unsigned int imodel,
    const char * modelname,
    FLOAT modelscale,
    FLOAT translation_x,
    FLOAT translation_y
);

extern int entity_model_sequence_set(ENTITY entity, unsigned int imodel, const entity_framessequence_t * fseq);

extern void entity_model_play_start(ENTITY entity, unsigned int imodel);
extern void entity_model_play_pause(ENTITY entity, unsigned int imodel);
extern void entity_model_play_pause_all(ENTITY entity);

extern void entity_restore(ENTITY entity, const void * vars);


#endif /* SRC_ENTITY_H_ */
