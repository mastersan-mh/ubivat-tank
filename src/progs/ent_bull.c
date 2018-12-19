/*
 * bull.c
 *
 *  Created on: 30 нояб. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"
#include "helpers.h"

#include "ent_bull.h"
#include "ent_explode.h"
#include "ent_player.h"
#include "ent_weap.h"

typedef enum
{
    BULL_ARTILLERY,
    BULL_MISSILE,
    BULL_MINE,
    BULL_NUM
} bulltype_t;

typedef struct
{
    //повреждение
    int damage;
    //повреждение
    int selfdamage;
    //дальность
    vec_t range;
    //начальная скорость пули
    vec_t speed;
} bullinfo_t;

static const bullinfo_t bullinfo_table[BULL_NUM] =
{
        { 15,   7,  -1,  75 },
        {100,  50,  -1,  80 },
        {200, 100, 100,  80 }
};

static void bull_artillery_detonate(entity_t * self, entity_t * other)
{
    entity_t * parent = self->owner;
    if (other && parent == other)
        return;

    entity_t * explode = explode_artillery_spawn(parent, NULL);
    VEC2_COPY(explode->c.origin, self->c.origin);
    explode->c.dir = self->c.dir;
    gi->unlink(self);
}

static void bull_missile_detonate(entity_t * self, entity_t * other)
{
    entity_t * owner = self->owner;
    if(other && owner == other)
        return;

    entity_t * explode = explode_missile_spawn(owner, NULL);

    VEC2_COPY(explode->c.origin, self->c.origin);
    explode->c.dir = self->c.dir;

    owner->bull = explode;
    entity_cam_set(owner, explode);
    gi->unlink(self);
}

static void bull_mine_detonate(entity_t * self, entity_t * other)
{
    entity_t * owner = self->owner;
    if(other && owner == other)
        return;

    entity_t * explode = explode_mine_spawn(owner, NULL);
    VEC2_COPY(explode->c.origin, self->c.origin);
    explode->c.dir = self->c.dir;

    gi->unlink(self);
}

/*
 * обработка пули
 * @return true  = сдетонировала
 * @return false = полёт продолжается
 */
static bool bull_common_think(entity_t * self, const bullinfo_t * bullinfo)
{
    map_t * map = world_map_get();

    entity_move(self, self->c.dir, bullinfo->speed, false);

    //предельное расстояние пройдено
    if(bullinfo->range > -1 && self->stat_traveled_distance > bullinfo->range)
        return true;

    //найдем препятствия
    bool Ul,Ur,Dl,Dr,Lu,Ld,Ru,Rd;
    map_clip_find(
        map,
        self->c.origin,
        self->c.bodybox,
        MAP_WALL_W0 | MAP_WALL_W1 | MAP_WALL_brick,
        &Ul,&Ur,&Dl,&Dr,&Lu,&Ld,&Ru,&Rd
    );
    if(Ul || Ur || Dl || Dr || Lu || Ld || Ru || Rd)
        return true;
    return false;
}

static void bull_common_modelaction_startplay(entity_t * self, unsigned int imodel)
{
    entity_model_play_start(self, imodel);
}

static void bull_artillery_think(entity_t * self)
{
    const bullinfo_t * bullinfo = &bullinfo_table[BULL_ARTILLERY];
    bool detonate = bull_common_think(self, bullinfo);
    if(detonate)
        bull_artillery_detonate(self, NULL);
}

static void bull_artillery_touch(entity_t * self, entity_t * other)
{
    if(
            entity_classname_cmp(other, "player") == 0 ||
            entity_classname_cmp(other, "enemy") == 0 ||
            entity_classname_cmp(other, "boss") == 0
    )
        bull_artillery_detonate(self, other);
}

entity_t * bull_artillery_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("bull_artillery", parent);

    self->c.flags = ENTITYFLAG_SOLIDWALL;
    self->c.bodybox = 2.0f;

    self->think = bull_artillery_think;
    self->touch = bull_artillery_touch;

    entity_model_set(self, 0, ":/bull_artillery", 2.0f / 2.0f /* picturesize = 2 x 2 */, 0.0f, 0.0f);

    entity_vars_set_all(self, spawninfo);

    return self;
}

/**
 * bull_missile
 */

static const entity_framessequence_t bull_missile_fseq_fly =
{
        .firstframe = 0,
        .firstframef = NULL,
        .lastframe = 1,
        .lastframef = bull_common_modelaction_startplay
};

static void bull_missile_handle(entity_t * self)
{
    const bullinfo_t * bullinfo = &bullinfo_table[BULL_MISSILE];
    bool detonate = bull_common_think(self, bullinfo);
    if(detonate)
        bull_missile_detonate(self, NULL);
}

static void bull_missile_touch(entity_t * self, entity_t * other)
{
    if(
            entity_classname_cmp(other, "player") == 0 ||
            entity_classname_cmp(other, "enemy") == 0 ||
            entity_classname_cmp(other, "boss") == 0
    )
    bull_missile_detonate(self, other);
}

entity_t * bull_missile_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("bull_missile", parent);

    self->c.flags = ENTITYFLAG_SOLIDWALL;
    self->c.bodybox = 8.0f;

    self->think = bull_missile_handle;
    self->touch = bull_missile_touch;

    entity_model_set(self, 0, ":/bull_missile", 8.0f / 2.0f /* picturesize = 8 x 64 */, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &bull_missile_fseq_fly);
    bull_common_modelaction_startplay(self, 0);

    entity_vars_set_all(self, spawninfo);

    if(parent)
    {
        parent->bull = self;
        entity_cam_set(parent, self);
    }
    return self;
}

/**
 * bull_mine
 */
static const entity_framessequence_t bull_mine_fseq_fly =
{
        .firstframe = 0,
        .firstframef = NULL,
        .lastframe = 1,
        .lastframef = bull_common_modelaction_startplay
};

static void bull_mine_think(entity_t * self)
{
    const bullinfo_t * bullinfo = &bullinfo_table[BULL_MINE];
    bool detonate = bull_common_think(self, bullinfo);
    if(detonate)
        bull_mine_detonate(self, NULL);
}

static void bull_mine_touch(entity_t * self, entity_t * other)
{
    if(
            entity_classname_cmp(other, "player") == 0 ||
            entity_classname_cmp(other, "enemy") == 0 ||
            entity_classname_cmp(other, "boss") == 0
    )
        bull_mine_detonate(self, other);
}

entity_t * bull_mine_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("bull_mine", parent);

    self->think = bull_mine_think;
    self->touch = bull_mine_touch;

    self->c.flags =  ENTITYFLAG_SOLIDWALL;
    self->c.bodybox = 8.0f;
    entity_model_set(self, 0, ":/bull_mine", 8.0f / 2.0f /* picturesize = 8 x 64 */, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &bull_mine_fseq_fly);
    bull_common_modelaction_startplay(self, 0);

    entity_vars_set_all(self, spawninfo);
    return self;
}
