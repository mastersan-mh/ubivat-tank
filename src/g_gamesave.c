/*
 * g_record.c
 *
 *  Created on: 20 янв. 2017 г.
 *      Author: mastersan
 */


#include "common/common_list2.h"
#include "types.h"
#include "server.h"
#include "server_private.h"
#include "client.h"
#include "g_gamesave.h"

#include <fcntl.h>
#include <unistd.h>

/* список записей */
gamesave_descr_t gamesaves[G_GAMESAVES_NUM];


static char * make_filename(char * filename, int i)
{
    sprintf(filename, "/ut_s%02d.sav", i);
    return filename;
}

/**
 * запись игрока
 * @return = 0 | -1
 */
static int g_gamesave_save_player(int fd, entity_t * player)
{
    write(fd, map_class_names[MAPDATA_MOBJ_SPAWN_PLAYER], strlen(map_class_names[MAPDATA_MOBJ_SPAWN_PLAYER])+1);
    gamesave_data_player_t savedata =
    {
            .fragstotal = ENTITY_VARIABLE_INTEGER(player, "fragstotal"),
            .frags      = ENTITY_VARIABLE_INTEGER(player, "frags"),
            .scores     = ENTITY_VARIABLE_INTEGER(player, "scores"),
            .health     = ENTITY_VARIABLE_INTEGER(player, "item_health"),
            .armor      = ENTITY_VARIABLE_INTEGER(player, "item_armor"),
            .ammo1      = ENTITY_VARIABLE_INTEGER(player, "item_ammo_artillery"),
            .ammo2      = ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile"),
            .ammo3      = ENTITY_VARIABLE_INTEGER(player, "item_ammo_mine")
    };
    write(fd, &savedata, sizeof(savedata));
    return 0;
};

/**
 * чтение игрока
 * @return true | false
 */
static int g_gamesave_load_player_internal(int fd, server_player_vars_storage_t * storage)
{
    mapdata_entity_type_t mapdata_mobj_type = map_file_class_get(fd);
    if(mapdata_mobj_type != MAPDATA_MOBJ_SPAWN_PLAYER)
        return -1;
    gamesave_data_player_t savedata;
    ssize_t c = read(fd, &savedata, sizeof(savedata));
    if(c != sizeof(savedata))return false;

    server_storage_vardata_get(storage, "fragstotal", VARTYPE_INTEGER)->value.i64  = savedata.fragstotal;
    server_storage_vardata_get(storage, "frags", VARTYPE_INTEGER)->value.i64       = savedata.frags;
    server_storage_vardata_get(storage, "scores", VARTYPE_INTEGER)->value.i64      = savedata.scores;
    server_storage_vardata_get(storage, "item_health", VARTYPE_INTEGER)->value.i64         = savedata.health;
    server_storage_vardata_get(storage, "item_armor", VARTYPE_INTEGER)->value.i64          = savedata.armor;
    server_storage_vardata_get(storage, "item_ammo_artillery", VARTYPE_INTEGER)->value.i64 = savedata.ammo1;
    server_storage_vardata_get(storage, "item_ammo_missile", VARTYPE_INTEGER)->value.i64   = savedata.ammo2;
    server_storage_vardata_get(storage, "item_ammo_mine", VARTYPE_INTEGER)->value.i64      = savedata.ammo3;

    vars_dump(storage->vars, "==== LOADED:");
    return 0;
};

/**
 * сохраниние записи
 * @return true| false
 */
int g_gamesave_save(int isave)
{
#define GS_WRITE(data, data_size) \
        do { \
            ssize_t count = write(fd, (data), (data_size)); \
            if(count != (data_size)) { \
                close(fd); \
                return -1; \
            } \
        } while(0)

#define GS_WRITE_V(data) \
        GS_WRITE(&data, sizeof(data))

#define GS_WRITE_U16(data) \
        do { \
            uint16_t value_u16 = data; \
            GS_WRITE(&value_u16, sizeof(value_u16)); \
        } while(0)

    gamesave_descr_t * gamesave = &gamesaves[isave];
    //strncpy(gamesave->name, name, G_GAMESAVE_NAME_SIZE);
    int fd;
    char filename[16];
    server_client_t * client;
    server_player_t * player;

    make_filename(filename, isave);

    check_directory(game_dir_saves);

    char * path;
    path = Z_malloc(strlen(game_dir_saves) + strlen(filename) + 1);
    strcpy(path, game_dir_saves);
    strcat(path, filename);

    //int ret = unlink(path);
    fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    Z_free(path);
    if(fd <= 0)
        return -1;
    gamesave_data_header_t header;
    strncpy(header.name, gamesave->name, G_GAMESAVE_NAME_SIZE);
    strncpy(header.mapfilename, map._file, MAP_FILENAME_SIZE);

    gamesave->flags = server.flags;
    header.flag_localgame = gamesave->flags.localgame;
    header.flag_allow_respawn = gamesave->flags.allow_respawn;

    GS_WRITE_V(header);

    int clients_num = server_clients_num_get();
    GS_WRITE_U16(clients_num);
    LIST2_FOREACHR(server.clients, client)
    {
        int players_num = server_client_players_num_get(client);
        GS_WRITE_U16(players_num);
    }

    LIST2_FOREACHR(server.clients, client)
    {
        LIST2_FOREACHR(client->players, player)
        {
            g_gamesave_save_player(fd, player->entity);
        }
    }

    close(fd);
    return 0;
};

/**
 * @brief чтение сохранённой игры
 * @return = 0 - успешно
 * @return = 1 - запись отсутствует
 * @return = 2 - карта отсутствует в списке карт
 * @return = 3 - ошибка чтения карты
 */
int g_gamesave_load_open(int isave, gamesave_load_context_t * ctx)
{
    ctx->fd = -1;
    ctx->clients_descr = NULL;

    /*
    gamesave_descr_t * gamesave = &gamesaves[isave];
    if(!gamesave->exist)
        return -1;
        */
    /*
    if(!(rec->flags & GAMEFLAG_CUSTOMGAME))
    {
        game.gamemap = map_find(rec->mapfilename);
        if(!game.gamemap) return 2;
    };
     */
    char filename[16];
    make_filename(filename, isave);
    char * path = Z_malloc(strlen(game_dir_saves) + strlen(filename) + 1);
    strcpy(path, game_dir_saves);
    strcat(path, filename);
    ctx->fd = open(path, O_RDONLY);
    Z_free(path);
    if(ctx->fd <= 0)
    {
        game_console_send("gamesave %d load error", isave);
        return -1;
    }
    return 0;
}

void g_gamesave_load_close(gamesave_load_context_t * ctx)
{
    if(ctx->fd >= 0)
    {
        Z_FREE(ctx->clients_descr);
        close(ctx->fd);
        memset(ctx, 0, sizeof(gamesave_load_context_t));
    }
}

#define GS_READ(data, data_size) \
        do { \
            ssize_t count = read(ctx->fd, (data), (data_size)); \
            if(count != (data_size)) { \
                g_gamesave_load_close(ctx); \
                return -1; \
            } \
        } while(0)

#define GS_READ_V(data) \
        GS_READ(&data, sizeof(data))

#define GS_READ_U16(data) \
        do { \
            uint16_t value_u16; \
            GS_READ(&value_u16, sizeof(value_u16)); \
            data = value_u16; \
        } while(0)

int g_gamesave_load_read_header(gamesave_load_context_t * ctx)
{
    gamesave_data_header_t header;

    GS_READ_V(header);
    strncpy(ctx->name, header.name, G_GAMESAVE_NAME_SIZE);
    strncpy(ctx->mapfilename, header.mapfilename, G_GAMESAVE_MAPFILENAME_SIZE);
    ctx->flag_localgame = header.flag_localgame;
    ctx->flag_allow_respawn = header.flag_allow_respawn;

    GS_READ_U16(ctx->clients_num);
    ctx->clients_descr = Z_malloc(sizeof(size_t) * ctx->clients_num);

    for(size_t clientId; clientId < ctx->clients_num; clientId++)
    {
        size_t players_num;
        GS_READ_U16(players_num);
        ctx->clients_descr[clientId] = players_num;
    }

    return 0;

}

void g_gamesave_load_player(
    gamesave_load_context_t * ctx,
    server_player_vars_storage_t * storage)
{
    g_gamesave_load_player_internal(ctx->fd, storage);
}

/**
 * @description кэширование заголовка записи
 * @return true | false
 */
static int g_gamesave_cacheinfo(int isave, gamesave_descr_t * rec)
{
    memset(rec, 0, sizeof(*rec));

    int res;
    gamesave_load_context_t ctx;
    res = g_gamesave_load_open(isave, &ctx);
    if(res) return -1;
    res = g_gamesave_load_read_header(&ctx);
    if(res) return -1;
    strncpy(rec->name, ctx.name, G_GAMESAVE_NAME_SIZE);
    rec->flags.localgame = ctx.flag_localgame;
    rec->flags.allow_respawn = ctx.flag_allow_respawn;
    rec->players_num_total = 0;
    for(size_t i = 0; i < ctx.clients_num; i++)
    {
        rec->players_num_total += ctx.clients_descr[i];
    }
    rec->exist = true;

    g_gamesave_load_close(&ctx);
    return 0;
}


/**
 * формируем листинг записей
 */
void g_gamesave_cacheinfos(void)
{
    size_t i;
    memset(gamesaves, 0, sizeof(gamesaves));
    for(i = 0; i < G_GAMESAVES_NUM; i++)
    {
        g_gamesave_cacheinfo(i, &(gamesaves[i]));
    }
}
