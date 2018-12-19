/*
 * entity_helpers.h
 *
 *  Created on: 9 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_ENTITY_HELPERS_H_
#define SRC_ENTITY_HELPERS_H_
#include "progs.h"
#include "progs_main.h"

#define MIN(a, b) ((a)<(b) ? (a) : (b))

#define ENTITIES_FOREACH(entity, i) \
        for(entity = &g_entities[i = 0]; i < g_entities_num; entity = &g_entities[i++])

/* цикл по объектам одного определённого типа */
#define ENTITIES_FOREACH_CLASSNAME(entity, entity_classname) \
        for((entity) = entity_first_classname((entity_classname)); !entity_end((entity)) ; (entity) = entity_next_classname((entity), (entity_classname)))

/**
 * @breif Последовательности кадров
 */
typedef struct
{
    unsigned int firstframe; /* первый кадр */
    void (*firstframef)(entity_t * self, unsigned int imodel);
    unsigned int lastframe; /* последний кадр */
    void (*lastframef)(entity_t * self, unsigned int imodel);
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


void coerce_value_int(INTEGER * val, INTEGER min, INTEGER max);

entity_t * entity_init(const char * classname, entity_t * owner);
void entity_done(entity_t * entity);

int entity_classname_cmp(const entity_t * entity, const char * classname);
void entity_cam_set(entity_t *entity, entity_t *cam_entity);
void entity_cam_reset(entity_t *entity);

#ifdef HAVE_STRLCPY
error We have strlcpy(), check the code before use it.
#else
size_t strlcpy(char *__restrict dest, const char *__restrict src, size_t n);
#endif

char * s_snprintf(char * str, size_t n, const char *format, ...);
void entity_vars_set_all(entity_t * entity, const char * info);
entity_t * entity_get_random(const char * entityname);
direction_t entity_direction_invert(direction_t dir);
void entity_move(entity_t * entity, direction_t dir, vec_t speed, bool check_clip);

BOOL entities_in_contact(entity_t * ent1, entity_t * ent2);

int entity_model_set(
    entity_t * entity,
    unsigned int imodel,
    const char * modelname,
    FLOAT modelscale,
    FLOAT translation_x,
    FLOAT translation_y
);

int entity_model_sequence_set(entity_t * entity, unsigned int imodel, const entity_framessequence_t * fseq);
void entity_model_play_start(entity_t * entity, unsigned int imodel);
void entity_model_play_pause(entity_t * entity, unsigned int imodel);
void entity_model_play_pause_all(entity_t * entity);

#endif /* SRC_ENTITY_HELPERS_H_ */
