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

/*
#define ENTITY_PRINTF_FORMAT "#%ld class \"%s\""
#define ENTITY_PRINTF_VALUE(entity) \
    (long)(entity)->id, (entity)->classname
*/
#define ENTITY_PRINTF_FORMAT "class \"%s\""
#define ENTITY_PRINTF_VALUE(entity) \
    (entity)->classname

#define BODY_ID_MAX SIZE_MAX

typedef struct body_s
{
    CIRCLEQ_ENTRY(body_s) list;
    size_t id;
    /* linked entity */
    entity_common_t * entity;

} body_t;

typedef CIRCLEQ_HEAD(, body_s) body_head_t;

/* привязанные тела, обрабатываются */
extern size_t bodies_num;
extern body_head_t bodies;

void body_link_entity(void * entity);
void body_unlink_entity(void * entity);
void entities_handle(void);

extern body_t * body_find_by_id(size_t ibody);
extern void bodies_render(camera_t * cam);
extern void entities_erase(void);


#endif /* SRC_ENTITY_INTERNAL_H_ */
