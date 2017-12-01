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

static var_descr_t bull_vars[] =
{
        ENTITY_VARS_COMMON,
};

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

static void bull_artillery_detonate(ENTITY this, ENTITY that)
{
    ENTITY player = entity_parent(this);
    if(that && player == that)
        return;

    ENTITY explode = entity_new("explode_artillery", player);
    entity_vars_common_t * this_vars = entity_vars(this);
    entity_explode_t * explode_vars = entity_vars(explode);
    VEC2_COPY(explode_vars->origin, this_vars->origin);
    explode_vars->dir = this_vars->dir;

    entity_erase(this);
}

static void bull_missile_detonate(ENTITY this, ENTITY that)
{
    ENTITY player = entity_parent(this);
    if(that && player == that)
        return;

    ENTITY e = entity_new("explode_missile", player);

    bull_vars_t * bull_vars = entity_vars(this);
    entity_explode_t * explode_vars = entity_vars(e);
    VEC2_COPY(explode_vars->origin, bull_vars->origin);
    explode_vars->dir = bull_vars->dir;

    player_vars_t * player_vars = entity_vars(player);
    player_vars->bull = e;

    entity_cam_set(player, e);

    entity_erase(this);
}

static void bull_mine_detonate(ENTITY this, ENTITY that)
{
    ENTITY player = entity_parent(this);
    if(that && player == that)
        return;

    ENTITY e = entity_new("explode_mine", player);

    entity_vars_common_t * this_vars = entity_vars(this);
    entity_explode_t * explode_vars = entity_vars(e);
    VEC2_COPY(explode_vars->origin, this_vars->origin);
    explode_vars->dir = this_vars->dir;

    entity_erase(this);
}

/*
 * обработка пули
 * @return true  = сдетонировала
 * @return false = полёт продолжается
 */
static bool bull_common_handle(ENTITY this, const bullinfo_t * bullinfo)
{
    map_t * map = world_map_get();

    bull_vars_t * bull = entity_vars(this);
    entity_move(this, bull->dir, bullinfo->speed, false);

    //предельное расстояние пройдено
    if(bullinfo->range > -1 && bull->stat_traveled_distance > bullinfo->range)
        return true;

    //найдем препятствия
    bool Ul,Ur,Dl,Dr,Lu,Ld,Ru,Rd;
    map_clip_find(
        map,
        bull->origin,
        entity_bodybox_get(this),
        MAP_WALL_W0 | MAP_WALL_W1 | MAP_WALL_brick,
        &Ul,&Ur,&Dl,&Dr,&Lu,&Ld,&Ru,&Rd
    );
    if(Ul || Ur || Dl || Dr || Lu || Ld || Ru || Rd)
        return true;
    return false;
}

static void bull_common_modelaction_startplay(ENTITY this, unsigned int imodel)
{
    entity_model_play_start(this, imodel);
}

static ENTITY_FUNCTION_HANDLE(bull_artillery_handle)
{
    const bullinfo_t * bullinfo = &bullinfo_table[BULL_ARTILLERY];
    bool detonate = bull_common_handle(this, bullinfo);
    if(detonate)
        bull_artillery_detonate(this, NULL);
}

static ENTITY_FUNCTION_TOUCH(bull_artillery_touch)
{
    if(
            entity_classname_cmp(other, "player") == 0 ||
            entity_classname_cmp(other, "enemy") == 0 ||
            entity_classname_cmp(other, "boss") == 0
    )
        bull_artillery_detonate(this, other);
}

static ENTITY_FUNCTION_INIT(bull_artillery_init)
{
    entity_flags_set(this, ENTITYFLAG_SOLIDWALL);
    entity_bodybox_set(this, 2.0f);
    entity_model_set(this, 0, ":/bull_artillery", 2.0f / 2.0f /* picturesize = 2 x 2 */, 0.0f, 0.0f);
}

static const entityinfo_t bull_artillery_reginfo = {
        .name_ = "bull_artillery",
        ENTITYINFO_VARS(bull_vars_t, bull_vars),
        .models_num = 1,
        .init = bull_artillery_init,
        .done = ENTITY_FUNCTION_NONE,
        .handle = bull_artillery_handle,
        .touch  = bull_artillery_touch
};

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

static ENTITY_FUNCTION_INIT(bull_missile_init)
{
    entity_flags_set(this, ENTITYFLAG_SOLIDWALL);
    entity_bodybox_set(this, 8.0f);
    entity_model_set(this, 0, ":/bull_missile", 8.0f / 2.0f /* picturesize = 8 x 64 */, 0.0f, 0.0f);
    entity_model_sequence_set(this, 0, &bull_missile_fseq_fly);
    bull_common_modelaction_startplay(this, 0);
    player_vars_t * parent_vars = entity_vars(parent);
    parent_vars->bull = this;
    entity_cam_set(parent, this);
}

static ENTITY_FUNCTION_HANDLE(bull_missile_handle)
{
    const bullinfo_t * bullinfo = &bullinfo_table[BULL_MISSILE];
    bool detonate = bull_common_handle(this, bullinfo);
    if(detonate)
        bull_missile_detonate(this, NULL);
}

ENTITY_FUNCTION_TOUCH(bull_missile_touch)
{
    if(
            entity_classname_cmp(other, "player") == 0 ||
            entity_classname_cmp(other, "enemy") == 0 ||
            entity_classname_cmp(other, "boss") == 0
    )
    bull_missile_detonate(this, other);
}


static const entityinfo_t bull_missile_reginfo = {
        .name_ = "bull_missile",
        ENTITYINFO_VARS(bull_vars_t, bull_vars),
        .models_num = 1,
        .init = bull_missile_init,
        .done = ENTITY_FUNCTION_NONE,
        .handle = bull_missile_handle,
        .touch = bull_missile_touch
};

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


static ENTITY_FUNCTION_INIT(bull_mine_init)
{
    entity_flags_set(this, ENTITYFLAG_SOLIDWALL);
    entity_bodybox_set(this, 8.0f);
    entity_model_set(this, 0, ":/bull_mine", 8.0f / 2.0f /* picturesize = 8 x 64 */, 0.0f, 0.0f);
    entity_model_sequence_set(this, 0, &bull_mine_fseq_fly);
    bull_common_modelaction_startplay(this, 0);
}

static ENTITY_FUNCTION_HANDLE(bull_mine_handle)
{
    const bullinfo_t * bullinfo = &bullinfo_table[BULL_MINE];
    bool detonate = bull_common_handle(this, bullinfo);
    if(detonate)
        bull_mine_detonate(this, NULL);
}

ENTITY_FUNCTION_TOUCH(bull_mine_touch)
{
    if(
            entity_classname_cmp(other, "player") == 0 ||
            entity_classname_cmp(other, "enemy") == 0 ||
            entity_classname_cmp(other, "boss") == 0
    )
        bull_mine_detonate(this, other);
}

static const entityinfo_t bull_mine_reginfo =
{
        .name_ = "bull_mine",
        ENTITYINFO_VARS(bull_vars_t, bull_vars),
        .models_num = 1,
        .init = bull_mine_init,
        .done = ENTITY_FUNCTION_NONE,
        .handle = bull_mine_handle,
        .touch = bull_mine_touch
};

/**
 * регистрация
 */
void entity_bull_init(void)
{
    entity_register(&bull_artillery_reginfo);
    entity_register(&bull_missile_reginfo);
    entity_register(&bull_mine_reginfo);
}
