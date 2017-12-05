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

#include "common/common_queue.h"

#define ENTITY_PRINTF_FORMAT "#%ld class \"%s\""
#define ENTITY_PRINTF_VALUE(entity) \
    (long)(entity)->id, (entity)->classname

#define ENTITY_ID_MAX SIZE_MAX
typedef size_t entity_id_t;
/*
 * список объектов одного типа, у которых
 * .erased == false
 */
typedef struct entity_s
{
    CIRCLEQ_ENTRY(entity_s) list;

    /* вызывается при разрушении объекта в памяти */
    void (*done)(ENTITY self);

    void (*think)(ENTITY self);
    /* соприкосновения объекта с другими объектами */
    void (*touch)(ENTITY self, ENTITY other);

    entity_id_t id;

    char classname[ENTITY_CLASSNAME_SIZE];

    /* entity flags */
    int flags;

    /* для простоты все объекты квадратные */
    FLOAT bodybox;

    const struct game_exports_entityinfo_s * info;


    /* объект, за которым следит камера данного объекта, обычно равен self */
    struct entity_s * cam_entity;

    /* родитель объекта. Если родитель уничтожатеся, родителем становится "дедушка" */
    struct entity_s * parent;

    /* массив: модели */
    entity_model_t * models;

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

typedef CIRCLEQ_HEAD(, entity_s) entity_head_t;

typedef struct
{
    /* информация о объектах в списках */
    const game_exports_entityinfo_t * info;
} entity_registered_t;

extern entity_head_t entities;

/* удалённые объекты */
extern entity_head_t entities_erased;

extern entity_registered_t * entityregs;
extern size_t entityregs_size;
extern size_t entityregs_num;

extern entity_t * entity_find_by_id(entity_id_t entityId);

extern entity_registered_t * entityregisteredinfo_get(const char * name);

extern void entities_handle(void);

extern void entities_render(camera_t * cam);

extern void entities_erase(void);

extern entity_t * entity_new_(const char * name, entity_t * parent, const var_value_t * vars_values, size_t vars_values_num);

#endif /* SRC_ENTITY_INTERNAL_H_ */
