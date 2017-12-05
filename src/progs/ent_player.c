/*
 *        Ubivat Tank
 *        обработка игрока
 *
 * by Master San
 */

#include "progs.h"
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

player_invitemtype_t player_entity_to_itemtype(const ENTITY entity)
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
    game_halt("player_entity_to_itemtype(): invalid item = %s\n", entity_classname_get(entity));
    return -1;
}

/*
 * инициализируем игрока при спавне
 */
void player_spawn_init(ENTITY player)
{
    ENTITY spawn = entity_parent(player);
    if(!spawn)
        game_halt("Error: Player spawn is NULL, can not create player.");

    player_vars_t * pvars = entity_vars(player);

    spawn_vars_t * sp = entity_vars(spawn);

    VEC2_COPY(pvars->origin, sp->origin);

    INTEGER spawn_scores = sp->item_scores;
    if(0 <= spawn_scores && spawn_scores <= PLAYER_SCORES_MAX)
        pvars->scores = spawn_scores;

    player_class_init(player, pvars);


    playerinfo_t * playerinfo = &playerinfo_table[pvars->level];

    void inventory_item_init(ENTITY player, INTEGER * playervar, INTEGER infovar, INTEGER spawn_value)
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
                if(sv_game_flag_localgame())
                    spawn_value = infovar;
                else
                {
                    //по уровням
                    if(sv_game_is_first_map()) // первая карта
                        spawn_value = infovar;
                    else // не первая карта
                        spawn_value = *playervar;
                }
            }
        }
        *playervar = spawn_value;
    }

    INTEGER player_health = pvars->item_health;
    if(player_health < 0)
    {
        player_health = sp->item_health;
        if(player_health < 0)
            player_health = playerinfo->items[ITEM_SCORES];
        pvars->item_health = player_health;
    }

    /* инициализация инвентаря */
    inventory_item_init(player, &pvars->item_health      , playerinfo->items[ITEM_HEALTH]      , sp->item_health);
    inventory_item_init(player, &pvars->item_armor       , playerinfo->items[ITEM_ARMOR]       , sp->item_armor );
    inventory_item_init(player, &pvars->item_ammo_missile, playerinfo->items[ITEM_AMMO_MISSILE], sp->item_ammo_missile);
    inventory_item_init(player, &pvars->item_ammo_mine   , playerinfo->items[ITEM_AMMO_MINE]   , sp->item_ammo_mine);

    pvars->frags = 0;
    pvars->bull         = NULL;
    pvars->move.speed   = 0;
    pvars->move.go      = false;
    pvars->attack       = false;
    pvars->reloadtime_d = 0;
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

static void tank_common_modelaction_lastframef(ENTITY entity, unsigned int imodel)
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

static void player_done(ENTITY self);
static ENTITY_FUNCTION_THINK(player_think);

static void enemy_done(ENTITY self);
static ENTITY_FUNCTION_THINK(enemy_think);

static void boss_done(ENTITY self);
static ENTITY_FUNCTION_THINK(boss_think);


/*
 * touchs
 */
static void player_touch_message(ENTITY self, ENTITY exposed)
{
    message_vars_t * msg = entity_vars(exposed);
    //отправим сообщение игроку
    sv_game_message_send(msg->text);
}

static void player_touch_exit(ENTITY self, ENTITY exposed)
{
    exit_vars_t * msg = entity_vars(exposed);
    //отправим сообщение игроку
    sv_game_message_send(msg->text);
    sv_game_win();
}

static void player_item_pickup_scores(ENTITY player, ENTITY item)
{
    player_vars_t * pl = entity_vars(player);
    INTEGER level = pl->level;
    playerinfo_t *playerinfo = &playerinfo_table[level];

    entity_unspawn(item);

    item_vars_t * it = entity_vars(item);

    INTEGER scores = ( pl->scores += it->amount );

    player_class_init(player, entity_vars(player));
    if(scores / PLAYER_SCOREPERCLASS >= 5)
    {
        if(pl->item_health < playerinfo->items[ITEM_HEALTH])
            pl->item_health = playerinfo->items[ITEM_HEALTH];
        if(pl->item_armor < playerinfo->items[ITEM_ARMOR])
            pl->item_armor = playerinfo->items[ITEM_ARMOR];
    }
}


static void player_item_pickup(ENTITY player, ENTITY item)
{
    player_vars_t * pl = entity_vars(player);
    INTEGER level = pl->level;

    item_vars_t * it = entity_vars(item);
    player_invitemtype_t invitemtype = player_entity_to_itemtype(item);

    INTEGER * amount;

    switch(invitemtype)
    {
        /* case ITEM_SCORES: amount = &pl->scores; break; */
        case ITEM_HEALTH: amount = &pl->item_health; break;
        case ITEM_ARMOR : amount = &pl->item_armor; break;
        /* case ITEM_AMMO_ARTILLERY: amount = &pl->item_ammo_artillery; break; */
        case ITEM_AMMO_MISSILE  : amount = &pl->item_ammo_missile; break;
        case ITEM_AMMO_MINE     : amount = &pl->item_ammo_mine; break;
        default: return;
    }

    playerinfo_t *playerinfo = &playerinfo_table[level];

    INTEGER itemamount = it->amount;

    INTEGER itemamount_setted;
    if(
            playerinfo->items[invitemtype] > 0 &&
            (
                    ( (itemamount_setted = *amount) ) < playerinfo->items[invitemtype] ||
                    itemamount < 0
            )
    )
    {
        entity_unspawn(item);
        itemamount_setted += itemamount;
        coerce_value_int(&itemamount_setted, 0, playerinfo->items[invitemtype]);
        *amount = itemamount_setted;
    }
}

void player_action_move(player_vars_t * pl, direction_t dir, bool go)
{

/*
    if(server.flags.allow_respawn && !pl->item_health <=0)
    {
        game_console_send("server: respawn player.");
        server_player_vars_storage_t * storage = server_storage_find(clientId, playerId);
        void * vars = storage ? storage->vars : NULL;

void entity_respawn(entity_t * entity, const void * vars)
{
    if(entity->spawned)
        return;
    entity_restore((ENTITY)entity, vars);
    const game_exports_entityinfo_t * info = entity->info;
    if(info->spawn)
        info->spawn((ENTITY)entity, NULL);
    entity->spawned = true;
    entity_vars_common_t * common = entity->vars;
    common->alive = true;
}

        entity_respawn(entity, vars);
        return;
    }
*/

    if(go)
        pl->dir = dir;
    else
    {
        if(pl->dir != dir)
            return;
    }
    pl->move.go = go;
}

void player_action_attack(player_vars_t * pl, bool attack, weapontype_t weap)
{
    if(attack)
        pl->weap = weap;
    pl->attack = attack;
}

static void player_touch(ENTITY self, ENTITY other)
{
    if(entity_classname_cmp(other, "item_scores") == 0 ){ player_item_pickup_scores(self, other); return; }
    if(entity_classname_cmp(other, "item_health") == 0 ){ player_item_pickup(self, other); return; }
    if(entity_classname_cmp(other, "item_armor" ) == 0 ){ player_item_pickup(self, other); return; }
    if(entity_classname_cmp(other, "item_ammo_missile") == 0 ){ player_item_pickup(self, other); return; }
    if(entity_classname_cmp(other, "item_ammo_mine"   ) == 0 ){ player_item_pickup(self, other); return; }
    if(entity_classname_cmp(other, "message") == 0 ){ player_touch_message(self, other); return; }
    if(entity_classname_cmp(other, "exit"   ) == 0 ){ player_touch_exit(self, other); return; }
};

static void player_think_common(ENTITY self);

ENTITY player_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("player", parent);
    if(!self)
        return NULL;

    entity_done_set(self, player_done);
    entity_thinker_set(self, player_think);
    entity_toucher_set(self, player_touch);

    entity_flags_set(self, ENTITYFLAG_SOLIDWALL);
    entity_bodybox_set(self, 16.0f);

    entity_model_set(self, 0, ":/tank1"      , 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_set(self, 1, ":/flag_player", 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &tank_fseq_run);


    player_vars_t * pl = entity_vars(self);

#if defined(_DEBUG_PLAYERMAXLEVEL)
    pl->scores      = 9000;
    pl->item_health = 100;
    pl->item_armor  = 100;
    pl->item_ammo_missile = 100;
    pl->item_ammo_mine    = 100;
#endif

    //    entity_restore(player, storage);
    /*
        spawn_vars_t * sp = entity_vars(spawn);
        player_vars_t * vars = entity_vars(player);
        VEC2_COPY(vars->origin, sp->origin);
        vars->dir = sp->dir;
        vars->scores = sp->item_scores;
        vars->item_health = sp->item_health;
        vars->item_armor = sp->item_armor;
        vars->item_ammo_missile = sp->item_ammo_missile;
        vars->item_ammo_mine = sp->item_ammo_mine;
     */


    player_spawn_init(self);// parent = spawn

    return self;
}

static void player_done(ENTITY self)
{
    sound_play_stop(self, -1);
    player_vars_t * pl = entity_vars(self);
    ctrl_AI_done(&(pl->brain));
}

ENTITY_FUNCTION_THINK(player_think)
{
    player_think_common(self);
}

ENTITY enemy_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("enemy", parent);

    entity_done_set(self, enemy_done);
    entity_thinker_set(self, enemy_think);

    entity_flags_set(self, ENTITYFLAG_SOLIDWALL);
    entity_bodybox_set(self, 16.0f);
    entity_model_set(self, 0, ":/tank1"     , 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_set(self, 1, ":/flag_enemy", 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &tank_fseq_run);

    entity_vars_set_all(self, spawninfo);

    player_spawn_init(self);
    player_vars_t * pl = entity_vars(self);
    ctrl_AI_init(&pl->brain);
    return self;
}

static void enemy_done(ENTITY self)
{
    player_done(self);
}

ENTITY_FUNCTION_THINK(enemy_think)
{
    think_enemy(self);
    player_think_common(self);
}

ENTITY boss_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("boss", parent);

    entity_done_set(self, boss_done);
    entity_thinker_set(self, boss_think);

    entity_flags_set(self, ENTITYFLAG_SOLIDWALL);
    entity_bodybox_set(self, 16.0f);
    entity_model_set(self, 0, ":/tank1"    , 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_set(self, 1, ":/flag_boss", 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(self, 0, &tank_fseq_run);

    entity_vars_set_all(self, spawninfo);

    player_spawn_init(self);
    player_vars_t * pl = entity_vars(self);
    ctrl_AI_init(&pl->brain);
    return self;
}

static void boss_done(ENTITY self)
{
    player_done(self);
}

ENTITY_FUNCTION_THINK(boss_think)
{
    think_enemy(self);
    player_think_common(self);
}

void player_handle_common_shoot(ENTITY player, int info_item, INTEGER * item)
{
    player_vars_t * pl = entity_vars(player);

    if(
            (info_item == PLAYER_ITEM_AMOUNT_INF)||
            (*item > 0)
    )
    {
        // пули не кончились
        pl->reloadtime_d = c_p_WEAP_reloadtime;
        //создаем пулю
        weaponinfo_t * weaponinfo = &weaponinfo_table[pl->weap];
        direction_t dir;
        if(pl->weap != WEAP_MINE)
            dir = pl->dir;
        else
            dir = entity_direction_invert(pl->dir);

        ENTITY bull = spawn_entity_by_class(weaponinfo->entityname, NULL, player);
        bull_vars_t * bool_vars = entity_vars(bull);
        VEC2_COPY(bool_vars->origin, pl->origin);
        bool_vars->dir = dir;
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
static void player_think_common(ENTITY self)
{

    map_t * map = world_map_get();

    player_vars_t * pl = entity_vars(self);

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

    INTEGER level = pl->level;
    playerinfo_t *playerinfo = &playerinfo_table[level];

    pl->alive = ( pl->item_health > 0 );

    if(!pl->alive)
    {
        state = STATE_DEAD;
    }
    else
    {
        if(pl->move.go)
        {
            if(!pl->move.prev_go)
                state = STATE_RUN_BEGIN;
            else
                state = STATE_RUN;
        }
        else
        {
            if(pl->move.prev_go)
                state = STATE_RUN_END;
        }
        pl->move.prev_go = pl->move.go;
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

        if(entity_is_spawned(self))
        {
            ENTITY explode = explode_missile_spawn(self, NULL);
            entity_vars_common_t * vars = entity_vars(explode);
            VEC2_COPY(vars->origin, pl->origin);
            vars->dir = pl->dir;

            entity_unspawn(self);

            if(entity_classname_cmp(self, "player") != 0)
                entity_hide(self);
            pl->item_armor = 0;
            pl->item_ammo_missile = 0;
            pl->item_ammo_mine = 0;
        };
        if(entity_classname_cmp(self, "boss") == 0)
            sv_game_win();
        break;
    }


    if(pl->alive)
    {
        //игрок жив
        if(pl->bull)
        {
            bull_vars_t * bull_vars = entity_vars(pl->bull);
            bull_vars->dir = pl->dir;
            pl->move.go = false;
        };
        if(pl->move.go)
        {
            //игрок едет
            pl->move.speed += PLAYER_ACCEL * dtime;
            if(pl->move.speed > playerinfo->speed) pl->move.speed = playerinfo->speed;
            if(!sound_started(self, PLAYER_SOUND_MOVE))
            {
                sound_play_start(self, PLAYER_SOUND_MOVE, SOUND_PLAYER_TANKMOVE, -1);
            }

        }
        else
        {
            //игрок останавливается
            pl->move.speed -= PLAYER_DECEL * dtime;
        };
        if(pl->move.speed < 0)
        {
            sound_play_stop(self, PLAYER_SOUND_MOVE);
            pl->move.speed = 0;
        }
        entity_move(self, pl->dir, pl->move.speed, true);

        vec_t speed_s = playerinfo->speed / 4;

        vec_t halfbox = entity_bodybox_get(self) * 0.5;
        vec_t quarterbox = halfbox * 0.5f;
        //стрейф
        switch(pl->dir)
        {
        case DIR_UP:
        case DIR_DOWN:
            VEC2_COPY(Sorig, pl->origin);
            Sorig[0] -= quarterbox;
            map_clip_find_near(map, Sorig, halfbox, pl->dir, MAP_WALL_CLIP, halfbox + 2, &L);
            VEC2_COPY(Sorig, pl->origin);
            Sorig[0] += quarterbox;
            map_clip_find_near(map, Sorig, halfbox, pl->dir, MAP_WALL_CLIP, halfbox + 2, &R);
            if((halfbox<L) && (R-1<=halfbox)) entity_move(self, DIR_LEFT, speed_s, true);//strafe left
            if((halfbox<R) && (L-1<=halfbox)) entity_move(self, DIR_RIGHT, speed_s, true);//strafe right
            break;
        case DIR_LEFT:
        case DIR_RIGHT:
            VEC2_COPY(Sorig, pl->origin);
            Sorig[1] -= quarterbox;
            map_clip_find_near(map, Sorig, halfbox, pl->dir, MAP_WALL_CLIP, halfbox + 2, &D);
            VEC2_COPY(Sorig, pl->origin);
            Sorig[1] += quarterbox;
            map_clip_find_near(map, Sorig, halfbox, pl->dir, MAP_WALL_CLIP, halfbox + 2, &U);
            if((halfbox < U)&&(D-1 <= halfbox)) entity_move(self, DIR_UP  , speed_s, true);//strafe up
            if((halfbox < D)&&(U-1 <= halfbox)) entity_move(self, DIR_DOWN, speed_s, true);//strafe down
            break;
        }
    }
    //стрельба
    if(!pl->attack)
    {
        //игрок не атакует
        if(pl->reloadtime_d>0) pl->reloadtime_d -= dtime;//учитываем время на перезарядку
    }
    else
    {
        //игрок атакует
        if(pl->alive)
        {
            if(pl->reloadtime_d > 0) pl->reloadtime_d -= dtime;//учитываем время на перезарядку
            else
            {
                if(!pl->bull)
                {

                    switch(pl->weap)
                    {
                    case WEAP_ARTILLERY:
                        player_handle_common_shoot(self, playerinfo->items[ITEM_AMMO_ARTILLERY], &pl->item_ammo_artillery);
                        break;
                    case WEAP_MISSILE  :
                        player_handle_common_shoot(self, playerinfo->items[ITEM_AMMO_MISSILE], &pl->item_ammo_missile  );
                        break;
                    case WEAP_MINE     :
                        player_handle_common_shoot(self, playerinfo->items[ITEM_AMMO_MINE], &pl->item_ammo_mine);
                        break;
                    default: ;
                    }

                }
            }
        }
    }
    if(pl->reloadtime_d < 0) pl->reloadtime_d = 0;
}

/*
 * инициализация класса танка
 */
void player_class_init_item_amount(ENTITY player, INTEGER * item_amount_dest, INTEGER item_amount_src)
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

void player_class_init(ENTITY player, player_vars_t * pl)
{

    int level = pl->scores / PLAYER_SCOREPERCLASS;
    if(level > PLAYER_LEVEL5) level = PLAYER_LEVEL5;
    if( entity_classname_cmp(player, "boss") == 0 )
        level = PLAYER_LEVEL_BOSS;

    pl->level = level;

    playerinfo_t * playerinfo = &playerinfo_table[level];

    player_class_init_item_amount(player, &pl->scores     , playerinfo->items[ITEM_SCORES]);
    player_class_init_item_amount(player, &pl->item_health, playerinfo->items[ITEM_HEALTH]);
    player_class_init_item_amount(player, &pl->item_armor , playerinfo->items[ITEM_ARMOR]);
    player_class_init_item_amount(player, &pl->item_ammo_artillery, playerinfo->items[ITEM_AMMO_ARTILLERY]);
    player_class_init_item_amount(player, &pl->item_ammo_missile  , playerinfo->items[ITEM_AMMO_MISSILE]);
    player_class_init_item_amount(player, &pl->item_ammo_mine     , playerinfo->items[ITEM_AMMO_MINE]);

    entity_model_set(player, 0, playerinfo->modelname, 16.0f / 2.0f, 0.0f, 0.0f);
    entity_model_sequence_set(player, 0, &tank_fseq_run);

}

/*
 * вычисление повреждения наносимого игроку
 */
void player_getdamage(ENTITY player, ENTITY explode, bool self, vec_t distance, const explodeinfo_t * explodeinfo)
{
    player_vars_t * pl = entity_vars(player);
    int damage_full;
    int armor;

    vec_t radius = entity_bodybox_get(explode) * 0.5f;

    //weapon_info_t * weapinfo = &wtable[explode->explode.type];

    if( pl->item_health > 0 && distance <= radius )
    {
        if(self)
            damage_full = explodeinfo->selfdamage * (1-distance/radius);
        else
            damage_full = explodeinfo->damage     * (1-distance/radius);

        int damage_armor = damage_full*2/3;
        int damage_health = damage_full - damage_armor;

        armor = pl->item_armor - damage_armor;
        pl->item_health -= damage_health;
        if(armor < 0)
        {
            pl->item_health += armor;
            pl->item_armor = 0;
        }
        else
            pl->item_armor = armor;
        if(pl->item_health <= 0)
        {
            ENTITY killer = entity_parent(explode);
            if(killer)
            {
                player_vars_t * pl = entity_vars(killer);
                if(!self)
                {
                    //атакующему добавим очки
                    pl->fragstotal++;
                    pl->frags++;
                    pl->scores += PLAYER_SCORES_PER_ENEMY;
                }
                else
                {
                    //атакующий умер от своей пули
                    pl->fragstotal--;
                    pl->frags--;
                    pl->scores = 0;
                }
                player_class_init(killer, pl);
            }
        }
    }
}


/****************/

/*
 * вывод информации об игроке
 */
void player_ui_draw(camera_t * cam, ENTITY player)
{
    if(!world_valid())
        return;
    player_vars_t * pl = entity_vars(player);
    static image_index_t list[] =
    {
            IMG_HUD_ICON_TANK0,
            IMG_HUD_ICON_TANK1,
            IMG_HUD_ICON_TANK2,
            IMG_HUD_ICON_TANK3,
            IMG_HUD_ICON_TANK4,
    };

    INTEGER level = pl->level;
    playerinfo_t *playerinfo = &playerinfo_table[level];

    int ref_y = VIDEO_SCREEN_H - 16 * 2;

    font_color_set3i(COLOR_1);
    // gr2D_settext(cam->x,cam_y,0,'('+realtostr(player->move.orig.x,8,4)+';'+realtostr(player->move.orig.y,8,4)+')');
    // gr2D_settext(cam->x,cam_y,0,"PING %d", player->time.delta);

    ui_drawimage(cam, 16 * 0     , ref_y, IMG_ITEM_HEALTH);
    ui_drawimage(cam, 16 * 6     , ref_y, IMG_ITEM_ARMOR);

    ui_printf(cam, 16 * 0 + 16, ref_y, "%ld", pl->item_health);
    ui_printf(cam, 16 * 6 + 16, ref_y, "%ld", pl->item_armor);
    ui_printf(cam, 16 * 0 + 16, ref_y + 8, "%d", playerinfo->items[ITEM_HEALTH]);
    ui_printf(cam, 16 * 6 + 16, ref_y + 8, "%d", playerinfo->items[ITEM_ARMOR]);

    /* вторая строка */
    ref_y += 16;
    ui_drawimage(cam, 16 * 0, ref_y, list[level]);
    ui_drawimage(cam, 16 * 4, ref_y, weaponinfo_table[0].icon);
    ui_drawimage(cam, 16 * 6, ref_y, weaponinfo_table[1].icon);
    ui_drawimage(cam, 16 * 8, ref_y, weaponinfo_table[2].icon);

    ui_printf(cam, 16 * 4 + 16, ref_y + 4, "@"); // player->items[ITEM_AMMO_ARTILLERY]
    if(pl->item_ammo_missile >= 0)
        ui_printf(cam, 16 * 6 + 16, ref_y + 4, "%ld", pl->item_ammo_missile);
    if(pl->item_ammo_mine >= 0)
        ui_printf(cam, 16 * 8 + 16, ref_y + 4, "%ld", pl->item_ammo_mine);
    ui_printf(cam, 16 * 0 + 16, ref_y +  4, "%ld", pl->scores);

}
