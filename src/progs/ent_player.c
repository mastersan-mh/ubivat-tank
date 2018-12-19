/*
 *        Ubivat Tank
 *        обработка игрока
 *
 * by Master San
 */

#include "progs.h"
#include "helpers.h"
#include "progs_main.h"

#include "ent_player.h"
#include "ent_player_think.h"
#include "ent_spawn.h"
#include "ent_message.h"
#include "ent_exit.h"
#include "ent_items.h"
#include "ent_bull.h"
#include "ent_explode.h"
#include "ent_message.h"
#include "ent_weap.h"

#define PLAYER_SOUND_MOVE 0
#define PLAYER_SOUND_ATTACK 1

player_invitemtype_t player_entity_to_itemtype(const entity_t * entity)
{
    static const char *entityclassnames[] =
    {
            "item_scores",
            "item_health",
            "item_armor" ,
            "item_ammo_artillery",
            "item_ammo_missile",
            "item_ammo_mine"
    };
    size_t i;
    for(i = 0; i < ARRAYSIZE(entityclassnames); i++)
    {
        if(entity_classname_cmp(entity, entityclassnames[i]) == 0)
            return i;
    }
    gi->eprint("player_entity_to_itemtype(): invalid item = %s\n", entity->classname);
    return -1;
}

/*
 * инициализируем игрока при спавне
 */
void player_spawn_init(entity_t * player)
{
    entity_t * spawn = player->owner;
    if(!spawn)
        gi->eprint("Error: Player spawn is NULL, can not create player.");

    VEC2_COPY(player->c.origin, spawn->c.origin);

    INTEGER spawn_scores = spawn->item_scores;
    if(0 <= spawn_scores && spawn_scores <= PLAYER_SCORES_MAX)
        player->scores = spawn_scores;

    player_class_init(player);


    playerinfo_t * playerinfo = &playerinfo_table[player->level];

    void inventory_item_init(entity_t * player, INTEGER * playervar, INTEGER infovar, INTEGER spawn_value)
    {
        if( spawn_value > infovar )
            spawn_value = infovar;
        else if(spawn_value <=0)
        {
            if( entity_classname_cmp(player, "player") != 0 ) /* враг/босс */
                spawn_value = infovar;
            else
            {
                /* игрок-человек */
                if(gi->sv_game_flag_localgame())
                    spawn_value = infovar;
                else
                {
                    //по уровням
                    if(gi->sv_game_is_first_map()) // первая карта
                        spawn_value = infovar;
                    else // не первая карта
                        spawn_value = *playervar;
                }
            }
        }
        *playervar = spawn_value;
    }

    INTEGER player_health = player->item_health;
    if(player_health < 0)
    {
        player_health = spawn->item_health;
        if(player_health < 0)
            player_health = playerinfo->items[ITEM_SCORES];
        player->item_health = player_health;
    }

    /* инициализация инвентаря */
    inventory_item_init(player, &player->item_health      , playerinfo->items[ITEM_HEALTH]      , spawn->item_health);
    inventory_item_init(player, &player->item_armor       , playerinfo->items[ITEM_ARMOR]       , spawn->item_armor );
    inventory_item_init(player, &player->item_ammo_missile, playerinfo->items[ITEM_AMMO_MISSILE], spawn->item_ammo_missile);
    inventory_item_init(player, &player->item_ammo_mine   , playerinfo->items[ITEM_AMMO_MINE]   , spawn->item_ammo_mine);

    player->frags = 0;
    player->bull         = NULL;
    player->move.speed   = 0;
    player->move.go      = false;
    player->attack       = false;
    player->reloadtime_d = 0;
};


playerinfo_t playerinfo_table[__PLAYER_LEVEL_NUM] =
{
        /* SCORES                 , HEALTH, ARMOR,  AMMO_ARTILLERY,   AMMO_MISSILE,     AMMO_MINE,     speed,            modelname */
        { { PLAYER_SCOREPERCLASS * 1,    100,     0, PLAYER_ITEM_AMOUNT_INF, PLAYER_ITEM_AMOUNT_NA, PLAYER_ITEM_AMOUNT_NA }, 40/2 * SPEEDSCALE, ":/tank1"},
        { { PLAYER_SCOREPERCLASS * 2,    100,    50, PLAYER_ITEM_AMOUNT_INF, PLAYER_ITEM_AMOUNT_NA, PLAYER_ITEM_AMOUNT_NA }, 50/2 * SPEEDSCALE, ":/tank2"},
        { { PLAYER_SCOREPERCLASS * 3,    100,   100, PLAYER_ITEM_AMOUNT_INF, 50            , PLAYER_ITEM_AMOUNT_NA }, 60/2 * SPEEDSCALE, ":/tank3"},
        { { PLAYER_SCOREPERCLASS * 4,    200,   150, PLAYER_ITEM_AMOUNT_INF, 50            , PLAYER_ITEM_AMOUNT_NA }, 70/2 * SPEEDSCALE, ":/tank4"},
        { { PLAYER_SCOREPERCLASS * 5,    200,   200, PLAYER_ITEM_AMOUNT_INF, 50            , 50             }, 90/2 * SPEEDSCALE, ":/tank5"},
        { { PLAYER_SCOREPERCLASS * 6,   5000,  5000, PLAYER_ITEM_AMOUNT_INF, 50            , 50             }, 90/2 * SPEEDSCALE, ":/tank5"}  /* BOSS */
};

static void tank_common_modelaction_lastframef(entity_t * entity, unsigned int imodel)
{
    entity_model_play_start(entity, imodel);
}
static const entity_framessequence_t tank_fseq_run =
{
        .firstframe = 0,
        .firstframef = NULL,
        .lastframe = 3,
        .lastframef = tank_common_modelaction_lastframef
};

static void player_think(entity_t * self);

static void enemy_think(entity_t * self);

static void boss_think(entity_t * self);


/*
 * touchs
 */
static void player_touch_message(entity_t * self, entity_t * other)
{
    //отправим сообщение игроку
    gi->centerprint(other->text);
}

static void player_touch_exit(entity_t * self, entity_t * other)
{
    gi->centerprint(other->text);
    gi->sv_game_win();
}

static void player_item_pickup_scores(entity_t * player, entity_t * item)
{
    INTEGER level = player->level;
    playerinfo_t *playerinfo = &playerinfo_table[level];

    gi->unlink(item);
    entity_done(item);

    INTEGER scores = ( player->scores += item->amount );

    player_class_init(player);
    if(scores / PLAYER_SCOREPERCLASS >= 5)
    {
        if(player->item_health < playerinfo->items[ITEM_HEALTH])
            player->item_health = playerinfo->items[ITEM_HEALTH];
        if(player->item_armor < playerinfo->items[ITEM_ARMOR])
            player->item_armor = playerinfo->items[ITEM_ARMOR];
    }
}


static void player_item_pickup(entity_t * player, entity_t * item)
{
    INTEGER level = player->level;

    player_invitemtype_t invitemtype = player_entity_to_itemtype(item);

    INTEGER * amount;

    switch(invitemtype)
    {
        /* case ITEM_SCORES: amount = &player->scores; break; */
        case ITEM_HEALTH: amount = &player->item_health; break;
        case ITEM_ARMOR : amount = &player->item_armor; break;
        /* case ITEM_AMMO_ARTILLERY: amount = &player->item_ammo_artillery; break; */
        case ITEM_AMMO_MISSILE  : amount = &player->item_ammo_missile; break;
        case ITEM_AMMO_MINE     : amount = &player->item_ammo_mine; break;
        default: return;
    }

    playerinfo_t *playerinfo = &playerinfo_table[level];

    INTEGER itemamount = item->amount;

    INTEGER itemamount_setted;
    if(
            playerinfo->items[invitemtype] > 0 &&
            (
                    ( (itemamount_setted = *amount) ) < playerinfo->items[invitemtype] ||
                    itemamount < 0
            )
    )
    {
        entity_done(item);
        gi->unlink(item);
        itemamount_setted += itemamount;
        coerce_value_int(&itemamount_setted, 0, playerinfo->items[invitemtype]);
        *amount = itemamount_setted;
    }
}

void player_action_move(entity_t * player, direction_t dir, bool go)
{

/*
    if(server.flags.allow_respawn && !player->item_health <=0)
    {
        game_cprint("server: respawn player.");
        server_player_vars_storage_t * storage = server_storage_find(clientId, playerId);
        void * vars = storage ? storage->vars : NULL;

void entity_respawn(entity_t * entity, const void * vars)
{
    if(entity->spawned)
        return;
    entity_restore((entity_t *)entity, vars);
    const game_exports_entityinfo_t * info = entity->info;
    if(info->spawn)
        info->spawn((entity_t *)entity, NULL);
    entity->spawned = true;
    entity_vars_common_t * common = entity->vars;
    common->c.alive = true;
}

        entity_respawn(entity, vars);
        return;
    }
*/

    if(go)
        player->c.dir = dir;
    else
    {
        if(player->c.dir != dir)
            return;
    }
    player->move.go = go;
}

void player_action_attack(entity_t * player, bool attack, weapontype_t weap)
{
    if(attack)
        player->weap = weap;
    player->attack = attack;
}

static void player_touch(entity_t * self, entity_t * other)
{
    if(entity_classname_cmp(other, "item_scores") == 0 ){ player_item_pickup_scores(self, other); return; }
    if(entity_classname_cmp(other, "item_health") == 0 ){ player_item_pickup(self, other); return; }
    if(entity_classname_cmp(other, "item_armor" ) == 0 ){ player_item_pickup(self, other); return; }
    if(entity_classname_cmp(other, "item_ammo_missile") == 0 ){ player_item_pickup(self, other); return; }
    if(entity_classname_cmp(other, "item_ammo_mine"   ) == 0 ){ player_item_pickup(self, other); return; }
    if(entity_classname_cmp(other, "message") == 0 ){ player_touch_message(self, other); return; }
    if(entity_classname_cmp(other, "exit"   ) == 0 ){ player_touch_exit(self, other); return; }
};

static void player_think_common(entity_t * self);

entity_t * player_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("player", parent);
    if(!self)
        return NULL;

    self->c.flags =  ENTITYFLAG_SOLIDWALL;
    self->c.bodybox = 16.0f;

    self->think = player_think;
    self->touch = player_touch;

    entity_model_set(self, 0, ":/tank1"      , 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_set(self, 1, ":/flag_player", 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &tank_fseq_run);


#if defined(_DEBUG_PLAYERMAXLEVEL)
    self->scores      = 9000;
    self->item_health = 100;
    self->item_armor  = 100;
    self->item_ammo_missile = 100;
    self->item_ammo_mine    = 100;
#endif

    /*
        entity_restore(player, storage);
        spawn_vars_t * sp = entity_vars(spawn);
        player_vars_t * vars = entity_vars(player);
        VEC2_COPY(vars->c.origin, sp->c.origin);
        vars->c.dir = sp->c.dir;
        vars->scores = sp->item_scores;
        vars->item_health = sp->item_health;
        vars->item_armor = sp->item_armor;
        vars->item_ammo_missile = sp->item_ammo_missile;
        vars->item_ammo_mine = sp->item_ammo_mine;
     */


    player_spawn_init(self);// parent = spawn

    gi->link(self);

    return self;
}

void player_think(entity_t * self)
{
    player_think_common(self);
}

entity_t * enemy_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("enemy", parent);

    self->c.flags =  ENTITYFLAG_SOLIDWALL;
    self->c.bodybox = 16.0f;

    self->think = enemy_think;

    entity_model_set(self, 0, ":/tank1"     , 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_set(self, 1, ":/flag_enemy", 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &tank_fseq_run);

    entity_vars_set_all(self, spawninfo);

    player_spawn_init(self);
    ctrl_AI_init(&self->brain);
    return self;
}

void enemy_think(entity_t * self)
{
    think_enemy(self);
    player_think_common(self);
}

entity_t * boss_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("boss", parent);

    self->c.flags =  ENTITYFLAG_SOLIDWALL;
    self->c.bodybox = 16.0f;

    self->think = boss_think;

    entity_model_set(self, 0, ":/tank1"    , 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_set(self, 1, ":/flag_boss", 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &tank_fseq_run);

    entity_vars_set_all(self, spawninfo);

    player_spawn_init(self);
    ctrl_AI_init(&self->brain);
    return self;
}

void boss_think(entity_t * self)
{
    think_enemy(self);
    player_think_common(self);
}

void player_handle_common_shoot(entity_t * player, int info_item, INTEGER * item)
{
    if(
            (info_item == PLAYER_ITEM_AMOUNT_INF)||
            (*item > 0)
    )
    {
        // пули не кончились
        player->reloadtime_d = c_p_WEAP_reloadtime;
        //создаем пулю
        weaponinfo_t * weaponinfo = &weaponinfo_table[player->weap];
        direction_t dir;
        if(player->weap != WEAP_MINE)
            dir = player->c.dir;
        else
            dir = entity_direction_invert(player->c.dir);

        entity_t * bull = spawn_entity_by_class(weaponinfo->entityname, NULL, player);
        VEC2_COPY(bull->c.origin, player->c.origin);
        bull->c.dir = dir;
        sound_play_start(player, PLAYER_SOUND_ATTACK, weaponinfo->sound_index, 1);

        if(
                info_item > 0 && //если пули у оружия не бесконечны и
                entity_classname_cmp(player, "player") == 0   // игрок не монстр(у монстров пули не кончаются)
        )
            (*item)--;
    }
}

/*
 * обработка игрока
 */
static void player_think_common(entity_t * self)
{

    map_t * map = world_map_get();

    enum
    {
        STATE_IDLE,
        STATE_RUN_BEGIN,
        STATE_RUN_END,
        STATE_RUN,
        STATE_DEAD
    };
    int state = STATE_IDLE;
    vec2_t Sorig;
    vec_t L,R,U,D;

    INTEGER level = self->level;
    playerinfo_t *playerinfo = &playerinfo_table[level];

    self->c.alive = ( self->item_health > 0 );

    if(!self->c.alive)
    {
        state = STATE_DEAD;
    }
    else
    {
        if(self->move.go)
        {
            if(!self->move.prev_go)
                state = STATE_RUN_BEGIN;
            else
                state = STATE_RUN;
        }
        else
        {
            if(self->move.prev_go)
                state = STATE_RUN_END;
        }
        self->move.prev_go = self->move.go;
    }

    switch(state)
    {
    case STATE_IDLE: break;
    case STATE_RUN_BEGIN: entity_model_play_start(self, 0); break;
    case STATE_RUN_END  : entity_model_play_pause(self, 0); break;
    case STATE_RUN: break;
    case STATE_DEAD:
        //если игрок мертв

        sound_play_stop(self, -1);

        if(self->spawned)
        {
            entity_t * explode = explode_missile_spawn(self, NULL);
            VEC2_COPY(explode->c.origin, self->c.origin);
            explode->c.dir = self->c.dir;

            self->spawned = false;

            if(entity_classname_cmp(self, "player") != 0)
                gi->unlink(self);
            self->item_armor = 0;
            self->item_ammo_missile = 0;
            self->item_ammo_mine = 0;
        };
        if(entity_classname_cmp(self, "boss") == 0)
            gi->sv_game_win();
        break;
    }


    if(self->c.alive)
    {
        //игрок жив
        if(self->bull)
        {
            entity_t * bull = self->bull;
            bull->c.dir = self->c.dir;
            self->move.go = false;
        };
        if(self->move.go)
        {
            //игрок едет
            self->move.speed += PLAYER_ACCEL * gi->dtime();
            if(self->move.speed > playerinfo->speed) self->move.speed = playerinfo->speed;
            if(!sound_started(self, PLAYER_SOUND_MOVE))
            {
                sound_play_start(self, PLAYER_SOUND_MOVE, SOUND_PLAYER_TANKMOVE, -1);
            }

        }
        else
        {
            //игрок останавливается
            self->move.speed -= PLAYER_DECEL * gi->dtime();
        };
        if(self->move.speed < 0)
        {
            sound_play_stop(self, PLAYER_SOUND_MOVE);
            self->move.speed = 0;
        }
        entity_move(self, self->c.dir, self->move.speed, true);

        vec_t speed_s = playerinfo->speed / 4;

        vec_t halfbox = self->c.bodybox * 0.5;
        vec_t quarterbox = halfbox * 0.5f;
        //стрейф
        switch(self->c.dir)
        {
        case DIR_UP:
        case DIR_DOWN:
            VEC2_COPY(Sorig, self->c.origin);
            Sorig[0] -= quarterbox;
            map_clip_find_near(map, Sorig, halfbox, self->c.dir, MAP_WALL_CLIP, halfbox + 2, &L);
            VEC2_COPY(Sorig, self->c.origin);
            Sorig[0] += quarterbox;
            map_clip_find_near(map, Sorig, halfbox, self->c.dir, MAP_WALL_CLIP, halfbox + 2, &R);
            if((halfbox<L) && (R-1<=halfbox)) entity_move(self, DIR_LEFT, speed_s, true);//strafe left
            if((halfbox<R) && (L-1<=halfbox)) entity_move(self, DIR_RIGHT, speed_s, true);//strafe right
            break;
        case DIR_LEFT:
        case DIR_RIGHT:
            VEC2_COPY(Sorig, self->c.origin);
            Sorig[1] -= quarterbox;
            map_clip_find_near(map, Sorig, halfbox, self->c.dir, MAP_WALL_CLIP, halfbox + 2, &D);
            VEC2_COPY(Sorig, self->c.origin);
            Sorig[1] += quarterbox;
            map_clip_find_near(map, Sorig, halfbox, self->c.dir, MAP_WALL_CLIP, halfbox + 2, &U);
            if((halfbox < U)&&(D-1 <= halfbox)) entity_move(self, DIR_UP  , speed_s, true);//strafe up
            if((halfbox < D)&&(U-1 <= halfbox)) entity_move(self, DIR_DOWN, speed_s, true);//strafe down
            break;
        }
    }
    //стрельба
    if(!self->attack)
    {
        //игрок не атакует
        if(self->reloadtime_d>0) self->reloadtime_d -= gi->dtime();//учитываем время на перезарядку
    }
    else
    {
        //игрок атакует
        if(self->c.alive)
        {
            if(self->reloadtime_d > 0) self->reloadtime_d -= gi->dtime();//учитываем время на перезарядку
            else
            {
                if(!self->bull)
                {

                    switch(self->weap)
                    {
                    case WEAP_ARTILLERY:
                        player_handle_common_shoot(self, playerinfo->items[ITEM_AMMO_ARTILLERY], &self->item_ammo_artillery);
                        break;
                    case WEAP_MISSILE  :
                        player_handle_common_shoot(self, playerinfo->items[ITEM_AMMO_MISSILE], &self->item_ammo_missile  );
                        break;
                    case WEAP_MINE     :
                        player_handle_common_shoot(self, playerinfo->items[ITEM_AMMO_MINE], &self->item_ammo_mine);
                        break;
                    default: ;
                    }

                }
            }
        }
    }
    if(self->reloadtime_d < 0) self->reloadtime_d = 0;
}

/*
 * инициализация класса танка
 */
void player_class_init_item_amount(entity_t * player, INTEGER * item_amount_dest, INTEGER item_amount_src)
{
    if(
            item_amount_src == PLAYER_ITEM_AMOUNT_NA ||
            item_amount_src == PLAYER_ITEM_AMOUNT_INF
    )
    {
        *item_amount_dest = item_amount_src;
        return;
    }
    if( item_amount_src < 0 )
        *item_amount_dest = 0;
    if( entity_classname_cmp(player, "player") != 0 )
        *item_amount_dest = item_amount_src;
}

void player_class_init(entity_t * player)
{

    int level = player->scores / PLAYER_SCOREPERCLASS;
    if(level > PLAYER_LEVEL5) level = PLAYER_LEVEL5;
    if( entity_classname_cmp(player, "boss") == 0 )
        level = PLAYER_LEVEL_BOSS;

    player->level = level;

    playerinfo_t * playerinfo = &playerinfo_table[level];

    player_class_init_item_amount(player, &player->scores     , playerinfo->items[ITEM_SCORES]);
    player_class_init_item_amount(player, &player->item_health, playerinfo->items[ITEM_HEALTH]);
    player_class_init_item_amount(player, &player->item_armor , playerinfo->items[ITEM_ARMOR]);
    player_class_init_item_amount(player, &player->item_ammo_artillery, playerinfo->items[ITEM_AMMO_ARTILLERY]);
    player_class_init_item_amount(player, &player->item_ammo_missile  , playerinfo->items[ITEM_AMMO_MISSILE]);
    player_class_init_item_amount(player, &player->item_ammo_mine     , playerinfo->items[ITEM_AMMO_MINE]);

    entity_model_set(player, 0, playerinfo->modelname, 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(player, 0, &tank_fseq_run);

}

/*
 * вычисление повреждения наносимого игроку
 */
void player_getdamage(entity_t * player, entity_t * explode, bool self, vec_t distance, const explodeinfo_t * explodeinfo)
{
    int damage_full;
    int armor;

    vec_t radius = explode->c.bodybox * 0.5f;

    //weapon_info_t * weapinfo = &wtable[explode->explode.type];

    if( player->item_health > 0 && distance <= radius )
    {
        if(self)
            damage_full = explodeinfo->selfdamage * (1-distance/radius);
        else
            damage_full = explodeinfo->damage     * (1-distance/radius);

        int damage_armor = damage_full*2/3;
        int damage_health = damage_full - damage_armor;

        armor = player->item_armor - damage_armor;
        player->item_health -= damage_health;
        if(armor < 0)
        {
            player->item_health += armor;
            player->item_armor = 0;
        }
        else
            player->item_armor = armor;
        if(player->item_health <= 0)
        {
            entity_t * killer = explode->owner;
            if(killer)
            {
                if(!self)
                {
                    //атакующему добавим очки
                    killer->fragstotal++;
                    killer->frags++;
                    killer->scores += PLAYER_SCORES_PER_ENEMY;
                }
                else
                {
                    //атакующий умер от своей пули
                    killer->fragstotal--;
                    killer->frags--;
                    killer->scores = 0;
                }
                player_class_init(killer);
            }
        }
    }
}


/****************/

/*
 * вывод информации об игроке
 */
void player_ui_draw(camera_t * cam, entity_common_t * entity)
{
    if(!world_valid())
        return;
    static image_index_t list[] =
    {
            IMG_HUD_ICON_TANK0,
            IMG_HUD_ICON_TANK1,
            IMG_HUD_ICON_TANK2,
            IMG_HUD_ICON_TANK3,
            IMG_HUD_ICON_TANK4,
    };

    entity_t * player = (entity_t*)entity;
    INTEGER level = player->level;
    playerinfo_t *playerinfo = &playerinfo_table[level];

    int ref_y = VIDEO_SCREEN_H - 16 * 2;

    font_color_set3i(COLOR_1);
    // gr2D_settext(cam->x,cam_y,0,'('+realtostr(player->move.orig.x,8,4)+';'+realtostr(player->move.orig.y,8,4)+')');
    // gr2D_settext(cam->x,cam_y,0,"PING %d", player->time.delta);

    ui_drawimage(cam, 16 * 0     , ref_y, IMG_ITEM_HEALTH);
    ui_drawimage(cam, 16 * 6     , ref_y, IMG_ITEM_ARMOR);

    ui_printf(cam, 16 * 0 + 16, ref_y, "%ld", player->item_health);
    ui_printf(cam, 16 * 6 + 16, ref_y, "%ld", player->item_armor);
    ui_printf(cam, 16 * 0 + 16, ref_y + 8, "%d", playerinfo->items[ITEM_HEALTH]);
    ui_printf(cam, 16 * 6 + 16, ref_y + 8, "%d", playerinfo->items[ITEM_ARMOR]);

    /* вторая строка */
    ref_y += 16;
    ui_drawimage(cam, 16 * 0, ref_y, list[level]);
    ui_drawimage(cam, 16 * 4, ref_y, weaponinfo_table[0].icon);
    ui_drawimage(cam, 16 * 6, ref_y, weaponinfo_table[1].icon);
    ui_drawimage(cam, 16 * 8, ref_y, weaponinfo_table[2].icon);

    ui_printf(cam, 16 * 4 + 16, ref_y + 4, "@"); // player->items[ITEM_AMMO_ARTILLERY]
    if(player->item_ammo_missile >= 0)
        ui_printf(cam, 16 * 6 + 16, ref_y + 4, "%ld", player->item_ammo_missile);
    if(player->item_ammo_mine >= 0)
        ui_printf(cam, 16 * 8 + 16, ref_y + 4, "%ld", player->item_ammo_mine);
    ui_printf(cam, 16 * 0 + 16, ref_y +  4, "%ld", player->scores);

}
