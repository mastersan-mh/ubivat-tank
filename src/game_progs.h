/*
 * game_progs.h
 *
 *  Created on: 1 дек. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_GAME_PROGS_H_
#define SRC_GAME_PROGS_H_

#include "entity.h"

#define ENTITY_FUNCTION_ACTION(x) \
        void x (ENTITY self, const char * action)

typedef struct entityaction_s
{
    char * action;
    void (*action_f)(ENTITY self, const char * action);
} entity_action_t;

typedef struct
{
    char * name;
    void (*spawn)(ENTITY entity);
} spawn_t;

typedef struct
{

    size_t infos_num;
    const game_exports_entityinfo_t *infos;

    /* массив действий, допустимых для entity, при управлении игроком */
    size_t actions_num;
    const entity_action_t * actions;

    /*
     * вызывается при присоединении игрока к серверу,
     *  вызывается один раз
     *  @return true - connection accepted
     */
    bool (*client_player_connect)(const char * userinfo);
    void (*client_player_disconnect)(ENTITY entity);
    /* вызывается при появлении игрока на карте, вызывается каждый раз, после чтения карты */
    ENTITY (*client_player_spawn)(const char * userinfo);

    /* called on entity loaded from map file */
    void (*entity_on_read)(const char *classname, const char * info);

} game_exports_t;

#endif /* SRC_GAME_PROGS_H_ */
