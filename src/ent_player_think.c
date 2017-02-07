/*
 *        Ubivat Tank
 *        управление игроками
 * by Master San
 */
#include "ent_bull.h"
#include "ent_explode.h"
#include "ent_player_think.h"
#include "ent_player.h"
#include "ent_weap.h"

#include "entity_helpers.h"
#include "types.h"
#include "game.h"
#include "map.h"
#include "client.h"
#include "system.h"

#include <stdlib.h>

//расстояние с которого бот будет стремиться вести атаку
#define c_BOT_dist      (16 * 3)

static bool debug_noAI = false;

static explodetype_t weapontype_to_explodetype(weapontype_t weapontype)
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

/*
 * инициализация AI
 */
void ctrl_AI_init(think_t * thiker)
{
	thiker->danger  = 0;
	thiker->Fdanger = 0;
	thiker->attack  = false;
	thiker->target  = NULL;
	thiker->count   = xrand(BOT_THINK_TIME*2);
};
/*
 * удаление AI
 */
void ctrl_AI_done(think_t * thinker)
{
	thinker->target = NULL;
};


/*
 * уворачивание от снарядов
 */
static bool ctrl_AI_checkdanger(entity_t * player, entity_t * dangerous)
{
	vec_t Udist;
	vec_t Ddist;
	vec_t Ldist;
	vec_t Rdist;
	vec_t Ud;
	vec_t Dd;
	vec_t Ld;
	vec_t Rd;

	player_t * pl = player->data;

	bool danger = false;
	if(pl->bull)
		return false;
	if(dangerous->parent == player)
		return false;

	vec_t radius = dangerous->info->bodybox * 0.5f;

	vec_t halfbox = ENTITY_HALFBODYBOX(player);

	//верхняя ближайшая стена
	map_clip_find_near(player->origin, 0, DIR_UP, MAP_WALL_CLIP, 100, &Udist);
	//нижняя ближайшая стена
	map_clip_find_near(player->origin, 0, DIR_DOWN, MAP_WALL_CLIP, 100, &Ddist);
	//левая ближайшая стена
	map_clip_find_near(player->origin, 0, DIR_LEFT, MAP_WALL_CLIP, 160, &Ldist);
	//правая ближайшая стена
	map_clip_find_near(player->origin, 0, DIR_RIGHT, MAP_WALL_CLIP, 160, &Rdist);
	Ud = (player->origin_y + Udist - halfbox) - (dangerous->origin_y + radius);
	Dd = (dangerous->origin_y - radius) - (player->origin_y-Ddist + halfbox);
	Rd = (player->origin_x + Rdist - halfbox) - (dangerous->origin_x + radius);
	Ld = (dangerous->origin_x - radius) - (player->origin_x - Ldist + halfbox);
	if(
			(player->origin_x - halfbox <= dangerous->origin_x + radius)&&
			(dangerous->origin_x - radius <= player->origin_x + halfbox)&&
			(VEC_ABS(player->origin_y-dangerous->origin_y) < 128)
	)
	{
		if(
				(dangerous->dir == DIR_UP) &&
				(VEC_ABS(player->origin_y-dangerous->origin_y)<Ddist) &&
				(dangerous->origin_y + radius < player->origin_y-halfbox)
		)
		{
			danger = true;
			if(!pl->brain.Fdanger)
			{
				pl->move.go = true;
				pl->brain.Fdanger = true;
				if(0 <= Ld && 0 <= Rd)
					player->dir = xrand(2) + 2;
				else
					if(0 < Ld) player->dir = DIR_LEFT;
					else
						if(0 < Rd) player->dir = DIR_RIGHT;
			}
		}
		else
		{
			if(
					(dangerous->dir == DIR_DOWN) &&
					(VEC_ABS(player->origin_y-dangerous->origin_y)<Udist) &&
					(player->origin_y+halfbox < dangerous->origin_y - radius)
			)
			{
				danger = true;
				if(!pl->brain.Fdanger) {
					pl->move.go = true;
					pl->brain.Fdanger = true;
					if( 0 <= Ld && 0 <= Rd) player->dir = xrand(2)+2;
					else
						if(0 < Ld) player->dir = DIR_LEFT;
						else
							if(0 < Rd) player->dir = DIR_RIGHT;
				}
			}
		}
	}
	else
	{
		if(
				(player->origin_y - halfbox <= dangerous->origin_y + radius) &&
				(dangerous->origin_y - radius <= player->origin_y + halfbox) &&
				(VEC_ABS(player->origin_x - dangerous->origin_x) < 128)
		)
		{
			if (
					(dangerous->dir == DIR_LEFT)&&
					(VEC_ABS(player->origin_x-dangerous->origin_x)<Rdist)&&
					(player->origin_x+halfbox < dangerous->origin_x- radius)
			)
			{
				danger = true;
				if(!pl->brain.Fdanger)
				{
					pl->move.go = true;
					pl->brain.Fdanger = true;
					if(0 <= Ud && 0 <= Dd)
						player->dir = xrand(2);
					else
						if(0 < Ud)
							player->dir = DIR_UP;
						else
							if(0 < Dd)
								player->dir = DIR_DOWN;
				}
			}
			else
			{
				if(
						(dangerous->dir == DIR_RIGHT) &&
						(VEC_ABS(player->origin_x - dangerous->origin_x)<Ldist) &&
						(dangerous->origin_x + radius < player->origin_x - halfbox)
				)
				{
					danger = true;
					if(!pl->brain.Fdanger)
					{
						pl->move.go = true;
						pl->brain.Fdanger = true;
						if(0 <= Ud && 0 <= Dd) player->dir = xrand(2);
						else
							if(0 < Ud) player->dir = DIR_UP;
							else
								if(0 < Dd) player->dir = DIR_DOWN;
					}
				}
			}
		}
	}
	return danger;
}



/*
 * уворачивание от снарядов
 */
static void ctrl_AI_checkdangers(entity_t * player)
{
	player_t * pl = player->data;
	bool danger = false;
	entity_t * dangerous;

	ENTITIES_FOREACH("bull_artillery", dangerous)
	{
		danger = ctrl_AI_checkdanger(player, dangerous);
		if(danger)
			goto next;
	}
	ENTITIES_FOREACH("bull_missile", dangerous)
	{
		danger = ctrl_AI_checkdanger(player, dangerous);
		if(danger)
			goto next;
	}
	ENTITIES_FOREACH("bull_mine", dangerous)
	{
		danger = ctrl_AI_checkdanger(player, dangerous);
		if(danger)
			goto next;
	}

	next:
	pl->brain.danger = danger;
	if(!pl->brain.danger)
	{
		pl->brain.Fdanger = false;
		//player->move.go = false;
	}
}

/*
 * повернуться лицом к цели
 * @return true - успешно
 * @return false - не можем повернуться к цели
 */
static bool turn_to_target(entity_t * player, entity_t * target)
{
	/* повернёмся к противнику */
	if( (player->origin_x - ENTITY_HALFBODYBOX(player) < target->origin_x) && (target->origin_x < player->origin_x + ENTITY_HALFBODYBOX(player)) )
	{
		if(target->origin_y < player->origin_y)
			player->dir = DIR_DOWN;
		else
			player->dir = DIR_UP;
		return true;
	}
	if( (player->origin_y - ENTITY_HALFBODYBOX(player) < target->origin_y) && (target->origin_y < player->origin_y + ENTITY_HALFBODYBOX(player)) )
	{
		if(target->origin_x < player->origin_x)
			player->dir = DIR_LEFT;
		else
			player->dir = DIR_RIGHT;
		return true;
	}
	return false;
}


/*
 * атака
 */
static void ctrl_AI_attack(entity_t * player, entity_t * target)
{

	/* противник в прямой видимости */
	void P_weapon_select_direct_view(entity_t *player)
	{
		player_t * pl = player->data;
		if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_mine") > 0)
		{
			//выбираем наугад ракету или мину
			pl->brain.weap = 1 + xrand(2);
			if(pl->brain.weap == WEAP_MINE)
				player->dir = entity_direction_invert(player->dir);
		}
		else
		{
			if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") > 0)
				pl->brain.weap = WEAP_MISSILE;
			else
				pl->brain.weap = WEAP_ARTILLERY;
		}
		pl->brain.attack = true;
	}

	/* противник за стеной, пытаемся пробиться через стену */
	void P_weapon_select_behind_wall(entity_t* player, player_t* pl, char wall, vec_t dist, vec_t explode_radius)
	{
		if (MAP_WALL_TEXTURE(wall) == MAP_WALL_W0)
			pl->brain.attack = false; //сильная броня, не стреляем
		else
		{
			if (MAP_WALL_TEXTURE(wall) == MAP_WALL_brick) /* кирпич */
			{
				pl->brain.weap = WEAP_ARTILLERY;
				pl->brain.attack = true;
			}
			else
			{
				if (MAP_WALL_TEXTURE(wall) == MAP_WALL_W1) /* слабая броня */
					P_weapon_select_direct_view(player);
			}
		}
		if(
				(dist - ENTITY_HALFBODYBOX(player) * 2 < explode_radius) &&
				(
						MAP_WALL_CLIPPED(wall) &&
						( MAP_WALL_TEXTURE(wall) == MAP_WALL_W0 || MAP_WALL_TEXTURE(wall) == MAP_WALL_W1)
				)
		)
			pl->brain.attack = false;
	}


	player_t * pl = player->data;

	vec_t dist;
	char wall;

	static char *list[] =
	{
			"explode_artillery",
			"explode_missile",
			"explode_mine",
	};
	const entityinfo_t * explode_entityinfo = entityinfo_get(list[weapontype_to_explodetype(pl->brain.weap)]);
	vec_t explode_radius = explode_entityinfo->bodybox * 0.5f;

	/* управляемый снаряд */
	if( pl->bull && pl->brain.target )
	{
		if(
				VEC_ABS(pl->bull->origin_y - pl->brain.target->origin_y) <
				VEC_ABS(pl->bull->origin_x - pl->brain.target->origin_x)
		)
		{
			if(pl->bull->origin_x < pl->brain.target->origin_x)
				player->dir = DIR_RIGHT;
			else
				player->dir = DIR_LEFT;
		}
		else
		{
			if(pl->bull->origin_y < pl->brain.target->origin_y)
				player->dir = DIR_UP;
			else
				player->dir = DIR_DOWN;
		}
		return;
	}

	if
	(
			VEC_ABS(player->origin_x - target->origin_x) > 160.0 ||
			VEC_ABS(player->origin_y - target->origin_y) > 100.0
	)
	{
		pl->attack = false;
		return;
	}

	//если оружие не перезарядилось
	if(0 < pl->reloadtime_d)
		return;

	vec_t halfbox = ENTITY_HALFBODYBOX(player);

	pl->brain.target = NULL;
	if(ENTITY_VARIABLE_INTEGER(player, "item_ammo_missile") > 0)
	{
		map_clip_find_near_wall(player->origin, player->dir, &dist, &wall);
		if(
				MAP_WALL_CLIPPED(wall) &&
				(
						MAP_WALL_TEXTURE(wall) == MAP_WALL_W0 ||
						( MAP_WALL_TEXTURE(wall) == MAP_WALL_W1 && dist - halfbox < explode_radius )
				)
		)
			pl->brain.attack = false;
		else
		{
			pl->brain.weap = WEAP_MISSILE;
			pl->brain.attack = true;
		}
	}
	else
	{
		if(turn_to_target(player, target))
		{
			map_clip_find_near_wall(player->origin, player->dir, &dist, &wall);
			if(
					( (player->dir == DIR_DOWN || player->dir == DIR_UP   ) && VEC_ABS(player->origin_y - target->origin_y) < dist ) ||
					( (player->dir == DIR_LEFT || player->dir == DIR_RIGHT) && VEC_ABS(player->origin_x - target->origin_x) < dist ) ||
					!MAP_WALL_CLIPPED(wall) ||
					MAP_WALL_TEXTURE(wall) == MAP_WALL_water
			)
				P_weapon_select_direct_view(player); /* противник в прямой видимости */
			else
				P_weapon_select_behind_wall(player, pl, wall, dist, explode_radius); /* противник за стеной, пытаемся пробиться через стену */
		}

	}
	pl->weap = pl->brain.weap;
	pl->attack = pl->brain.attack;
	if(pl->brain.weap == WEAP_MISSILE)
		pl->brain.target = target; //если выпустил ракету, тогда цель постоянная
	pl->brain.attack = false;
}
/*
 * поиск врага
 */
static void ctrl_AI_findenemy(entity_t * player, entity_t * target)
{
	player_t * pl = player->data;

	if(
			(160<VEC_ABS(player->origin_x-target->origin_x))||
			(100<VEC_ABS(player->origin_y-target->origin_y))
	)
	{
		pl->attack = false;
		return;
	}
	if(pl->brain.count == 0) {
		player->dir = xrand(4);
		pl->move.go = true;
	}
	else
	{
		if(BOT_THINK_TIME + xrand(BOT_THINK_TIME) < pl->brain.count)
		{
			if(VEC_ABS(player->origin_x-target->origin_x)>VEC_ABS(player->origin_y-target->origin_y))
			{
				if(player->origin_x < target->origin_x)
					player->dir = DIR_RIGHT;
				else
					player->dir = DIR_LEFT;
			}
			else
			{
				if(player->origin_y < target->origin_y)
					player->dir = DIR_UP;
				else
					player->dir = DIR_DOWN;
			}
			if(
					(VEC_ABS(player->origin_x - target->origin_x) < c_BOT_dist)&&
					(VEC_ABS(player->origin_y - target->origin_y) < c_BOT_dist)
			)
				pl->move.go = false;
			else
				pl->move.go = true;
		}
	}
	pl->brain.count += dtime;
	if(BOT_THINK_TIME * 2 < pl->brain.count)
		pl->brain.count = 0;
}

/*
 * управление вражеским игроком
 */
void think_enemy(entity_t * player)
{
	player_t * pl = player->data;
	if(debug_noAI)
		return;
	if(!player->alive)
		return;
	ctrl_AI_checkdangers(player);
	if(pl->brain.danger)
		return;
	entity_t * target = entity_get_random("player");
	if(!target)
		return;

	if(!target->alive)
		pl->attack = false;
	else
	{
		if(!pl->bull && !pl->attack)
			ctrl_AI_findenemy(player, target);
		ctrl_AI_attack(player, target);
	}
}
