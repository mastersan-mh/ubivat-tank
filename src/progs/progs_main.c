/*
 * @file progs_main.c
 *
 * @brief Progs init file
 *
 *  Created on: 27 нояб. 2017 г.
 *      Author: mastersan
 */

#include "progs_main.h"

#include "entities_includes.h"

void progs_init(void)
{
    entity_bull_init();
    entity_explode_init();
    entity_spawn_init();
    entity_player_init();
    entity_message_init();
    entity_exit_init();
    entity_items_init();
}
