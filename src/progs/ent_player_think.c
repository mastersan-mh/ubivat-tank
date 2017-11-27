/*
 *        Ubivat Tank
 *        управление игроками
 * by Master San
 */

#include "progs.h"

#include "ent_bull.h"
#include "ent_explode.h"
#include "ent_player_think.h"
#include "ent_player.h"
#include "ent_weap.h"

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
static bool ctrl_AI_checkdanger(ENTITY player, ENTITY dangerous)
{
    map_t * map = world_map_get();
	vec_t Udist;
	vec_t Ddist;
	vec_t Ldist;
	vec_t Rdist;
	vec_t Ud;
	vec_t Dd;
	vec_t Ld;
	vec_t Rd;

	player_vars_t * pl = entity_vars(player);
    player_vars_t * dng = entity_vars(dangerous);

	bool danger = false;
	if(pl->bull)
		return false;
	if(entity_parent(dangerous) == player)
		return false;

	vec_t radius = entity_info_bodybox(dangerous) * 0.5f;

	vec_t halfbox = ENTITY_HALFBODYBOX(player);

	//верхняя ближайшая стена
	map_clip_find_near(map, pl->origin, 0, DIR_UP, MAP_WALL_CLIP, 100, &Udist);
	//нижняя ближайшая стена
	map_clip_find_near(map, pl->origin, 0, DIR_DOWN, MAP_WALL_CLIP, 100, &Ddist);
	//левая ближайшая стена
	map_clip_find_near(map, pl->origin, 0, DIR_LEFT, MAP_WALL_CLIP, 160, &Ldist);
	//правая ближайшая стена
	map_clip_find_near(map, pl->origin, 0, DIR_RIGHT, MAP_WALL_CLIP, 160, &Rdist);
	Ud = (pl->origin_y + Udist - halfbox) - (dng->origin_y + radius);
	Dd = (dng->origin_y - radius) - (pl->origin_y-Ddist + halfbox);
	Rd = (pl->origin_x + Rdist - halfbox) - (dng->origin_x + radius);
	Ld = (dng->origin_x - radius) - (pl->origin_x - Ldist + halfbox);
	if(
			(pl->origin_x - halfbox <= dng->origin_x + radius)&&
			(dng->origin_x - radius <= pl->origin_x + halfbox)&&
			(VEC_ABS(pl->origin_y-dng->origin_y) < 128)
	)
	{
		if(
				(dng->dir == DIR_UP) &&
				(VEC_ABS(pl->origin_y-dng->origin_y)<Ddist) &&
				(dng->origin_y + radius < pl->origin_y-halfbox)
		)
		{
			danger = true;
			if(!pl->brain.Fdanger)
			{
				pl->move.go = true;
				pl->brain.Fdanger = true;
				if(0 <= Ld && 0 <= Rd)
					pl->dir = xrand(2) + 2;
				else
					if(0 < Ld) pl->dir = DIR_LEFT;
					else
						if(0 < Rd) pl->dir = DIR_RIGHT;
			}
		}
		else
		{
			if(
					(dng->dir == DIR_DOWN) &&
					(VEC_ABS(pl->origin_y-dng->origin_y)<Udist) &&
					(pl->origin_y+halfbox < dng->origin_y - radius)
			)
			{
				danger = true;
				if(!pl->brain.Fdanger) {
					pl->move.go = true;
					pl->brain.Fdanger = true;
					if( 0 <= Ld && 0 <= Rd) pl->dir = xrand(2)+2;
					else
						if(0 < Ld) pl->dir = DIR_LEFT;
						else
							if(0 < Rd) pl->dir = DIR_RIGHT;
				}
			}
		}
	}
	else
	{
		if(
				(pl->origin_y - halfbox <= dng->origin_y + radius) &&
				(dng->origin_y - radius <= pl->origin_y + halfbox) &&
				(VEC_ABS(pl->origin_x - dng->origin_x) < 128)
		)
		{
			if (
					(dng->dir == DIR_LEFT)&&
					(VEC_ABS(pl->origin_x-dng->origin_x)<Rdist)&&
					(pl->origin_x+halfbox < dng->origin_x- radius)
			)
			{
				danger = true;
				if(!pl->brain.Fdanger)
				{
					pl->move.go = true;
					pl->brain.Fdanger = true;
					if(0 <= Ud && 0 <= Dd)
						pl->dir = xrand(2);
					else
						if(0 < Ud)
							pl->dir = DIR_UP;
						else
							if(0 < Dd)
								pl->dir = DIR_DOWN;
				}
			}
			else
			{
				if(
						(dng->dir == DIR_RIGHT) &&
						(VEC_ABS(pl->origin_x - dng->origin_x)<Ldist) &&
						(dng->origin_x + radius < pl->origin_x - halfbox)
				)
				{
					danger = true;
					if(!pl->brain.Fdanger)
					{
						pl->move.go = true;
						pl->brain.Fdanger = true;
						if(0 <= Ud && 0 <= Dd) pl->dir = xrand(2);
						else
							if(0 < Ud) pl->dir = DIR_UP;
							else
								if(0 < Dd) pl->dir = DIR_DOWN;
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
static void ctrl_AI_checkdangers(ENTITY player)
{
	player_vars_t * pl = entity_vars(player);
	bool danger = false;
	ENTITY dangerous;

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
static bool turn_to_target(ENTITY player, ENTITY target)
{
    player_vars_t * pl = entity_vars(player);
    entity_vars_common_t * trg = entity_vars(target);
	/* повернёмся к противнику */
	if( (pl->origin_x - ENTITY_HALFBODYBOX(player) < trg->origin_x) && (trg->origin_x < pl->origin_x + ENTITY_HALFBODYBOX(player)) )
	{
		if(trg->origin_y < pl->origin_y)
			pl->dir = DIR_DOWN;
		else
			pl->dir = DIR_UP;
		return true;
	}
	if( (pl->origin_y - ENTITY_HALFBODYBOX(player) < trg->origin_y) && (trg->origin_y < pl->origin_y + ENTITY_HALFBODYBOX(player)) )
	{
		if(trg->origin_x < pl->origin_x)
			pl->dir = DIR_LEFT;
		else
			pl->dir = DIR_RIGHT;
		return true;
	}
	return false;
}


/*
 * атака
 */
static void ctrl_AI_attack(ENTITY player, ENTITY target)
{
    map_t * map = world_map_get();

	/* противник в прямой видимости */
	void P_weapon_select_direct_view(ENTITY player)
	{
		player_vars_t * pl = entity_vars(player);
		if(pl->item_ammo_mine > 0)
		{
			//выбираем наугад ракету или мину
			pl->brain.weap = 1 + xrand(2);
			if(pl->brain.weap == WEAP_MINE)
				pl->dir = entity_direction_invert(pl->dir);
		}
		else
		{
			if(pl->item_ammo_missile > 0)
				pl->brain.weap = WEAP_MISSILE;
			else
				pl->brain.weap = WEAP_ARTILLERY;
		}
		pl->brain.attack = true;
	}

	/* противник за стеной, пытаемся пробиться через стену */
	void P_weapon_select_behind_wall(ENTITY player, player_vars_t* pl, char wall, vec_t dist, vec_t explode_radius)
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


	player_vars_t * pl = entity_vars(player);

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
	    bull_vars_t * bull_vars = entity_vars(pl->bull);
        player_vars_t * target_vars = entity_vars(pl->brain.target);
		if(
				VEC_ABS(bull_vars->origin_y - target_vars->origin_y) <
				VEC_ABS(bull_vars->origin_x - target_vars->origin_x)
		)
		{
			if(bull_vars->origin_x < target_vars->origin_x)
				pl->dir = DIR_RIGHT;
			else
				pl->dir = DIR_LEFT;
		}
		else
		{
			if(bull_vars->origin_y < target_vars->origin_y)
				pl->dir = DIR_UP;
			else
				pl->dir = DIR_DOWN;
		}
		return;
	}

    entity_vars_common_t * trg = entity_vars(target);

	if
	(
			VEC_ABS(pl->origin_x - trg->origin_x) > 160.0 ||
			VEC_ABS(pl->origin_y - trg->origin_y) > 100.0
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
	if(pl->item_ammo_missile > 0)
	{
		map_clip_find_near_wall(map, pl->origin, pl->dir, &dist, &wall);
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
			map_clip_find_near_wall(map, pl->origin, pl->dir, &dist, &wall);
			if(
					( (pl->dir == DIR_DOWN || pl->dir == DIR_UP   ) && VEC_ABS(pl->origin_y - trg->origin_y) < dist ) ||
					( (pl->dir == DIR_LEFT || pl->dir == DIR_RIGHT) && VEC_ABS(pl->origin_x - trg->origin_x) < dist ) ||
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
static void ctrl_AI_findenemy(ENTITY player, ENTITY target)
{
    player_vars_t * pl = entity_vars(player);
    entity_vars_common_t * trg = entity_vars(target);

	if(
			(160<VEC_ABS(pl->origin_x-trg->origin_x))||
			(100<VEC_ABS(pl->origin_y-trg->origin_y))
	)
	{
		pl->attack = false;
		return;
	}
	if(pl->brain.count == 0) {
		pl->dir = xrand(4);
		pl->move.go = true;
	}
	else
	{
		if(BOT_THINK_TIME + xrand(BOT_THINK_TIME) < pl->brain.count)
		{
			if(VEC_ABS(pl->origin_x-trg->origin_x)>VEC_ABS(pl->origin_y-trg->origin_y))
			{
				if(pl->origin_x < trg->origin_x)
					pl->dir = DIR_RIGHT;
				else
					pl->dir = DIR_LEFT;
			}
			else
			{
				if(pl->origin_y < trg->origin_y)
					pl->dir = DIR_UP;
				else
					pl->dir = DIR_DOWN;
			}
			if(
					(VEC_ABS(pl->origin_x - trg->origin_x) < c_BOT_dist)&&
					(VEC_ABS(pl->origin_y - trg->origin_y) < c_BOT_dist)
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
void think_enemy(ENTITY player)
{

    player_vars_t * pl = entity_vars(player);

	if(debug_noAI)
		return;
	if(!pl->alive)
		return;
	ctrl_AI_checkdangers(player);
	if(pl->brain.danger)
		return;
	ENTITY target = entity_get_random("player");

	if(!target)
		return;

    entity_vars_common_t * trg = entity_vars(target);

	if(!trg->alive)
		pl->attack = false;
	else
	{
		if(!pl->bull && !pl->attack)
			ctrl_AI_findenemy(player, target);
		ctrl_AI_attack(player, target);
	}
}
