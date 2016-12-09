/*
 *        Ubivat Tank
 *        управление оружием, пулями, взрывами
 * by Master San
 */

#include <video.h>
#include <game.h>
#include "weap.h"
#include <img.h>
#include <map.h>
#include <_gr2D.h>
#include "sound.h"


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


bulltype_t mobj_weapon_type_to_bull_type(weapontype_t type)
{
	switch(type)
	{
	case WEAP_ARTILLERY: return EXPLODE_ARTILLERY;
	case WEAP_MISSILE  : return EXPLODE_MISSILE;
	case WEAP_MINE     : return EXPLODE_MINE;
	default: ;
	}
	return EXPLODE_ARTILLERY;
}

explodetype_t mobj_bull_type_to_explode_type(bulltype_t bulltype)
{
	switch(bulltype)
	{
	case BULL_ARTILLERY: return EXPLODE_ARTILLERY;
	case BULL_MISSILE  : return EXPLODE_MISSILE;
	case BULL_MINE     : return EXPLODE_MINE;
	default: ;
	}
	return EXPLODE_ARTILLERY;
}

mobj_type_t mobl_byulltype_to_mobj(bulltype_t bulltype)
{
	switch(bulltype)
	{
		case BULL_ARTILLERY: return MOBJ_BULL_ARTILLERY;
		case BULL_MISSILE  : return MOBJ_BULL_MISSILE;
		case BULL_MINE     : return MOBJ_BULL_MINE;
		default: ;
	}
	return __MOBJ_NUM;
}

mobj_type_t mobj_explodetype_to_mobjtype(explodetype_t type)
{
	switch(type)
	{
		case EXPLODE_ARTILLERY: return MOBJ_EXPLODE_ARTILLERY;
		case EXPLODE_MISSILE  : return MOBJ_EXPLODE_MISSILE;
		case EXPLODE_MINE     : return MOBJ_EXPLODE_MINE;
		default: ;
	}
	return __MOBJ_NUM;
}

