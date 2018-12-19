/*
 * explode.c
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"
#include "helpers.h"

#include "ent_explode.h"
#include "ent_player.h"


//оружия
explodeinfo_t explodeinfo_table[EXPLODE_NUM] =
{
        {  15,   7, MAP_WALL_brick               , SOUND_EXPLODE_ARTILLERY},
        { 100,  50, MAP_WALL_brick | MAP_WALL_W1 , SOUND_EXPLODE_MISSILE  },
        { 200, 100, MAP_WALL_brick | MAP_WALL_W1 , SOUND_EXPLODE_GRENADE  }
};

static void explode_common_destroy_walls(entity_t * self, const explodeinfo_t * explodeinfo)
{
    map_t * map = world_map_get();

    vec_t ix;
    vec_t iy;

    vec_t halfbox = self->c.bodybox * 0.5f;

    //проверка попаданий в стены
    for(iy = -halfbox; iy <= halfbox; iy++)
    {
        for(ix = -halfbox; ix <= halfbox; ix++)
        {
            int x8 = VEC_TRUNC((self->c.origin[0] + ix) / 8);
            int y8 = VEC_TRUNC((self->c.origin[1] + iy) / 8);
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

static void explode_touch_common(entity_t * self, entity_t * other, const explodeinfo_t * explodeinfo)
{
    vec_t r;
    vec2_t d;
    bool self_attack;

    vec_t other_halfbox = self->c.bodybox * 0.5f;

    VEC2_SUB(d, other->c.origin, self->c.origin);
    if(
            (VEC_ABS(d[0]) <= other_halfbox) &&
            (VEC_ABS(d[1]) <= other_halfbox)
    )
        r = 0;
    else
    {
        r = VEC_SQRT(DOT_PRODUCT2(d, d)) - VEC_SQRT(sqrf(other_halfbox) + VEC_SQRT(other_halfbox))/2;
    }
    if(r <= self->c.bodybox * 0.5f)
    {
        //r = dx < dy ? dx : dy;
        //взрывом задели себя или товарища по команде(не для монстров)
        self_attack = ( self->owner == other ) && entity_classname_cmp(other, "player") == 0 ;
        player_getdamage(other, self, self_attack, r, explodeinfo);
    }

}

static void explode_detonate(entity_t * self, explodetype_t type)
{
    const explodeinfo_t * explodeinfo = &explodeinfo_table[type];

    sound_play_start(NULL, 0, explodeinfo->soundIndex, 1);

    explode_common_destroy_walls(self, explodeinfo);
    //проверка попаданий в игрока
    entity_t * attacked;
    size_t i;
    ENTITIES_FOREACH(attacked, i)
    {
        if(
                entity_classname_cmp(attacked, "player") == 0 ||
                entity_classname_cmp(attacked, "enemy") == 0 ||
                entity_classname_cmp(attacked, "boss") == 0
        )
            explode_touch_common(self, attacked, explodeinfo);
    }
}

static void explode_common_modelaction_startframef(entity_t * self, unsigned int imodel)
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

static void explode_common_modelaction_endframef(entity_t * self, unsigned int imodel)
{
    entity_t * owner;
    if((owner = self->owner))
    {
        if(owner->bull == self)
        {
            owner->bull = NULL;
            /* вернуть камеру игроку */
            entity_cam_reset(owner);
        }
    }
    gi->unlink(self);
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

entity_t * explode_artillery_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("explode_artillery", parent);

    self->c.bodybox = 14.0f;
    entity_model_set(self, 0, ":/explode_small", 14.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &explode_small_fseq_explode);
    entity_model_play_start(self, 0);

    entity_vars_set_all(self, spawninfo);
    return self;
}

entity_t * explode_missile_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("explode_missile", parent);

    self->c.bodybox = 22.0f;
    entity_model_set(self, 0, ":/explode_big", 22.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &explode_big_fseq_explode);
    entity_model_play_start(self, 0);

    entity_vars_set_all(self, spawninfo);
    return self;
}

entity_t * explode_mine_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("explode_mine", parent);

    self->c.bodybox = 22.0f;
    entity_model_set(self, 0, ":/explode_big", 22.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &explode_big_fseq_explode);
    entity_model_play_start(self, 0);

    entity_vars_set_all(self, spawninfo);
    return self;
}
