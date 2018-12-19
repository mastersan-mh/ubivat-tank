/*
 * game_progs.h
 *
 *  Created on: 1 дек. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_GAME_PROGS_H_
#define SRC_GAME_PROGS_H_

#include "vars.h"

/* #include "entity.h" */
typedef struct entity_common_s entity_common_t;

#define GAME_EXPORTS_ACTIONS(xactions) \
        .actions_num = ARRAYSIZE(xactions), \
        .actions = xactions

#define GAME_EXPORTS_VARS_DESCR(xvarstype, xvars) \
        .vars_size = sizeof(xvarstype), \
        .vars_descr_num = ARRAYSIZE(xvars), \
        .vars_descr = xvars \

#define ENTITY_FUNCTION_ACTION(x) \
        void x (entity_common_t * self, const char * action)

#define ENTITY_VARS_COMMON \
        VAR_DESCR( VARTYPE_BOOL     , entity_common_t, alive  ), \
        VAR_DESCR( VARTYPE_VECTOR2  , entity_common_t, origin_prev), \
        VAR_DESCR( VARTYPE_VECTOR2  , entity_common_t, origin     ), \
        VAR_DESCR( VARTYPE_DIRECTION, entity_common_t, dir        )


typedef struct entityaction_s
{
    char * action;
    void (*action_f)(entity_common_t * self, const char * action);
} entity_action_t;

typedef struct
{
    entity_common_t *target; /* entity target. game_client_t valid if != NULL */
    /* объект, за которым следит камера данного объекта, обычно равен self */
    entity_common_t * cam_target;

    ssize_t iclient; /* номер клиента на серере */

    INTEGER frags;
    INTEGER scores;

} game_client_t;

typedef struct
{
    size_t maxclients;
    game_client_t * clients;
    size_t maxentities;
} game_local_t;

typedef struct
{
    size_t clients_max;
    size_t entities_max;
} game_inits_t;

typedef struct
{

    void (*eprint)(const char *format, ...); /* error print and halt! */
    void (*cprint)(const char *format, ...); /* console print */
    void (*centerprint)(const char *format, ...);

    void (*link)(void * entity);
    void (*unlink)(void * entity);

    /* deprecated */
    void (*vars_descr_get)(const var_descr_t ** vars_descr, size_t * vars_descr_num);
    long (*dtime)(void);
    double (*dtimed1000)(void);
    void (*sv_game_win)(void);
    int (*sv_game_flag_localgame)(void);
    int (*sv_game_is_first_map)(void);
} game_imports_t;

typedef struct
{
    /* массив действий, допустимых для entity, при управлении игроком */
    size_t actions_num;
    const entity_action_t * actions;

    /* размер буфера переменных */
    size_t vars_size;
    /* массив описателей переменных */
    size_t vars_descr_num;
    const var_descr_t * vars_descr;

    void (*init)(const game_inits_t * init);
    void (*done)(void);

    /* called on entity loaded from map file */
    void (*world_create)(size_t entities_num);
    void (*world_destroy)(void);
    void (*world_handle)(void);

    /* called on entity loaded from map file */
    void (*entity_on_read)(const char *classname, const char * info);

    /*
     * вызывается при присоединении игрока к серверу,
     *  вызывается один раз
     *  @return true - connection accepted
     */
    bool (*client_player_connect)(const char * userinfo);
    void (*client_player_disconnect)(entity_common_t * entity);
    /* вызывается при появлении игрока на карте, вызывается каждый раз, после чтения карты */
    entity_common_t * (*client_player_spawn)(const char * userinfo);

} game_exports_t;

#endif /* SRC_GAME_PROGS_H_ */
