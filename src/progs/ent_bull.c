/*
 * bull.c
 *
 *  Created on: 30 нояб. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"

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

static void bull_artillery_detonate(ENTITY self, ENTITY other)
{
    ENTITY parent = entity_parent(self);
    if (other && parent == other)
        return;

    ENTITY explode = explode_artillery_spawn(parent, NULL);
    entity_vars_common_t* self_vars = entity_vars(self);
    explode_vars_t* explode_vars = entity_vars(explode);
    VEC2_COPY(explode_vars->origin, self_vars->origin);
    explode_vars->dir = self_vars->dir;
    entity_erase(self);
}

static void bull_missile_detonate(ENTITY self, ENTITY other)
{
    ENTITY parent = entity_parent(self);
    if(other && parent == other)
        return;

    ENTITY explode = explode_missile_spawn(parent, NULL);

    bull_vars_t * bull_vars = entity_vars(self);
    explode_vars_t * explode_vars = entity_vars(explode);
    VEC2_COPY(explode_vars->origin, bull_vars->origin);
    explode_vars->dir = bull_vars->dir;

    player_vars_t * player_vars = entity_vars(parent);
    player_vars->bull = explode;
    entity_cam_set(parent, explode);

    entity_erase(self);
}

static void bull_mine_detonate(ENTITY self, ENTITY other)
{
    ENTITY parent = entity_parent(self);
    if(other && parent == other)
        return;

    ENTITY e = explode_mine_spawn(parent, NULL);

    entity_vars_common_t * self_vars = entity_vars(self);
    explode_vars_t * explode_vars = entity_vars(e);
    VEC2_COPY(explode_vars->origin, self_vars->origin);
    explode_vars->dir = self_vars->dir;

    entity_erase(self);
}

/*
 * обработка пули
 * @return true  = сдетонировала
 * @return false = полёт продолжается
 */
static bool bull_common_think(ENTITY self, const bullinfo_t * bullinfo)
{
    map_t * map = world_map_get();

    bull_vars_t * bull = entity_vars(self);
    entity_move(self, bull->dir, bullinfo->speed, false);

    //предельное расстояние пройдено
    if(bullinfo->range > -1 && bull->stat_traveled_distance > bullinfo->range)
        return true;

    //найдем препятствия
    bool Ul,Ur,Dl,Dr,Lu,Ld,Ru,Rd;
    map_clip_find(
        map,
        bull->origin,
        entity_bodybox_get(self),
        MAP_WALL_W0 | MAP_WALL_W1 | MAP_WALL_brick,
        &Ul,&Ur,&Dl,&Dr,&Lu,&Ld,&Ru,&Rd
    );
    if(Ul || Ur || Dl || Dr || Lu || Ld || Ru || Rd)
        return true;
    return false;
}

static void bull_common_modelaction_startplay(ENTITY self, unsigned int imodel)
{
    entity_model_play_start(self, imodel);
}

static ENTITY_FUNCTION_THINK(bull_artillery_think)
{
    const bullinfo_t * bullinfo = &bullinfo_table[BULL_ARTILLERY];
    bool detonate = bull_common_think(self, bullinfo);
    if(detonate)
        bull_artillery_detonate(self, NULL);
}

static ENTITY_FUNCTION_TOUCH(bull_artillery_touch)
{
    if(
            entity_classname_cmp(other, "player") == 0 ||
            entity_classname_cmp(other, "enemy") == 0 ||
            entity_classname_cmp(other, "boss") == 0
    )
        bull_artillery_detonate(self, other);
}

ENTITY bull_artillery_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("bull_artillery", parent);

    entity_thinker_set(self, bull_artillery_think);
    entity_toucher_set(self, bull_artillery_touch);

    entity_flags_set(self, ENTITYFLAG_SOLIDWALL);
    entity_bodybox_set(self, 2.0f);
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

static ENTITY_FUNCTION_THINK(bull_missile_handle)
{
    const bullinfo_t * bullinfo = &bullinfo_table[BULL_MISSILE];
    bool detonate = bull_common_think(self, bullinfo);
    if(detonate)
        bull_missile_detonate(self, NULL);
}

static ENTITY_FUNCTION_TOUCH(bull_missile_touch)
{
    if(
            entity_classname_cmp(other, "player") == 0 ||
            entity_classname_cmp(other, "enemy") == 0 ||
            entity_classname_cmp(other, "boss") == 0
    )
    bull_missile_detonate(self, other);
}

ENTITY bull_missile_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("bull_missile", parent);

    entity_thinker_set(self, bull_missile_handle);
    entity_toucher_set(self, bull_missile_touch);

    entity_flags_set(self, ENTITYFLAG_SOLIDWALL);
    entity_bodybox_set(self, 8.0f);
    entity_model_set(self, 0, ":/bull_missile", 8.0f / 2.0f /* picturesize = 8 x 64 */, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &bull_missile_fseq_fly);
    bull_common_modelaction_startplay(self, 0);

    entity_vars_set_all(self, spawninfo);

    if(parent)
    {
        player_vars_t * parent_vars = entity_vars(parent);
        parent_vars->bull = self;
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

static ENTITY_FUNCTION_THINK(bull_mine_think)
{
    const bullinfo_t * bullinfo = &bullinfo_table[BULL_MINE];
    bool detonate = bull_common_think(self, bullinfo);
    if(detonate)
        bull_mine_detonate(self, NULL);
}

static ENTITY_FUNCTION_TOUCH(bull_mine_touch)
{
    if(
            entity_classname_cmp(other, "player") == 0 ||
            entity_classname_cmp(other, "enemy") == 0 ||
            entity_classname_cmp(other, "boss") == 0
    )
        bull_mine_detonate(self, other);
}

ENTITY bull_mine_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("bull_mine", parent);

    entity_thinker_set(self, bull_mine_think);
    entity_toucher_set(self, bull_mine_touch);

    entity_flags_set(self, ENTITYFLAG_SOLIDWALL);
    entity_bodybox_set(self, 8.0f);
    entity_model_set(self, 0, ":/bull_mine", 8.0f / 2.0f /* picturesize = 8 x 64 */, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &bull_mine_fseq_fly);
    bull_common_modelaction_startplay(self, 0);

    entity_vars_set_all(self, spawninfo);
    return self;
}
