/*
 *        Ubivat Tank
 *        обработка карт
 * by Master San
 */

#include "ent_explode.h"
#include "ent_spawn.h"

#include "video.h"
#include "game.h"
#include "img.h"
#include "_gr2D.h"
#include "fonts.h"
#include "utf8.h"


#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include <types.h>

// карта
#define MAP_FILE_EXT    ".mut"
// Map Ubivat Tank
#define MAP_DATA_HEADER "MUT"

char * map_class_names[__MAPDATA_MOBJ_NUM] =
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

int map_error_get()
{
	return map_error;
}

void map_clear();

void map_init()
{
	map.mobjs = NULL;
}


/*
 * добавление объекта
 */
static void map_mobj_add(mapdata_mobj_type_t mapdata_mobj_type, map_data_mobj_t * data)
{
#define BUFSIZE 2048
	static char buf[BUFSIZE];
	int i;
	int value_int;
	spawn_t spawn;
	switch(mapdata_mobj_type)
	{
	case MAPDATA_MOBJ_SPAWN_PLAYER:
		for(i = 0; i < __ITEM_NUM; i++) spawn.items[i] = ITEM_AMOUNT_NA;
		spawn.items[ITEM_SCORES] = data->spawn.scores;
		spawn.items[ITEM_HEALTH] = data->spawn.health;
		spawn.items[ITEM_ARMOR]  = data->spawn.armor;
		mobj_new(MOBJ_SPAWN_PLAYER, data->pos.x, data->pos.y, DIR_UP, NULL, &spawn);
		break;
	case MAPDATA_MOBJ_SPAWN_ENEMY:
		for(i = 0; i < __ITEM_NUM; i++) spawn.items[i] = ITEM_AMOUNT_NA;
		spawn.items[ITEM_SCORES] = data->spawn.scores;
		spawn.items[ITEM_HEALTH] = data->spawn.health;
		spawn.items[ITEM_ARMOR]  = data->spawn.armor;
		mobj_new(MOBJ_SPAWN_ENEMY, data->pos.x, data->pos.y, DIR_UP, NULL, &spawn);
		break;
	case MAPDATA_MOBJ_SPAWN_BOSS:
		for(i = 0; i < __ITEM_NUM; i++) spawn.items[i] = ITEM_AMOUNT_NA;
		spawn.items[ITEM_SCORES] = data->spawn.scores;
		spawn.items[ITEM_HEALTH] = data->spawn.health;
		spawn.items[ITEM_ARMOR]  = data->spawn.armor;
		mobj_new(MOBJ_SPAWN_BOSS, data->pos.x, data->pos.y, DIR_UP, NULL, &spawn);
		break;
	case MAPDATA_MOBJ_ITEM_HEALTH :
		value_int = data->item.amount;
		mobj_new(MOBJ_ITEM_HEALTH, data->pos.x, data->pos.y, DIR_UP, NULL, &value_int);
		break;
	case MAPDATA_MOBJ_ITEM_ARMOR:
		value_int = data->item.amount;
		mobj_new(MOBJ_ITEM_ARMOR, data->pos.x, data->pos.y, DIR_UP, NULL, &value_int);
		break;
	case MAPDATA_MOBJ_ITEM_STAR:
		value_int = data->item.amount;
		mobj_new(MOBJ_ITEM_SCORES, data->pos.x, data->pos.y, DIR_UP, NULL, &value_int);
		break;
	case MAPDATA_MOBJ_ITEM_ROCKET:
		value_int = data->item.amount;
		mobj_new(MOBJ_ITEM_AMMO_MISSILE, data->pos.x, data->pos.y, DIR_UP, NULL, &value_int);
		break;
	case MAPDATA_MOBJ_ITEM_MINE:
		value_int = data->item.amount;
		mobj_new(MOBJ_ITEM_AMMO_MINE, data->pos.x, data->pos.y, DIR_UP, NULL, &value_int);
		break;
	case MAPDATA_MOBJ_OBJ_EXIT:
		strn_cpp866_to_utf8(buf, BUFSIZE - 1, data->obj.message);
		mobj_new(MOBJ_EXIT, data->pos.x, data->pos.y, DIR_UP, NULL, buf);
		break;
	case MAPDATA_MOBJ_OBJ_MESS:
		strn_cpp866_to_utf8(buf, BUFSIZE - 1, data->obj.message);
		mobj_new(MOBJ_MESSAGE, data->pos.x, data->pos.y, DIR_UP, NULL, buf);
		break;
	default: ;
	}
#undef BUFSIZE
}




/*
 * поиск препятствия на карте
 */
void map_clip_find(
	vec2_t * orig,
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
			orig->x < 0 || MAP_SX * MAP_BLOCKSIZE < orig->x ||
			orig->y < 0 || MAP_SY * MAP_BLOCKSIZE < orig->y
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

	vec_t x_add_hbox = orig->x + hbox;
	vec_t x_sub_hbox = orig->x - hbox;

	vec_t y_add_hbox = orig->y + hbox;
	vec_t y_sub_hbox = orig->y - hbox;

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
				((map.map[y_add_hbox_8][x_sub_hbox_add_count_8]& 0xF0) != 0) &&
				((map.map[y_add_hbox_8][x_sub_hbox_add_count_8]& mask) != 0)) *Ul = true;
		if(
				((map.map[y_add_hbox_8][x_add_hbox_sub_count_8]& 0xF0) != 0) &&
				((map.map[y_add_hbox_8][x_add_hbox_sub_count_8]& mask) != 0)) *Ur = true;
		if(
				((map.map[y_sub_hbox_8][x_sub_hbox_add_count_8]& 0xF0) != 0) &&
				((map.map[y_sub_hbox_8][x_sub_hbox_add_count_8]& mask) != 0)) *Dl = true;
		if(
				((map.map[y_sub_hbox_8][x_add_hbox_sub_count_8]& 0xF0) != 0) &&
				((map.map[y_sub_hbox_8][x_add_hbox_sub_count_8]& mask) != 0)) *Dr = true;
		if(
				((map.map[y_add_hbox_sub_count_8][x_sub_hbox_8]& 0xF0) != 0) &&
				((map.map[y_add_hbox_sub_count_8][x_sub_hbox_8]& mask) != 0)) *Lu = true;
		if(
				((map.map[y_sub_hbox_add_count_8][x_sub_hbox_8]& 0xF0) != 0) &&
				((map.map[y_sub_hbox_add_count_8][x_sub_hbox_8]& mask) != 0)) *Ld = true;
		if(
				((map.map[y_add_hbox_sub_count_8][x_add_hbox_8]& 0xF0) != 0) &&
				((map.map[y_add_hbox_sub_count_8][x_add_hbox_8]& mask) != 0)) *Ru = true;
		if(
				((map.map[y_sub_hbox_add_count_8][x_add_hbox_8]& 0xF0) != 0) &&
				((map.map[y_sub_hbox_add_count_8][x_add_hbox_8]& mask) != 0)) *Rd = true;
		count++;
	}
}
/*
 * вычисление расстояния до ближайшей стены
 */
void map_clip_find_near(vec2_t * orig, vec_t box, int dir, char mask, vec_t DISTmax, vec_t * dist)
{
	char wall;
	float c;
	box = box/2;
	*dist = box/2;
	int mapx;
	int mapy;
	switch(dir)
	{
	case DIR_UP:
		if(MAP_SY*8<DISTmax) DISTmax = MAP_SY * 8;
		do
		{
			(*dist)++;
			c = -box+1;
			do
			{
				wall = map.map[(int)VEC_TRUNC((orig->y+(*dist))/8)][(int)VEC_TRUNC((orig->x+c   )/8)];
				c++;
			}while (!((+box<=c)||((wall & mask) != 0)));
		}while(!((DISTmax<=(*dist))||((wall & mask) != 0)));
		break;
	case DIR_DOWN:
		if(MAP_SY*8<DISTmax) DISTmax = MAP_SY*8;
		do
		{
			(*dist)++;
			c = -box+1;
			do
			{
				mapy = VEC_TRUNC((orig->y-(*dist))/8);
				mapx = VEC_TRUNC((orig->x+c   )/8);
				wall = map.map[mapy][mapx];
				c++;
			}while(!((+box<=c)||((wall & mask) != 0)));
		}while(!((DISTmax<=(*dist))||((wall & mask) != 0)));
		break;
	case DIR_LEFT:
		if(MAP_SX*8<DISTmax) DISTmax = MAP_SX*8;
		do
		{
			(*dist)++;
			c = -box+1;
			do
			{
				wall = map.map[(int)VEC_TRUNC((orig->y+c   )/8)][(int)VEC_TRUNC((orig->x-(*dist))/8)];
				c++;
			}while(!( (+box<=c)||((wall & mask) != 0)));
		}while(!( (DISTmax<=(*dist))||((wall & mask) != 0)));
		break;
	case DIR_RIGHT:
		if(MAP_SX*8<DISTmax) DISTmax = MAP_SX*8;
		do
		{
			(*dist)++;
			c = -box+1;
			do
			{
				wall = map.map[(int)VEC_TRUNC((orig->y+c   )/8)][(int)VEC_TRUNC((orig->x+(*dist))/8)];
				c++;
			}while(!( (+box<=c)||((wall & mask) != 0)));
		}while(!( (DISTmax*8<=(*dist)) || (wall & mask ) ));
		break;
	}
}
/*
 * вычисление расстояния до ближайшей стены и определение стены
 */
void map_clip_find_near_wall(vec2_t * orig, int dir, float * dist, char * wall)
{
	*dist = 0;
	switch(dir)
	{
	case 0:
		do
		{
			*wall = map.map[(int)VEC_TRUNC((orig->y+(*dist))/8)][(int)VEC_TRUNC((orig->x     )/8)];
			(*dist)++;
		}while(!( (MAP_SY*8<=(*dist))||((*wall & 0xF0) != 0)));
		break;
	case 1:
		do
		{
			*wall = map.map[(int)VEC_TRUNC((orig->y-(*dist))/8)][(int)VEC_TRUNC((orig->x     )/8)];
			(*dist)++;
		}while(!( (MAP_SY*8<=(*dist))||((*wall & 0xF0) != 0)));
		break;
	case 2:
		do
		{
			*wall = map.map[(int)VEC_TRUNC((orig->y     )/8)][(int)VEC_TRUNC((orig->x-(*dist))/8)];
			(*dist)++;
		}while(!( (MAP_SX*8<=(*dist))||((*wall & 0xF0) != 0)));
		break;
	case 3:
		do
		{
			*wall = map.map[(int)VEC_TRUNC((orig->y     )/8)][(int)VEC_TRUNC((orig->x+(*dist))/8)];
			(*dist)++;
		}while(!( (MAP_SX*8<=(*dist))||((*wall & 0xF0) != 0)));
		break;
	}
}

static mapdata_mobj_type_t mobj_type_name_to_value(const char * class)
{
	int mapdata_mobj_type;
	for(mapdata_mobj_type = 0; mapdata_mobj_type < __MAPDATA_MOBJ_NUM; mapdata_mobj_type++)
	{
		if(strcmp(class, map_class_names[mapdata_mobj_type]) == 0) return mapdata_mobj_type;
	}
	return MAPDATA_MOBJ_UNKNOWN;
}



/*
 * чтение класса предмета
 */
mapdata_mobj_type_t map_file_class_get(int fd)
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
	mobj_type_t mobj_type = mobj_type_name_to_value(class);
	return mobj_type;
}


/**
 * чтение класса предмета
 */
static int map_load_mobj(int fd, mapdata_mobj_type_t * mapdata_mobj_type, map_data_mobj_t * data)
{
	static size_t datasize[__MAPDATA_MOBJ_NUM] =
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

	*mapdata_mobj_type = mobj_type_name_to_value(class);
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
#define BUFSIZE 2048
	static char buf[BUFSIZE];
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
	len = strn_cpp866_to_utf8(buf, BUFSIZE - 1, header.name);
	map.name = Z_strndup(buf, len);

	//краткое описание
	len = strn_cpp866_to_utf8(buf, BUFSIZE - 1, header.brief);
	map.brief = Z_strndup(buf, len);

	//чтение карты
	count = read(fd, map.map, MAP_SX * MAP_SY);
	if(count != MAP_SX * MAP_SY) RETURN_ERR(MAP_ERR_READ);

	bool player_spawn_exist = false;
	int i = 0;
	for(;;)
	{
		mapdata_mobj_type_t mapdata_mobj_type;
		map_data_mobj_t data;
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
void map_clear()
{
	if(!map.loaded)return;
	Z_FREE(map._file);
	Z_FREE(map.name);
	Z_FREE(map.brief);
	mobjs_erase();
	map.loaded = false;
}

/*
 * рисование объектов на карте
 */
void map_draw(camera_t * cam)
{

	int x, y;
	int x0,x1;
	int y0,y1;

	x0 = VEC_TRUNC((cam->pos.x-(cam->sx / 2))/8);
	if(x0 < 0) x0 = 0;
	y0 = VEC_TRUNC((cam->pos.y-(cam->sy / 2))/8);
	if(y0 < 0) y0 = 0;
	x1 = x0 + (cam->sx / 8) + 1;
	if(MAP_SX < x1) x1 = MAP_SX;
	y1 = y0 + (cam->sy / 8) + 1;
	if(MAP_SY < y1) y1 = MAP_SY;

	for(y = y0; y < y1; y++)
	{
		for(x = x0; x < x1; x++ )
		{
			char map_block = map.map[y][x] & 0x0F;
			item_img_t * img = NULL;
			switch(map_block)
			{
				case c_m_w_w0   : img = image_get(IMG_WALL_W0); break;
				case c_m_w_w1   : img = image_get(IMG_WALL_W1); break;
				case c_m_w_brick: img = image_get(IMG_WALL_BRICK); break;
				case c_m_water  : img = game.w_water[xrand(3)]; break;
			}
			if(img)
			{
				int pos_x = VEC_TRUNC(cam->x + (x  ) * 8 - (cam->pos.x - cam->sx / 2));
				int pos_y = VEC_TRUNC(cam->y - (y+1) * 8 + (cam->pos.y + cam->sy / 2));
				gr2D_setimage0(pos_x, pos_y, img);
			}
		}
	}

	mobjs_render(cam);

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
	if(mapList == NULL) mapList = mapEnt;
	else
	{
		p = mapList;
		while(p->next) p = p->next;
		mapEnt->prev = p;
		p->next = mapEnt;
	}
	game.custommap = mapList;
	game.gamemap = mapList;
}
/*
 * удаление списка карт
 */
void map_list_removeall()
{
	maplist_t * mapEnt;
	game.custommap = NULL;
	game.gamemap = NULL;
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
void map_load_list()
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
