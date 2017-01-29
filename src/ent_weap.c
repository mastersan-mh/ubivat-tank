/*
 *        Ubivat Tank
 *        управление оружием, пулями, взрывами
 * by Master San
 */

#include "entity.h"
#include "ent_weap.h"
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

explodetype_t weapontype_to_explodetype(weapontype_t weapontype)
{
	switch(weapontype)
	{
		case WEAP_ARTILLERY: return EXPLODE_ARTILLERY;
		case WEAP_MISSILE  : return EXPLODE_MISSILE;
		case WEAP_MINE     : return EXPLODE_MINE;
		default: ;
	}
	return EXPLODE_ARTILLERY;
}

const char * weapontype_to_bullentity(weapontype_t weapontype)
{
	static const char *list[] =
	{
			"bull_artillery",
			"bull_missile",
			"bull_mine"
	};
	return list[weapontype];
}

explodetype_t bullentity_to_explodetype(entity_t * bull)
{
	if(ENTITY_IS(bull, "bull_artillery"))
	{
		return EXPLODE_ARTILLERY;
	}
	if(ENTITY_IS(bull, "bull_missile"))
	{
		return EXPLODE_MISSILE;
	}
	if(ENTITY_IS(bull, "bull_mine"))
	{
		return EXPLODE_MINE;
	}
	return EXPLODE_ARTILLERY;
}

const char * explodetype_to_explodeentity(explodetype_t type)
{
	static const char *list[] =
	{
			"explode_artillery",
			"explode_missile",
			"explode_mine"
	};
	return list[type];
}

