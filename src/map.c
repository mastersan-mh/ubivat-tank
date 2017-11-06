/*
 *        Ubivat Tank
 *        обработка карт
 * by Master San
 */

#include "map.h"
#include "entity.h"
#include "video.h"
#include "game.h"
#include "img.h"
#include "fonts.h"
#include "utf8.h"
#include "system.h"
#include "types.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

// карта
#define MAP_FILE_EXT    ".mut"
// Map Ubivat Tank
#define MAP_DATA_HEADER "MUT"

char * map_class_names[MAPDATA_MOBJ_NUM] =
{
		"SPAWN.PLAYER",
		"SPAWN.ENEMY" ,
		"SPAWN.BOSS"  ,
		"ITEM.HEALTH" ,
		"ITEM.ARMOR"  ,
		"ITEM.STAR"   ,
		"ITEM.ROCKET" ,
		"ITEM.MINE"   ,
		"OBJ.EXIT"    ,
		"OBJ.MESS"    ,
		NULL
};

//список карт
maplist_t * mapList;

//карта
map_t map = {};

enum
{
	MAP_ERROR_NO,
	MAP_ERR_NOFILE,
	MAP_ERR_READ,
	MAP_ERR_FORMAT
};
static int map_error;

maplist_t * map_find(const char * name)
{
	maplist_t * gamemap = mapList;
	while(gamemap)
	{
		if(!strcmp(name, gamemap->map))
			return gamemap;
		gamemap = gamemap->next;
	}
	return NULL;
}

int map_error_get(void)
{
	return map_error;
}

void map_init(void)
{
	/* empty */
}

/*
 * добавление объекта
 */
static void map_mobj_add(mapdata_entity_type_t mapdata_mobj_type, map_data_entity_t * data)
{

/* предмет не используется */
#define PLAYER_ITEM_AMOUNT_NA  (0)
/* бесконечно */
#define PLAYER_ITEM_AMOUNT_INF (-1)

#define STRBUFSIZE 2048
	static char strbuf[STRBUFSIZE];
	entity_t * entity;
	switch(mapdata_mobj_type)
	{
	case MAPDATA_MOBJ_SPAWN_PLAYER:
		entity = entity_new("spawn_player", data->pos.x, data->pos.y, DIR_UP, NULL);
		ENTITY_VARIABLE_INTEGER(entity, "item_scores") = data->spawn.scores;
		ENTITY_VARIABLE_INTEGER(entity, "item_health") = data->spawn.health;
		ENTITY_VARIABLE_INTEGER(entity, "item_armor")  = data->spawn.armor;
		ENTITY_VARIABLE_INTEGER(entity, "item_ammo_missile") = PLAYER_ITEM_AMOUNT_NA;
		ENTITY_VARIABLE_INTEGER(entity, "item_ammo_mine")    = PLAYER_ITEM_AMOUNT_NA;
		break;
	case MAPDATA_MOBJ_SPAWN_ENEMY:
		entity = entity_new("spawn_enemy", data->pos.x, data->pos.y, DIR_UP, NULL);
		ENTITY_VARIABLE_INTEGER(entity, "item_scores") = data->spawn.scores;
		ENTITY_VARIABLE_INTEGER(entity, "item_health") = data->spawn.health;
		ENTITY_VARIABLE_INTEGER(entity, "item_armor")  = data->spawn.armor;
		ENTITY_VARIABLE_INTEGER(entity, "item_ammo_missile") = PLAYER_ITEM_AMOUNT_NA;
		ENTITY_VARIABLE_INTEGER(entity, "item_ammo_mine")    = PLAYER_ITEM_AMOUNT_NA;
		break;
	case MAPDATA_MOBJ_SPAWN_BOSS:
		entity = entity_new("spawn_boss", data->pos.x, data->pos.y, DIR_UP, NULL);
		ENTITY_VARIABLE_INTEGER(entity, "item_scores") = data->spawn.scores;
		ENTITY_VARIABLE_INTEGER(entity, "item_health") = data->spawn.health;
		ENTITY_VARIABLE_INTEGER(entity, "item_armor")  = data->spawn.armor;
		ENTITY_VARIABLE_INTEGER(entity, "item_ammo_missile") = PLAYER_ITEM_AMOUNT_NA;
		ENTITY_VARIABLE_INTEGER(entity, "item_ammo_mine")    = PLAYER_ITEM_AMOUNT_NA;
		break;
	case MAPDATA_MOBJ_ITEM_HEALTH :
		entity = entity_new("item_health", data->pos.x, data->pos.y, DIR_UP, NULL);
		ENTITY_VARIABLE_INTEGER(entity, "amount") = data->item.amount;
		break;
	case MAPDATA_MOBJ_ITEM_ARMOR:
		entity = entity_new("item_armor", data->pos.x, data->pos.y, DIR_UP, NULL);
		ENTITY_VARIABLE_INTEGER(entity, "amount") = data->item.amount;
		break;
	case MAPDATA_MOBJ_ITEM_STAR:
		entity = entity_new("item_scores", data->pos.x, data->pos.y, DIR_UP, NULL);
		ENTITY_VARIABLE_INTEGER(entity, "amount") = data->item.amount;
		break;
	case MAPDATA_MOBJ_ITEM_ROCKET:
		entity = entity_new("item_ammo_missile", data->pos.x, data->pos.y, DIR_UP, NULL);
		ENTITY_VARIABLE_INTEGER(entity, "amount") = data->item.amount;
		break;
	case MAPDATA_MOBJ_ITEM_MINE:
		entity = entity_new("item_ammo_mine", data->pos.x, data->pos.y, DIR_UP, NULL);
		ENTITY_VARIABLE_INTEGER(entity, "amount") = data->item.amount;
		break;
	case MAPDATA_MOBJ_OBJ_EXIT:
		strn_cpp866_to_utf8(strbuf, STRBUFSIZE - 1, data->obj.message);
		entity = entity_new("exit", data->pos.x, data->pos.y, DIR_UP, NULL);
		ENTITY_VARIABLE_STRING(entity, "text") = VARIABLE_STRING_DUP(strbuf);
		break;
	case MAPDATA_MOBJ_OBJ_MESS:
		strn_cpp866_to_utf8(strbuf, STRBUFSIZE - 1, data->obj.message);
		entity = entity_new("message", data->pos.x, data->pos.y, DIR_UP, NULL);
		ENTITY_VARIABLE_STRING(entity, "text") = VARIABLE_STRING_DUP(strbuf);
		break;
	default: ;
	}
#undef STRBUFSIZE
}




/*
 * поиск препятствия на карте
 */
void map_clip_find(
	const vec2_t origin,
	vec_t bodybox,
	char mask,
	bool * Ul,
	bool * Ur,
	bool * Dl,
	bool * Dr,
	bool * Lu,
	bool * Ld,
	bool * Ru,
	bool * Rd
)
{

	if(
			origin[0] < 0 || MAP_SX * MAP_WALLBLOCKSIZE < origin[0] ||
			origin[1] < 0 || MAP_SY * MAP_WALLBLOCKSIZE < origin[1]
	)
	{
		*Ul = true; *Ur = true;
		*Dl = true; *Dr = true;
		*Lu = true; *Ld = true;
		*Ru = true; *Rd = true;
		return;
	}

	*Ul = false; *Ur = false;
	*Dl = false; *Dr = false;
	*Lu = false; *Ld = false;
	*Ru = false; *Rd = false;

	vec_t hbox = bodybox/2;

	vec_t x_add_hbox = origin[0] + hbox;
	vec_t x_sub_hbox = origin[0] - hbox;

	vec_t y_add_hbox = origin[1] + hbox;
	vec_t y_sub_hbox = origin[1] - hbox;

	int x_add_hbox_8 = VEC_TRUNC(x_add_hbox/8);
	int x_sub_hbox_8 = VEC_TRUNC(x_sub_hbox/8);

	int y_add_hbox_8 = VEC_TRUNC((y_add_hbox)/8);
	int y_sub_hbox_8 = VEC_TRUNC((y_sub_hbox)/8);

	vec_t count = 0;
	while(
			(count <= hbox)&&
			(
					!(
							*Ul && *Ur &&
							*Dl && *Dr &&
							*Lu && *Ld &&
							*Ru && *Rd
					)
			)
	)
	{

		vec_t x_sub_hbox_add_count = x_sub_hbox + count;
		vec_t x_add_hbox_sub_count = x_add_hbox - count;

		int x_sub_hbox_add_count_8 = VEC_TRUNC(x_sub_hbox_add_count/8);
		int x_add_hbox_sub_count_8 = VEC_TRUNC(x_add_hbox_sub_count/8);

		vec_t y_add_hbox_sub_count = y_add_hbox - count;
		vec_t y_sub_hbox_add_count = y_sub_hbox + count;

		int y_add_hbox_sub_count_8 = VEC_TRUNC((y_add_hbox_sub_count)/8);
		int y_sub_hbox_add_count_8 = VEC_TRUNC((y_sub_hbox_add_count)/8);

		if(
				MAP_WALL_CLIPPED( map.map[y_add_hbox_8][x_sub_hbox_add_count_8] ) &&
				(          mask & map.map[y_add_hbox_8][x_sub_hbox_add_count_8] )    ) *Ul = true;
		if(
				( map.map[y_add_hbox_8][x_add_hbox_sub_count_8] & MAP_WALL_CLIP ) &&
				( map.map[y_add_hbox_8][x_add_hbox_sub_count_8] & mask )) *Ur = true;
		if(
				( map.map[y_sub_hbox_8][x_sub_hbox_add_count_8] & MAP_WALL_CLIP ) &&
				( map.map[y_sub_hbox_8][x_sub_hbox_add_count_8] & mask )) *Dl = true;
		if(
				( map.map[y_sub_hbox_8][x_add_hbox_sub_count_8] & MAP_WALL_CLIP ) &&
				( map.map[y_sub_hbox_8][x_add_hbox_sub_count_8] & mask )) *Dr = true;
		if(
				( map.map[y_add_hbox_sub_count_8][x_sub_hbox_8] & MAP_WALL_CLIP ) &&
				( map.map[y_add_hbox_sub_count_8][x_sub_hbox_8] & mask )) *Lu = true;
		if(
				( map.map[y_sub_hbox_add_count_8][x_sub_hbox_8] & MAP_WALL_CLIP ) &&
				( map.map[y_sub_hbox_add_count_8][x_sub_hbox_8] & mask )) *Ld = true;
		if(
				( map.map[y_add_hbox_sub_count_8][x_add_hbox_8] & MAP_WALL_CLIP ) &&
				( map.map[y_add_hbox_sub_count_8][x_add_hbox_8] & mask )) *Ru = true;
		if(
				( map.map[y_sub_hbox_add_count_8][x_add_hbox_8] & MAP_WALL_CLIP ) &&
				( map.map[y_sub_hbox_add_count_8][x_add_hbox_8] & mask )) *Rd = true;
		count++;
	}
}

/*
 * вычисление расстояния до ближайшей стены
 */
void map_clip_find_near(const vec2_t origin, vec_t bodybox, direction_t dir, char mask, vec_t maxdist, vec_t * dist)
{
	char wall;
	vec_t halfbox = bodybox/2;
	size_t mapx;
	size_t mapy;
	vec_t oy = origin_y;
	vec_t ox = origin_x;

	switch(dir)
	{
		case DIR_UP: case DIR_DOWN:
			if(MAP_SY * MAP_WALLBLOCKSIZE < maxdist) maxdist = MAP_SY * MAP_WALLBLOCKSIZE;
			break;
		case DIR_LEFT: case DIR_RIGHT:
			if(MAP_SX * MAP_WALLBLOCKSIZE < maxdist) maxdist = MAP_SX * MAP_WALLBLOCKSIZE;
			break;
	}

	vec_t ddist = halfbox/2;
	do
	{
		ddist++;

		vec_t dwidth = -halfbox+1;
		do
		{

			vec_t dx;
			vec_t dy;

			switch(dir)
			{
				case DIR_UP   : dx = dwidth; dy =  ddist; break;
				case DIR_DOWN : dx = dwidth; dy = -ddist; break;
				case DIR_LEFT : dx = -ddist; dy = dwidth; break;
				case DIR_RIGHT: dx =  ddist; dy = dwidth; break;
			}

			mapy = VEC_TRUNC((oy + dy) / MAP_WALLBLOCKSIZE);
			mapx = VEC_TRUNC((ox + dx) / MAP_WALLBLOCKSIZE);
			wall = map.map[mapy][mapx];
			dwidth++;
		} while ( (dwidth < halfbox ) && !(wall & mask) );
	} while     ( (ddist  < maxdist ) && !(wall & mask) );

	*dist = ddist;
}

/*
 * вычисление расстояния до ближайшей стены и определение стены
 */
void map_clip_find_near_wall(const vec2_t origin, direction_t dir, vec_t * dist, char * wall)
{
	vec_t oy = origin_y;
	vec_t ox = origin_x;
	size_t y;
	size_t x;
	*dist = 0;
	vec_t maxdist;

	switch(dir)
	{
		case DIR_UP   : maxdist = MAP_SY * MAP_WALLBLOCKSIZE; break;
		case DIR_DOWN : maxdist = MAP_SY * MAP_WALLBLOCKSIZE; break;
		case DIR_LEFT : maxdist = MAP_SX * MAP_WALLBLOCKSIZE; break;
		case DIR_RIGHT: maxdist = MAP_SX * MAP_WALLBLOCKSIZE; break;
	}

	do
	{
		y = VEC_TRUNC(oy / MAP_WALLBLOCKSIZE);
		x = VEC_TRUNC(ox / MAP_WALLBLOCKSIZE);
		*wall = map.map[y][x];
		switch(dir)
		{
			case DIR_UP   : oy += 1.0f; break;
			case DIR_DOWN : oy -= 1.0f; break;
			case DIR_LEFT : ox -= 1.0f; break;
			case DIR_RIGHT: ox += 1.0f; break;
		}
		(*dist)++;
	} while( (*dist) < maxdist && !MAP_WALL_CLIPPED((*wall)) );
}

static mapdata_entity_type_t map_entity_name_to_entity_type(const char * class)
{
	int mapdata_mobj_type;
	for(mapdata_mobj_type = 0; mapdata_mobj_type < MAPDATA_MOBJ_NUM; mapdata_mobj_type++)
	{
		if(strcmp(class, map_class_names[mapdata_mobj_type]) == 0) return mapdata_mobj_type;
	}
	return MAPDATA_MOBJ_UNKNOWN;
}

/*
 * чтение класса предмета
 */
mapdata_entity_type_t map_file_class_get(int fd)
{
	char class[256];
	char ch;
	int i = 0;
	ssize_t count;
	do
	{
		count = read(fd, &ch, 1);
		class[i] = ch;
		i++;
	}while(count != 0 && ch!=0 );
	return map_entity_name_to_entity_type(class);
}


/**
 * чтение класса предмета
 */
static int map_load_mobj(int fd, mapdata_entity_type_t * mapdata_mobj_type, map_data_entity_t * data)
{
	static size_t datasize[MAPDATA_MOBJ_NUM] =
	{
			sizeof(map_data_spawn_t),	/* MAP_SPAWN_PLAYER */
			sizeof(map_data_spawn_t),	/* MAP_SPAWN_ENEMY */
			sizeof(map_data_spawn_t),	/* MAP_SPAWN_BOSS */
			sizeof(map_data_item_t),	/* MAP_ITEM_HEALTH */
			sizeof(map_data_item_t),	/* MAP_ITEM_ARMOR */
			sizeof(map_data_item_t),	/* MAP_ITEM_STAR */
			sizeof(map_data_item_t),	/* MAP_ITEM_ROCKET */
			sizeof(map_data_item_t),	/* MAP_ITEM_MINE */
			sizeof(map_data_obj_t),	/* MAP_OBJ_EXIT */
			sizeof(map_data_obj_t)	/* MAP_OBJ_MESS */
	};
	char class[256];
	char ch;
	int i = 0;
	ssize_t count;
	do
	{
		count = read(fd, &ch, 1);
		class[i++] = ch;
	}while(count != 0 && ch != 0);
	if(count == 0) return 1;

	*mapdata_mobj_type = map_entity_name_to_entity_type(class);
	if(*mapdata_mobj_type == MAPDATA_MOBJ_UNKNOWN)
	{
		game_console_send("map load error: no map object type %s", class);
		return -1;
	}
	count = read(fd, data, datasize[*mapdata_mobj_type]);
	if(count == 0) return 1;
	if(count != datasize[*mapdata_mobj_type])return 1;
	return 0;
}

/*
 * открытие и чтение карты
 */
//map_load=0 -нет ошибок
//map_load=1 -файл карты не найден
//map_load=2 -ошибка формата
//map_load=3 -ошибка чтения(файл поврежден)
//map_load=4 -не найден спавн-поинт для GAME игры
//map_load=5 -не найден спавн-поинт для CASE игры
int map_load(const char * mapname)
{

	bool parental_lock = !strcmp(mapname, "map02");

	if(map.loaded) return -1;
#define RETURN_ERR(err) \
		do{ \
			close(fd); \
			map_error = (err); \
			return -1; \
		}while(0);
#define STRBUFSIZE 2048
	static char buf[STRBUFSIZE];
	int fd;
	int ret;
	char *path;
	size_t len;

	map._file = Z_strdup(mapname);

	path = Z_malloc(
		strlen(BASEDIR MAPSDIR "/")+
		strlen(map._file)+
		strlen(MAP_FILE_EXT)+
		1
	);
	strcpy(path, BASEDIR MAPSDIR "/");
	strcat(path, map._file);
	strcat(path, MAP_FILE_EXT);
	fd = open(path, O_RDONLY);
	Z_free(path);
	if(fd < 0) RETURN_ERR(MAP_ERR_NOFILE);

	map_data_header_t header;

	ssize_t count;
	count = read(fd, &header, sizeof(header));
	if(count != sizeof(header)) RETURN_ERR(MAP_ERR_READ);

	ret = memcmp(header.sign, MAP_DATA_HEADER, sizeof(header.sign));
	if(ret) RETURN_ERR(MAP_ERR_FORMAT);

	//название карты
	len = strn_cpp866_to_utf8(buf, STRBUFSIZE - 1, header.name);
	map.name = Z_strndup(buf, len);

	//краткое описание
	len = strn_cpp866_to_utf8(buf, STRBUFSIZE - 1, header.brief);
	map.brief = Z_strndup(buf, len);

	//чтение карты
	count = read(fd, map.map, MAP_SX * MAP_SY);
	if(count != MAP_SX * MAP_SY) RETURN_ERR(MAP_ERR_READ);

	bool player_spawn_exist = false;
	int i = 0;
	for(;;)
	{
		mapdata_entity_type_t mapdata_mobj_type;
		map_data_entity_t data;
		int ret = map_load_mobj(fd, &mapdata_mobj_type, &data);
		if(ret) break;
		if(mapdata_mobj_type == MAPDATA_MOBJ_SPAWN_PLAYER) player_spawn_exist = true;

		i++;
		if (
				parental_lock &&
				(
						i == 12 || i == 30
				)
		) continue;
		map_mobj_add(mapdata_mobj_type, &data);
	}
	close(fd);
	map.loaded = true;
	if(!player_spawn_exist || ret < 0)
	{
		map.loaded = true;
		map_clear();
		RETURN_ERR(MAP_ERR_READ);
	}
	return 0;
};
/*
 * закрытие карты
 */
void map_clear(void)
{
	if(!map.loaded)
	    return;
	Z_FREE(map._file);
	Z_FREE(map.name);
	Z_FREE(map.brief);
	entities_erase();
	map.loaded = false;
}

/*
 * вывод байтового образа bytemap размерами full_x,full_y, в позицию
 * out_x,out_y на экран с позиции get_x,get_y, размером sx,sy
 */
static void map_wall_render(
	camera_t * cam,
	float pos_x,
	float pos_y,
	const item_img_t * image
)
{

	float translation_x = 0.0f;
	float translation_y = 0.0f;
	float modelscale = 1.0f;

	vec_t tr_x = ( cam->x + cam->sx / 2 + (pos_x - cam->origin[0]) + translation_x ) * VIDEO_SCALE;
	vec_t tr_y = ( cam->y + cam->sy / 2 - (pos_y - cam->origin[1]) + translation_y ) * VIDEO_SCALE;

	vec_t modelscale_x = modelscale * VIDEO_SCALE;
	vec_t modelscale_y = modelscale * VIDEO_SCALE;

	GLfloat mdl_sx = image->img_sx * modelscale_x;
	GLfloat mdl_sy = image->img_sy * modelscale_y;

	GLfloat texture_x1 = image->img_sx/image->texture_sx;
	GLfloat texture_y1 = image->img_sy/image->texture_sy;
/*
	int sfactor = list[game_video_sfactor];
	int dfactor = list[game_video_dfactor];
	glBlendFunc(sfactor, dfactor);
*/

	glBlendFunc(image->sfactor, image->dfactor);

	glBindTexture(GL_TEXTURE_2D, image->texture);
	glLoadIdentity();
	glTranslatef(tr_x, tr_y , 0.0f);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(texture_x1, texture_y1); glVertex2f(mdl_sx, mdl_sy); // Верхний правый угол квадрата
	glTexCoord2f(texture_x1, 0.0f      ); glVertex2f(mdl_sx, 0.0f  ); // Нижний правый
	glTexCoord2f(0.0f      , 0.0f      ); glVertex2f(0.0f  , 0.0f  ); // Нижний левый
	glTexCoord2f(0.0f      , texture_y1); glVertex2f(0.0f  , mdl_sy); // Верхний левый
	glEnd();
}


/*
 * рисование объектов на карте
 */
void map_draw(camera_t * cam)
{
	static image_index_t water_images[] =
	{
			IMG_WATER0,
			IMG_WATER1,
			IMG_WATER2,
	};

	int x, y;
	int x0,x1;
	int y0,y1;

	x0 = VEC_TRUNC((cam->origin[0]-(cam->sx / 2))/8);
	if(x0 < 0) x0 = 0;
	y0 = VEC_TRUNC((cam->origin[1]-(cam->sy / 2))/8);
	if(y0 < 0) y0 = 0;
	x1 = x0 + (cam->sx / 8) + 1;
	if(MAP_SX < x1) x1 = MAP_SX;
	y1 = y0 + (cam->sy / 8) + 1;
	if(MAP_SY < y1) y1 = MAP_SY;

	for(y = y0; y < y1; y++)
	{
		for(x = x0; x < x1; x++ )
		{
			const item_img_t * img = NULL;
			switch(MAP_WALL_TEXTURE(map.map[y][x]))
			{
				case MAP_WALL_W0   : img = image_get(IMG_WALL_W0); break;
				case MAP_WALL_W1   : img = image_get(IMG_WALL_W1); break;
				case MAP_WALL_brick: img = image_get(IMG_WALL_BRICK); break;
				case MAP_WALL_water: img = image_get(water_images[xrand(3)]); break;
			}
			if(img)
			{
				float pos_x = (x  ) * 8;
				float pos_y = (y+1) * 8;
				map_wall_render(cam, pos_x, pos_y, img);
			}
		}
	}

	entities_render(cam);

}
/*
 * добавление карты в список
 */
void map_list_add(const char *map, const char * name)
{
	maplist_t * mapEnt, * p;


	mapEnt = Z_malloc(sizeof(*mapEnt));
	mapEnt->prev = NULL;
	mapEnt->next = NULL;
	mapEnt->map  = Z_strdup(map);
	mapEnt->name = Z_strdup(name);
	if(mapList == NULL)
		mapList = mapEnt;
	else
	{
		p = mapList;
		while(p->next)
			p = p->next;
		mapEnt->prev = p;
		p->next = mapEnt;
	}
}
/*
 * удаление списка карт
 */
void map_list_removeall(void)
{
	maplist_t * mapEnt;
	while(mapList)
	{
		mapEnt  = mapList;
		Z_free(mapEnt->map);
		Z_free(mapEnt->name);
		mapList = mapList->prev;
		Z_free(mapEnt);
	}
}

/*
 * чтение из файла списка карт
 */
void map_load_list(void)
{
	printf("Maps list loading: ");

	FILE *fconf;

	size_t str_size = 256;
	char * map = NULL;

	if ((fconf = fopen(BASEDIR FILENAME_MAPSLIST, "r")) == NULL)
	{
		game_halt("Could not load %s", BASEDIR FILENAME_MAPSLIST);
	}

	mapList = NULL;
	bool isString = false;
	char * str = Z_calloc(1, str_size);
	if(!str)
	{
		game_halt("Maps list is empty");
	}
	while(fconf)
	{

		size_t str_len = strlen(str);
		if(str_size <= str_len + 1)
		{
			char * tmp = Z_realloc(str, str_size + 256);
			if (tmp)
			{
				str = tmp;
				str_size += 256;
			}
			else
			{
				Z_free(str);
				fclose(fconf);
				game_halt("Failed to allocate memory\n");
			}
		}

		int c = fgetc(fconf);
		if(c < 0)break;
		if(!isString)
		{
			switch(c)
			{
				case ' ':
				case '\n':
				case '\r': break;
				case '"': isString = true;break;
				default:
					if(str)Z_free(str);
					fclose(fconf);
					game_halt("File \"%s\" format error.\n", FILENAME_MAPSLIST);
			}
		}
		else
		{
			if(c != '"')
			{
				str[str_len++] = (c);
				str[str_len  ] = 0;
			}
			else
			{
				isString = false;
				if(!map)
				{
					map = Z_strdup(str);
					if(!map)
					{
						game_halt("Failed to allocate memory\n");
					}
					str[0] = 0;
				}
				else
				{
					map_list_add(map, str);
					Z_free(map);
					map = NULL;
					str[0] = 0;
				}

			}
		}
	};
	if(str)Z_free(str);
	if(map)Z_free(map);
	if(isString)
	{
		game_halt("File \"%s\" format error.\n", FILENAME_MAPSLIST);
	}
	printf("OK\n");
}
