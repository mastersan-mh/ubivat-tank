/*
 *        Ubivat Tank
 *        управление игроками
 * by Master San
 */

#include "progs.h"
#include "helpers.h"

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
void ctrl_AI_init(player_ai_t * thiker)
{
	thiker->danger  = 0;
	thiker->Fdanger = 0;
	thiker->attack  = false;
	thiker->target  = NULL;
	thiker->count   = xrand(BOT_THINK_TIME*2);
};

/*
 * уворачивание от снарядов
 */
static bool ctrl_AI_checkdanger(entity_t * player, entity_t * dangerous)
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

	bool danger = false;
	if(player->bull)
		return false;
	if(dangerous->owner == player)
		return false;

	vec_t radius = dangerous->c.bodybox * 0.5f;

	vec_t halfbox = player->c.bodybox * 0.5;

	//верхняя ближайшая стена
	map_clip_find_near(map, player->c.origin, 0, DIR_UP, MAP_WALL_CLIP, 100, &Udist);
	//нижняя ближайшая стена
	map_clip_find_near(map, player->c.origin, 0, DIR_DOWN, MAP_WALL_CLIP, 100, &Ddist);
	//левая ближайшая стена
	map_clip_find_near(map, player->c.origin, 0, DIR_LEFT, MAP_WALL_CLIP, 160, &Ldist);
	//правая ближайшая стена
	map_clip_find_near(map, player->c.origin, 0, DIR_RIGHT, MAP_WALL_CLIP, 160, &Rdist);
	Ud = (player->c.origin_y + Udist - halfbox) - (dangerous->c.origin_y + radius);
	Dd = (dangerous->c.origin_y - radius) - (player->c.origin_y-Ddist + halfbox);
	Rd = (player->c.origin_x + Rdist - halfbox) - (dangerous->c.origin_x + radius);
	Ld = (dangerous->c.origin_x - radius) - (player->c.origin_x - Ldist + halfbox);
	if(
			(player->c.origin_x - halfbox <= dangerous->c.origin_x + radius)&&
			(dangerous->c.origin_x - radius <= player->c.origin_x + halfbox)&&
			(VEC_ABS(player->c.origin_y-dangerous->c.origin_y) < 128)
	)
	{
		if(
				(dangerous->c.dir == DIR_UP) &&
				(VEC_ABS(player->c.origin_y-dangerous->c.origin_y)<Ddist) &&
				(dangerous->c.origin_y + radius < player->c.origin_y-halfbox)
		)
		{
			danger = true;
			if(!player->brain.Fdanger)
			{
				player->move.go = true;
				player->brain.Fdanger = true;
				if(0 <= Ld && 0 <= Rd)
					player->c.dir = xrand(2) + 2;
				else
					if(0 < Ld) player->c.dir = DIR_LEFT;
					else
						if(0 < Rd) player->c.dir = DIR_RIGHT;
			}
		}
		else
		{
			if(
					(dangerous->c.dir == DIR_DOWN) &&
					(VEC_ABS(player->c.origin_y-dangerous->c.origin_y)<Udist) &&
					(player->c.origin_y+halfbox < dangerous->c.origin_y - radius)
			)
			{
				danger = true;
				if(!player->brain.Fdanger) {
					player->move.go = true;
					player->brain.Fdanger = true;
					if( 0 <= Ld && 0 <= Rd) player->c.dir = xrand(2)+2;
					else
						if(0 < Ld) player->c.dir = DIR_LEFT;
						else
							if(0 < Rd) player->c.dir = DIR_RIGHT;
				}
			}
		}
	}
	else
	{
		if(
				(player->c.origin_y - halfbox <= dangerous->c.origin_y + radius) &&
				(dangerous->c.origin_y - radius <= player->c.origin_y + halfbox) &&
				(VEC_ABS(player->c.origin_x - dangerous->c.origin_x) < 128)
		)
		{
			if (
					(dangerous->c.dir == DIR_LEFT)&&
					(VEC_ABS(player->c.origin_x-dangerous->c.origin_x)<Rdist)&&
					(player->c.origin_x+halfbox < dangerous->c.origin_x- radius)
			)
			{
				danger = true;
				if(!player->brain.Fdanger)
				{
					player->move.go = true;
					player->brain.Fdanger = true;
					if(0 <= Ud && 0 <= Dd)
						player->c.dir = xrand(2);
					else
						if(0 < Ud)
							player->c.dir = DIR_UP;
						else
							if(0 < Dd)
								player->c.dir = DIR_DOWN;
				}
			}
			else
			{
				if(
						(dangerous->c.dir == DIR_RIGHT) &&
						(VEC_ABS(player->c.origin_x - dangerous->c.origin_x)<Ldist) &&
						(dangerous->c.origin_x + radius < player->c.origin_x - halfbox)
				)
				{
					danger = true;
					if(!player->brain.Fdanger)
					{
						player->move.go = true;
						player->brain.Fdanger = true;
						if(0 <= Ud && 0 <= Dd) player->c.dir = xrand(2);
						else
							if(0 < Ud) player->c.dir = DIR_UP;
							else
								if(0 < Dd) player->c.dir = DIR_DOWN;
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
    bool danger = false;
    entity_t * dangerous;

    size_t i;
    ENTITIES_FOREACH(dangerous, i)
    {
        if(entity_classname_cmp(dangerous, "bull_artillery") == 0)
        {
            danger = ctrl_AI_checkdanger(player, dangerous);
        }
        else if(entity_classname_cmp(dangerous, "bull_missile") == 0)
        {
            danger = ctrl_AI_checkdanger(player, dangerous);
        }
        else if(entity_classname_cmp(dangerous, "bull_mine") == 0)
        {
            danger = ctrl_AI_checkdanger(player, dangerous);
        }
        if(danger)
            break;
    }

    player->brain.danger = danger;
    if(!player->brain.danger)
    {
        player->brain.Fdanger = false;
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
    FLOAT player_bodybox_half = player->c.bodybox * 0.5f;
    if( (player->c.origin_x - player_bodybox_half < target->c.origin_x) && (target->c.origin_x < player->c.origin_x + player_bodybox_half) )
	{
		if(target->c.origin_y < player->c.origin_y)
			player->c.dir = DIR_DOWN;
		else
			player->c.dir = DIR_UP;
		return true;
	}
	if( (player->c.origin_y - player_bodybox_half < target->c.origin_y) && (target->c.origin_y < player->c.origin_y + player_bodybox_half) )
	{
		if(target->c.origin_x < player->c.origin_x)
			player->c.dir = DIR_LEFT;
		else
			player->c.dir = DIR_RIGHT;
		return true;
	}
	return false;
}


/*
 * атака
 */
static void ctrl_AI_attack(entity_t * player, entity_t * target)
{
    map_t * map = world_map_get();

	/* противник в прямой видимости */
	void P_weapon_select_direct_view(entity_t * player)
	{
		if(player->item_ammo_mine > 0)
		{
			//выбираем наугад ракету или мину
			player->brain.weap = 1 + xrand(2);
			if(player->brain.weap == WEAP_MINE)
				player->c.dir = entity_direction_invert(player->c.dir);
		}
		else
		{
			if(player->item_ammo_missile > 0)
				player->brain.weap = WEAP_MISSILE;
			else
				player->brain.weap = WEAP_ARTILLERY;
		}
		player->brain.attack = true;
	}

	/* противник за стеной, пытаемся пробиться через стену */
	void P_weapon_select_behind_wall(entity_t * player, char wall, vec_t dist, vec_t explode_radius)
	{
		if (MAP_WALL_TEXTURE(wall) == MAP_WALL_W0)
			player->brain.attack = false; //сильная броня, не стреляем
		else
		{
			if (MAP_WALL_TEXTURE(wall) == MAP_WALL_brick) /* кирпич */
			{
				player->brain.weap = WEAP_ARTILLERY;
				player->brain.attack = true;
			}
			else
			{
				if (MAP_WALL_TEXTURE(wall) == MAP_WALL_W1) /* слабая броня */
					P_weapon_select_direct_view(player);
			}
		}
		if(
				(dist - player->c.bodybox < explode_radius) &&
				(
						MAP_WALL_CLIPPED(wall) &&
						( MAP_WALL_TEXTURE(wall) == MAP_WALL_W0 || MAP_WALL_TEXTURE(wall) == MAP_WALL_W1)
				)
		)
			player->brain.attack = false;
	}

	FLOAT dist;
	char wall;

    static FLOAT list_explode_diameter[] =
    {
            14.0f, /* artillery */
            22.0f, /* missile */
            22.0f, /* mine */
    };
    FLOAT explode_radius = list_explode_diameter[weapontype_to_explodetype(player->brain.weap)] * 0.5f;

	/* управляемый снаряд */
	if( player->bull && player->brain.target )
	{
	    entity_t * bull = player->bull;
        entity_t * target = player->brain.target;
		if(
				VEC_ABS(bull->c.origin_y - target->c.origin_y) <
				VEC_ABS(bull->c.origin_x - target->c.origin_x)
		)
		{
			if(bull->c.origin_x < target->c.origin_x)
				player->c.dir = DIR_RIGHT;
			else
				player->c.dir = DIR_LEFT;
		}
		else
		{
			if(bull->c.origin_y < target->c.origin_y)
				player->c.dir = DIR_UP;
			else
				player->c.dir = DIR_DOWN;
		}
		return;
	}

	if
	(
			VEC_ABS(player->c.origin_x - target->c.origin_x) > 160.0 ||
			VEC_ABS(player->c.origin_y - target->c.origin_y) > 100.0
	)
	{
		player->attack = false;
		return;
	}

	//если оружие не перезарядилось
	if(0 < player->reloadtime_d)
		return;

	vec_t halfbox = player->c.bodybox * 0.5f;

	player->brain.target = NULL;
	if(player->item_ammo_missile > 0)
	{
		map_clip_find_near_wall(map, player->c.origin, player->c.dir, &dist, &wall);
		if(
				MAP_WALL_CLIPPED(wall) &&
				(
						MAP_WALL_TEXTURE(wall) == MAP_WALL_W0 ||
						( MAP_WALL_TEXTURE(wall) == MAP_WALL_W1 && dist - halfbox < explode_radius )
				)
		)
			player->brain.attack = false;
		else
		{
			player->brain.weap = WEAP_MISSILE;
			player->brain.attack = true;
		}
	}
	else
	{
		if(turn_to_target(player, target))
		{
			map_clip_find_near_wall(map, player->c.origin, player->c.dir, &dist, &wall);
			if(
					( (player->c.dir == DIR_DOWN || player->c.dir == DIR_UP   ) && VEC_ABS(player->c.origin_y - target->c.origin_y) < dist ) ||
					( (player->c.dir == DIR_LEFT || player->c.dir == DIR_RIGHT) && VEC_ABS(player->c.origin_x - target->c.origin_x) < dist ) ||
					!MAP_WALL_CLIPPED(wall) ||
					MAP_WALL_TEXTURE(wall) == MAP_WALL_water
			)
				P_weapon_select_direct_view(player); /* противник в прямой видимости */
			else
				P_weapon_select_behind_wall(player, wall, dist, explode_radius); /* противник за стеной, пытаемся пробиться через стену */
		}

	}
	player->weap = player->brain.weap;
	player->attack = player->brain.attack;
	if(player->brain.weap == WEAP_MISSILE)
		player->brain.target = target; //если выпустил ракету, тогда цель постоянная
	player->brain.attack = false;
}
/*
 * поиск врага
 */
static void ctrl_AI_findenemy(entity_t * player, entity_t * target)
{
	if(
			(160<VEC_ABS(player->c.origin_x-target->c.origin_x))||
			(100<VEC_ABS(player->c.origin_y-target->c.origin_y))
	)
	{
		player->attack = false;
		return;
	}
	if(player->brain.count == 0) {
		player->c.dir = xrand(4);
		player->move.go = true;
	}
	else
	{
		if(BOT_THINK_TIME + xrand(BOT_THINK_TIME) < player->brain.count)
		{
			if(VEC_ABS(player->c.origin_x-target->c.origin_x)>VEC_ABS(player->c.origin_y-target->c.origin_y))
			{
				if(player->c.origin_x < target->c.origin_x)
					player->c.dir = DIR_RIGHT;
				else
					player->c.dir = DIR_LEFT;
			}
			else
			{
				if(player->c.origin_y < target->c.origin_y)
					player->c.dir = DIR_UP;
				else
					player->c.dir = DIR_DOWN;
			}
			if(
					(VEC_ABS(player->c.origin_x - target->c.origin_x) < c_BOT_dist)&&
					(VEC_ABS(player->c.origin_y - target->c.origin_y) < c_BOT_dist)
			)
				player->move.go = false;
			else
				player->move.go = true;
		}
	}
	player->brain.count += gi->dtime();
	if(BOT_THINK_TIME * 2 < player->brain.count)
		player->brain.count = 0;
}

/*
 * управление вражеским игроком
 */
void think_enemy(entity_t * player)
{
	if(debug_noAI)
		return;
	if(!player->c.alive)
		return;
	ctrl_AI_checkdangers(player);
	if(player->brain.danger)
		return;
	entity_t * target = entity_get_random("player");

	if(!target)
		return;

	if(!target->c.alive)
		player->attack = false;
	else
	{
		if(!player->bull && !player->attack)
			ctrl_AI_findenemy(player, target);
		ctrl_AI_attack(player, target);
	}
}
