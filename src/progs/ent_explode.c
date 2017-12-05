/*
 * explode.c
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"

#include "ent_explode.h"
#include "ent_player.h"


//оружия
explodeinfo_t explodeinfo_table[EXPLODE_NUM] =
{
        {  15,   7, MAP_WALL_brick               , SOUND_EXPLODE_ARTILLERY},
        { 100,  50, MAP_WALL_brick | MAP_WALL_W1 , SOUND_EXPLODE_MISSILE  },
        { 200, 100, MAP_WALL_brick | MAP_WALL_W1 , SOUND_EXPLODE_GRENADE  }
};

static void explode_common_destroy_walls(ENTITY self, const explodeinfo_t * explodeinfo)
{
    map_t * map = world_map_get();

    vec_t ix;
    vec_t iy;

    vec_t halfbox = entity_bodybox_get(self) * 0.5f;
    explode_vars_t * explode = entity_vars(self);

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

static void explode_touch_common(ENTITY self, ENTITY other, const explodeinfo_t * explodeinfo)
{
    vec_t r;
    vec2_t d;
    bool self_attack;

    vec_t other_halfbox = entity_bodybox_get(self) * 0.5f;
    explode_vars_t * self_vars = entity_vars(self);
    entity_vars_common_t  * other_vars = entity_vars(other);

    VEC2_SUB(d, other_vars->origin, self_vars->origin);
    if(
            (VEC_ABS(d[0]) <= other_halfbox) &&
            (VEC_ABS(d[1]) <= other_halfbox)
    )
        r = 0;
    else
    {
        r = VEC_SQRT(DOT_PRODUCT2(d, d)) - VEC_SQRT(sqrf(other_halfbox) + VEC_SQRT(other_halfbox))/2;
    }
    if(r <= entity_bodybox_get(self) * 0.5f)
    {
        //r = dx < dy ? dx : dy;
        //взрывом задели себя или товарища по команде(не для монстров)
        self_attack = ( entity_parent(self) == other ) && entity_classname_cmp(other, "player") == 0 ;
        player_getdamage(other, self, self_attack, r, explodeinfo);
    }

}

static void explode_detonate(ENTITY self, explodetype_t type)
{
    const explodeinfo_t * explodeinfo = &explodeinfo_table[type];

    sound_play_start(NULL, 0, explodeinfo->soundIndex, 1);

    explode_common_destroy_walls(self, explodeinfo);
    //проверка попаданий в игрока
    ENTITY attacked;

    ENTITIES_FOREACH(attacked)
    {
        if(
                entity_classname_cmp(attacked, "player") == 0 ||
                entity_classname_cmp(attacked, "enemy") == 0 ||
                entity_classname_cmp(attacked, "boss") == 0
        )
            explode_touch_common(self, attacked, explodeinfo);
    }
}

static void explode_common_modelaction_startframef(ENTITY self, unsigned int imodel)
{
    if(entity_classname_cmp(self, "explode_artillery") == 0)
    {
        explode_detonate(self, EXPLODE_ARTILLERY);
        return;
    }
    if(entity_classname_cmp(self, "explode_missile") == 0);
    {
        explode_detonate(self, EXPLODE_MISSILE);
        return;
    }
    if(entity_classname_cmp(self, "explode_mine") == 0);
    {
        explode_detonate(self, EXPLODE_MINE);
        return;
    }
}

static void explode_common_modelaction_endframef(ENTITY self, unsigned int imodel)
{
    ENTITY parent;
    if((parent = entity_parent(self)))
    {
        player_vars_t * player_vars = entity_vars(parent);
        if(player_vars->bull == self)
        {
            player_vars->bull = NULL;
            /* вернуть камеру игроку */
            entity_cam_reset(parent);
        }
    }
    entity_erase(self);
}

/*
 * explode_small
 */

static const entity_framessequence_t explode_small_fseq_explode =
{
        .firstframe = 0,
        .firstframef = explode_common_modelaction_startframef,
        .lastframe = 7,
        .lastframef = explode_common_modelaction_endframef
};

/*
 * explode_big
 */

static const entity_framessequence_t explode_big_fseq_explode =
{
        .firstframe = 0,
        .firstframef = explode_common_modelaction_startframef,
        .lastframe = 7,
        .lastframef = explode_common_modelaction_endframef
};

ENTITY explode_artillery_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("explode_artillery", parent);

    entity_bodybox_set(self, 14.0f);
    entity_model_set(self, 0, ":/explode_small", 14.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &explode_small_fseq_explode);
    entity_model_play_start(self, 0);

    entity_vars_set_all(self, spawninfo);
    return self;
}

ENTITY explode_missile_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("explode_missile", parent);

    entity_bodybox_set(self, 22.0f);
    entity_model_set(self, 0, ":/explode_big", 22.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &explode_big_fseq_explode);
    entity_model_play_start(self, 0);

    entity_vars_set_all(self, spawninfo);
    return self;
}

ENTITY explode_mine_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("explode_mine", parent);

    entity_bodybox_set(self, 22.0f);
    entity_model_set(self, 0, ":/explode_big", 22.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &explode_big_fseq_explode);
    entity_model_play_start(self, 0);

    entity_vars_set_all(self, spawninfo);
    return self;
}
