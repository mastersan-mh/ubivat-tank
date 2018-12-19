/*
 * @file gamedefs.h
 *
 * @brief Global defines of the game
 *
 *  Created on: 26 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_GAMEDEFS_H_
#define SRC_GAMEDEFS_H_

#include "config.h"

#define CLIENT_PLAYERS_MAX 2
#define CLIENTS_MAX 4
#define PLAYERS_MAX (CLIENTS_MAX * CLIENT_PLAYERS_MAX)

#define UT_KEYSIZE 64
#define UT_VALUESIZE 2048

#define MIN(a, b) ((a)<(b) ? (a) : (b))

//#define _ZMEM_DEBUG _DEBUG
/* Next _ZMEM_DEBUG_* defines works only if _ZMEM_DEBUG are defined */
//#define _ZMEM_DEBUG_TRACE
//#define _ZMEM_DEBUG_MEMDUMP

/* Do dump of the entities variables */
#define VARS_DUMP_ALLOW

#endif /* SRC_GAMEDEFS_H_ */
