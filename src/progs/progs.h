/*
 * @file progs.h
 *
 * @brief Common include file for user progs files.
 * @note Do not include other files into user progs files.
 *
 *  Created on: 27 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_PROGS_PROGS_H_
#define SRC_PROGS_PROGS_H_

#include "../types.h"
#include "../system.h"
#include "../game_progs.h"
#include "../world.h"
#include "../map.h"
#include "../entity.h"
#include "../model.h"
#include "../img.h"
#include "../sound.h"
#include "../_gr2D.h"
#include "../video.h"
#include "../game.h"
#include "../ui.h"
#include "../server.h"
#include "../sv_game.h"
#include "../client.h"
#include "helpers.h"
#include <stdlib.h>

#define SPAWN_INFO(xclassname, xvarstype, xvars, xmodels_num) \
    { \
    .classname = (xclassname), \
            .vars_size = sizeof(xvarstype), \
            .vars_descr_num = ARRAYSIZE(xvars), \
            .vars_descr = xvars, \
            .models_num = (xmodels_num), \
    }

#define GAME_EXPORTS_ENTITIES_INFO(xinfos) \
        .infos_num = ARRAYSIZE(xinfos), \
        .infos = xinfos

#define GAME_EXPORTS_ACTIONS(xactions) \
        .actions_num = ARRAYSIZE(xactions), \
        .actions = xactions

#endif /* SRC_PROGS_PROGS_H_ */
