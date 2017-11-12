/*
 * entity_internal.h
 *
 *  Created on: 12 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_ENTITY_INTERNAL_H_
#define SRC_ENTITY_INTERNAL_H_

#include "types.h"
#include "entity.h"

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
    void * vars; /* entity_common_t */
} entity_t;

typedef struct
{
    /* информация о объектах в списках */
    const entityinfo_t * info;
    /* существующие объекты (все одного типа) */
    entity_t * entities;
    /* удалённые объекты (все одного типа) */
    entity_t * entities_erased;
} entity_registered_t;

extern entity_registered_t * entityregs;
extern size_t entityregs_size;
extern size_t entityregs_num;

extern entity_registered_t * entityregisteredinfo_get(const char * name);

extern const ent_modelaction_t * entity_reginfo_action_get(const entityinfo_t * info, unsigned int imodel, char * actionname);

extern void entities_handle(void);

extern void entities_render(camera_t * cam);

extern void entities_erase(void);

extern entity_t * entity_new_(const char * name, entity_t * parent, const var_value_t * vars_values, size_t vars_values_num);

extern entity_t * entity_player_spawn_random(void * storage);

extern void entity_respawn(entity_t * entity, const void * vars);

#endif /* SRC_ENTITY_INTERNAL_H_ */
