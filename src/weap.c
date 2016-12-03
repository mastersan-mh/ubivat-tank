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
weaponinfo_t wtable[__WEAP_NUM] =
{
		{ "Artillery", IMG_WEAPON_ARTILLERY },
		{ "Missile"  , IMG_WEAPON_MISSILE   },
		{ "Mine"     , IMG_WEAPON_MINE      }
};


