/*
 * explode.c
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"

#include "ent_explode.h"
#include "ent_player.h"

static var_descr_t explode_vars[] =
{
        ENTITY_VARS_COMMON,
};


//оружия
explodeinfo_t explodeinfo_table[EXPLODE_NUM] =
{
        {  15,   7, MAP_WALL_brick               , SOUND_EXPLODE_ARTILLERY},
        { 100,  50, MAP_WALL_brick | MAP_WALL_W1 , SOUND_EXPLODE_MISSILE  },
        { 200, 100, MAP_WALL_brick | MAP_WALL_W1 , SOUND_EXPLODE_GRENADE  }
};

static void explode_common_destroy_walls(ENTITY this, const explodeinfo_t * explodeinfo)
{
    map_t * map = world_map_get();

    vec_t ix;
    vec_t iy;

    vec_t halfbox = entity_bodybox_get(this) * 0.5f;
    entity_explode_t * explode = entity_vars(this);

    //проверка попаданий в стены
    for(iy = -halfbox; iy <= halfbox; iy++)
    {
        for(ix = -halfbox; ix <= halfbox; ix++)
        {
            int x8 = VEC_TRUNC((explode->origin[0] + ix) / 8);
            int y8 = VEC_TRUNC((explode->origin[1] + iy) / 8);
            if(
                    0 <= x8 && x8 < MAP_SX &&
                    0 <= y8 && y8 < MAP_SY
            )
            {
                char wall = map->map[y8][x8];
                if(MAP_WALL_CLIPPED(wall) && ( MAP_WALL_TEXTURE(wall) & explodeinfo->wall ) )
                    map->map[y8][x8] = 0;
            }
        }
    }
}

static void explode_touch_common(ENTITY this, ENTITY that, const explodeinfo_t * explodeinfo)
{
    vec_t r;
    vec2_t d;
    bool self;

    vec_t that_halfbox = entity_bodybox_get(this) * 0.5f;
    entity_explode_t * this_vars = entity_vars(this);
    entity_vars_common_t  * that_vars = entity_vars(that);

    VEC2_SUB(d, that_vars->origin, this_vars->origin);
    if(
            (VEC_ABS(d[0]) <= that_halfbox) &&
            (VEC_ABS(d[1]) <= that_halfbox)
    )
        r = 0;
    else
    {
        r = VEC_SQRT(DOT_PRODUCT2(d, d)) - VEC_SQRT(sqrf(that_halfbox) + VEC_SQRT(that_halfbox))/2;
    }
    if(r <= entity_bodybox_get(this) * 0.5f)
    {
        //r = dx < dy ? dx : dy;
        //взрывом задели себя или товарища по команде(не для монстров)
        self = ( entity_parent(this) == that ) && entity_is(that, "player") ;
        player_getdamage(that, this, self, r, explodeinfo);
    }

}

static void explode_detonate(ENTITY this, explodetype_t type)
{
    const explodeinfo_t * explodeinfo = &explodeinfo_table[type];

    sound_play_start(NULL, 0, explodeinfo->soundIndex, 1);

    explode_common_destroy_walls(this, explodeinfo);
    //проверка попаданий в игрока
    ENTITY ent_attacked;

    ENTITIES_FOREACH_NAME("player", ent_attacked)
    {
        explode_touch_common(this, ent_attacked, explodeinfo);
    }
    ENTITIES_FOREACH_NAME("enemy", ent_attacked)
    {
        explode_touch_common(this, ent_attacked, explodeinfo);
    }
    ENTITIES_FOREACH_NAME("boss", ent_attacked)
    {
        explode_touch_common(this, ent_attacked, explodeinfo);
    }

}

static void explode_common_modelaction_startframef(ENTITY this, unsigned int imodel, const char * actionname)
{
    if(entity_is(this, "explode_artillery"))
    {
        explode_detonate(this, EXPLODE_ARTILLERY);
        return;
    }
    if(entity_is(this, "explode_missile"));
    {
        explode_detonate(this, EXPLODE_MISSILE);
        return;
    }
    if(entity_is(this, "explode_mine"));
    {
        explode_detonate(this, EXPLODE_MINE);
        return;
    }
}

static void explode_common_modelaction_endframef(ENTITY this, unsigned int imodel, const char * actionname)
{
    ENTITY parent;
    if((parent = entity_parent(this)))
    {
        player_vars_t * player_vars = entity_vars(parent);
        if(player_vars->bull == this)
        {
            player_vars->bull = NULL;
            /* вернуть камеру игроку */
            entity_cam_reset(parent);
        }
    }
    ENTITY_ERASE(this);
}

/*
 * explode_small
 */

static const entity_framessequence_t explode_small_modelactions[] =
{
        {
                .imodel = 0,
                .seqname = "explode",
                .firstframe = 0,
                .firstframef = explode_common_modelaction_startframef,
                .lastframe = 7,
                .lastframef = explode_common_modelaction_endframef
        }
};

/*
 * explode_big
 */

static const entity_framessequence_t explode_big_modelactions[] =
{
        {
                .imodel = 0,
                .seqname = "explode",
                .firstframe = 0,
                .firstframef = explode_common_modelaction_startframef,
                .lastframe = 7,
                .lastframef = explode_common_modelaction_endframef
        }
};

static ENTITY_FUNCTION_INIT(explode_artillery_init)
{
    entity_bodybox_set(this, 14.0f);
    entity_model_set(this, 0, ":/explode_small", 14.0f / 2.0f, 0.0f, 0.0f);
    entity_model_play_start(this, 0, "explode");
}

static ENTITY_FUNCTION_INIT(explode_missile_init)
{
    entity_bodybox_set(this, 22.0f);
    entity_model_set(this, 0, ":/explode_big", 22.0f / 2.0f, 0.0f, 0.0f);
    entity_model_play_start(this, 0, "explode");
}

static ENTITY_FUNCTION_INIT(explode_mine_init)
{
    entity_bodybox_set(this, 22.0f);
    entity_model_set(this, 0, ":/explode_big", 22.0f / 2.0f, 0.0f, 0.0f);
    entity_model_play_start(this, 0, "explode");
}

static const entityinfo_t explode_artillery_reginfo = {
        .name_ = "explode_artillery",
        ENTITYINFO_VARS(entity_explode_t, explode_vars),
        ENTITYINFO_FRAMESSEQ(explode_small_modelactions),
        .models_num = 1,
        .init = explode_artillery_init,
        .done = ENTITY_FUNCTION_NONE,
        .handle = ENTITY_FUNCTION_NONE,
};

static const entityinfo_t explode_missile_reginfo = {
        .name_ = "explode_missile",
        ENTITYINFO_VARS(entity_explode_t, explode_vars),
        ENTITYINFO_FRAMESSEQ(explode_big_modelactions),
        .models_num = 1,
        .init = explode_missile_init,
        .done = ENTITY_FUNCTION_NONE,
        .handle = ENTITY_FUNCTION_NONE,
};

static const entityinfo_t explode_mine_reginfo = {
        .name_ = "explode_mine",
        ENTITYINFO_VARS(entity_explode_t, explode_vars),
        ENTITYINFO_FRAMESSEQ(explode_big_modelactions),
        .models_num = 1,
        .init = explode_mine_init,
        .done = ENTITY_FUNCTION_NONE,
        .handle = ENTITY_FUNCTION_NONE,
};

void entity_explode_init(void)
{
    entity_register(&explode_artillery_reginfo);
    entity_register(&explode_missile_reginfo);
    entity_register(&explode_mine_reginfo);
}
