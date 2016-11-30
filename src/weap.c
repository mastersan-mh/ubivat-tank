/*
 *        Ubivat Tank
 *        управление оружием, пулями, взрывами
 * by Master San
 */

#include <video.h>
#include <game.h>
#include <weap.h>
#include <img.h>
#include <map.h>
#include <_gr2D.h>
#include "sound.h"

#include "mobjs.h"
#include "explode.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <player.h>
#include <types.h>

//оружия
weapon_info_t wtable[3] =
{
		{ "Artillery",  15,   7, WEAP_AMMO_INFINITE,  7,  -1,  75, 2, IMG_WEAPON_ARTILLERY },
		{ "Missile"  , 100,  50, 50                , 11,  -1,  80, 8, IMG_WEAPON_MISSILE   },
		{ "Mine"     , 200, 100, 50                , 11, 100, -80, 8, IMG_WEAPON_MINE      }
};


