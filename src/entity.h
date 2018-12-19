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
#include "game_progs.h"

#define ENTITY_CLASSNAME_SIZE (64)

#define origin_x origin[0]
#define origin_y origin[1]

/* common entity */
typedef struct entity_common_s entity_common_t;
struct entity_common_s
{
    /* игровой клиент */
    game_client_t * client; /* NULL for no-player entities */

    /* объект живой. влияет на победу игроков и на взаимодействие (touch). */
    BOOL alive;
    /* позиция */
    VECTOR2 origin_prev;
    VECTOR2 origin;
    /* направление взгляда/движения */
    DIRECTION dir;

    /* для простоты все объекты квадратные */
    FLOAT bodybox;

    /* entity flags */
    INTEGER flags;

    INTEGER modelindex_body;
    INTEGER modelindex_flag;

    FLOAT frames_body;
    FLOAT frames_flag;
};

enum
{
    ENTITYFLAG_SOLIDWALL   = 0x01
    //	ENTITYFLAG_SOLIDENTITY = 0x02
};

#endif /* SRC_ENTITY_H_ */
