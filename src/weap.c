/*
 *        Ubivat Tank
 *        управление оружием, пулями, взрывами
 * by Master San
 */

#include "weap.h"
#include "ent_player.h"


#include <string.h>
#include <stdlib.h>
#include <math.h>

//оружия
weaponinfo_t wtable[__WEAP_NUM] =
{
		{ "Artillery", IMG_WEAPON_ARTILLERY },
		{ "Missile"  , IMG_WEAPON_MISSILE   },
		{ "Mine"     , IMG_WEAPON_MINE      }
};


bulltype_t entity_weapon_type_to_bull_type(weapontype_t type)
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

explodetype_t entity_bull_type_to_explode_type(bulltype_t bulltype)
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

const char * entity_bulltype_to_mobj(bulltype_t type)
{
	static const char *list[] =
	{
			"bull_artillery",
			"bull_missile",
			"bull_mine"
	};
	return list[type];
}

const char * entity_explodetype_to_mobjtype(explodetype_t type)
{
	static const char *list[] =
	{
			"explode_artillery",
			"explode_missile",
			"explode_mine"
	};
	return list[type];
}

