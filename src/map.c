/*
 *        Ubivat Tank
 *        обработка карт
 * by Master San
 */

#include <video.h>
#include <game.h>
#include <img.h>
#include <_gr2D.h>
#include <fonts.h>

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

char * map_class_names[__MAP_NUM] =
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
		""
};

//список карт
maplist_t * mapList;

//карта
map_t map;

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
	map.spawns = NULL;
	map.items  = NULL;
	map.objs   = NULL;
}


/*
 * добавление точки спавна
 */
static void map_spawn_add(mobj_type_t mobj_type, map_data_spawn_t * data)
{
	spawn_t * spawn = Z_malloc(sizeof(*spawn));
	spawn->class  = mobj_type;
	spawn->orig.x = data->orig.x;
	spawn->orig.y = data->orig.y;
	spawn->scores = data->scores;
	spawn->health = data->health;
	spawn->armor  = data->armor;

	spawn->next   = map.spawns;
	map.spawns = spawn;
}
/*
 * удаление всех точек спавна
 */
static void map_spawn_removeall()
{
	spawn_t * spawn;
	while(map.spawns)
	{
		spawn         = map.spawns;
		map.spawns = map.spawns->next;
		Z_free(spawn);
	}
}

/*
 * добавление предмета на карту
 */
static int map_item_add(mobj_type_t mobj_type, map_data_item_t * data)
{
	static char *itemList[] = {
			"I_HEALTH",
			"I_ARMOR" ,
			"I_STAR"  ,
			"W_ROCKET",
			"W_MINE"
	};
	item_t * item = Z_malloc(sizeof(*item));
	item->class = mobj_type;
	item->orig.x = data->orig.x;
	item->orig.y = data->orig.y;
	item->amount = data->amount;
	item->exist = true;

	int i;
	switch(mobj_type)
	{
	case MAP_ITEM_HEALTH: i = 0; break;
	case MAP_ITEM_ARMOR : i = 1; break;
	case MAP_ITEM_STAR  : i = 2; break;
	case MAP_ITEM_ROCKET: i = 3; break;
	case MAP_ITEM_MINE  : i = 4; break;
	default: return -1;
	};
	item->img  = IMG_connect(itemList[i]);
	item->next = map.items;
	map.items  = item;
	return 0;
}
/*
 * удаление всех предметов
 */
static void map_item_removeall()
{
	item_t * item;
	while(map.items)
	{
		item         = map.items;
		map.items = map.items->next;
		Z_free(item);
	}
}

/*
 * добавление объекта на карту
 */
static void map_obj_add(mobj_type_t mobj_type, map_data_obj_t * data)
{
	obj_t * obj = Z_malloc(sizeof(*obj));

	obj->class  = mobj_type;
	obj->orig.x = data->orig.x;
	obj->orig.y = data->orig.y;
	strncpy(obj->message, data->message, MAB_OBJ_MESAGE_SIZE-1);

	switch(mobj_type)
	{
	case MAP_OBJ_EXIT: obj->img = IMG_connect("O_EXIT");break;
	case MAP_OBJ_MESS: obj->img = NULL                 ;break;
	default:;
	};
	obj->next = map.objs;
	map.objs  = obj;
}
/*
 * удаление всех объектов
 */
static void map_obj_removeall()
{
	obj_t * obj;
	while(map.objs)
	{
		obj         = map.objs;
		map.objs = map.objs->next;
		Z_free(obj);
	}
}
/*
 * поиск препятствия на карте
 */
void map_clip_find(
		pos_t * orig,
		float BOX,
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
	float count;
	float hbox;
	*Ul = false;*Ur = false;
	*Dl = false;*Dr = false;
	*Lu = false;*Ld = false;
	*Ru = false;*Rd = false;
	hbox = BOX/2;
	count = 0;
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
		if(
				((map.map[(int)trunc((orig->y+hbox      )/8)][(int)trunc((orig->x-hbox+count)/8)]& 0xF0) != 0) &&
				((map.map[(int)trunc((orig->y+hbox      )/8)][(int)trunc((orig->x-hbox+count)/8)]& mask) != 0)) *Ul = true;
		if(
				((map.map[(int)trunc((orig->y+hbox      )/8)][(int)trunc((orig->x+hbox-count)/8)]& 0xF0) != 0) &&
				((map.map[(int)trunc((orig->y+hbox      )/8)][(int)trunc((orig->x+hbox-count)/8)]& mask) != 0)) *Ur = true;
		if(
				((map.map[(int)trunc((orig->y-hbox      )/8)][(int)trunc((orig->x-hbox+count)/8)]& 0xF0) != 0) &&
				((map.map[(int)trunc((orig->y-hbox      )/8)][(int)trunc((orig->x-hbox+count)/8)]& mask) != 0)) *Dl = true;
		if(
				((map.map[(int)trunc((orig->y-hbox      )/8)][(int)trunc((orig->x+hbox-count)/8)]& 0xF0) != 0) &&
				((map.map[(int)trunc((orig->y-hbox      )/8)][(int)trunc((orig->x+hbox-count)/8)]& mask) != 0)) *Dr = true;
		if(
				((map.map[(int)trunc((orig->y+hbox-count)/8)][(int)trunc((orig->x-hbox      )/8)]& 0xF0) != 0) &&
				((map.map[(int)trunc((orig->y+hbox-count)/8)][(int)trunc((orig->x-hbox      )/8)]& mask) != 0)) *Lu = true;
		if(
				((map.map[(int)trunc((orig->y-hbox+count)/8)][(int)trunc((orig->x-hbox      )/8)]& 0xF0) != 0) &&
				((map.map[(int)trunc((orig->y-hbox+count)/8)][(int)trunc((orig->x-hbox      )/8)]& mask) != 0)) *Ld = true;
		if(
				((map.map[(int)trunc((orig->y+hbox-count)/8)][(int)trunc((orig->x+hbox      )/8)]& 0xF0) != 0) &&
				((map.map[(int)trunc((orig->y+hbox-count)/8)][(int)trunc((orig->x+hbox      )/8)]& mask) != 0)) *Ru = true;
		if(
				((map.map[(int)trunc((orig->y-hbox+count)/8)][(int)trunc((orig->x+hbox      )/8)]& 0xF0) != 0) &&
				((map.map[(int)trunc((orig->y-hbox+count)/8)][(int)trunc((orig->x+hbox      )/8)]& mask) != 0)) *Rd = true;
		count++;
	}
}
/*
 * вычисление расстояния до ближайшей стены
 */
void map_clip_find_near(pos_t * orig, coord_t box, int dir, char mask, coord_t DISTmax, coord_t * dist)
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
				wall = map.map[(int)trunc((orig->y+(*dist))/8)][(int)trunc((orig->x+c   )/8)];
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
				mapy = trunc((orig->y-(*dist))/8);
				mapx = trunc((orig->x+c   )/8);
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
				wall = map.map[(int)trunc((orig->y+c   )/8)][(int)trunc((orig->x-(*dist))/8)];
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
				wall = map.map[(int)trunc((orig->y+c   )/8)][(int)trunc((orig->x+(*dist))/8)];
				c++;
			}while(!( (+box<=c)||((wall & mask) != 0)));
		}while(!( (DISTmax*8<=(*dist)) || (wall & mask ) ));
		break;
	}
}
/*
 * вычисление расстояния до ближайшей стены и определение стены
 */
void map_clip_find_near_wall(pos_t * orig, int dir, float * dist, char * wall)
{
	*dist = 0;
	switch(dir)
	{
	case 0:
		do
		{
			*wall = map.map[(int)trunc((orig->y+(*dist))/8)][(int)trunc((orig->x     )/8)];
			(*dist)++;
		}while(!( (MAP_SY*8<=(*dist))||((*wall & 0xF0) != 0)));
		break;
	case 1:
		do
		{
			*wall = map.map[(int)trunc((orig->y-(*dist))/8)][(int)trunc((orig->x     )/8)];
			(*dist)++;
		}while(!( (MAP_SY*8<=(*dist))||((*wall & 0xF0) != 0)));
		break;
	case 2:
		do
		{
			*wall = map.map[(int)trunc((orig->y     )/8)][(int)trunc((orig->x-(*dist))/8)];
			(*dist)++;
		}while(!( (MAP_SX*8<=(*dist))||((*wall & 0xF0) != 0)));
		break;
	case 3:
		do
		{
			*wall = map.map[(int)trunc((orig->y     )/8)][(int)trunc((orig->x+(*dist))/8)];
			(*dist)++;
		}while(!( (MAP_SX*8<=(*dist))||((*wall & 0xF0) != 0)));
		break;
	}
}

static bool _eof;
bool map_eof()
{
	return _eof;
}

/*
 * чтение класса предмета
 */
mobj_type_t map_file_class_get(int fd)
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
	_eof = (count == 0);
	for(i = 0; i < __MAP_NUM; i++)
	{
		if(strcmp(class, map_class_names[i]) == 0) return i;
	}
	return MAP_UNKNOWN;
}
/*
 * создание спавн-поита
 */
static bool map_load_spawn(int fd, mobj_type_t mobj_type)
{
	map_data_spawn_t data;

	ssize_t count = read(fd, &data, sizeof(data));
	_eof = (count == 0);
	if(count != sizeof(data))return false;

	map_spawn_add(mobj_type, &data);
	return true;
}
/*
 * создание предмета
 */
static bool map_load_item(int fd, mobj_type_t mobj_type)
{
	map_data_item_t data;

	ssize_t count = read(fd, &data, sizeof(data));
	_eof = (count == 0);
	if(count != sizeof(data))return false;

	map_item_add(mobj_type, &data);
	return true;
}
/*
 * попытка создания объекта
 */
static bool map_load_obj(int fd, mobj_type_t mobj_type)
{
	map_data_obj_t data;

	ssize_t count = read(fd, &data, sizeof(data));
	_eof = (count == 0);
	if(count != sizeof(data))return false;

	map_obj_add(mobj_type, &data);
	return true;
};

/********открытие и чтение карты********/
//map_load=0 -нет ошибок
//map_load=1 -файл карты не найден
//map_load=2 -ошибка формата
//map_load=3 -ошибка чтения(файл поврежден)
//map_load=4 -не найден спавн-поинт для GAME игры
//map_load=5 -не найден спавн-поинт для CASE игры
int map_load(const char * mapname)
{
#define RETURN_ERR(err) \
		do{ \
			close(fd); \
			map_error = (err); \
			return -1; \
		}while(0);

	int fd;
	int ret;
	char *path;

	map_clear();
	map._file = Z_strdup(mapname);
	path = Z_malloc(
			strlen(BASEDIR MAPSDIR)+
			strlen(map._file)+
			strlen(MAP_FILE_EXT)+
			1
	);
	strcpy(path, BASEDIR MAPSDIR);
	strcat(path, map._file);
	strcat(path, MAP_FILE_EXT);
	fd = open(path, O_RDONLY);
	Z_free(path);
	if(fd < 0) RETURN_ERR(MAP_ERR_NOFILE);

	ssize_t count;
	char MAPheader[3];
	count = read(fd, MAPheader, 3);
	if(count != 3) RETURN_ERR(MAP_ERR_READ);

	ret = memcmp(MAPheader, MAP_DATA_HEADER, 3);
	if(ret) RETURN_ERR(MAP_ERR_FORMAT);

	//название карты
	count = read(fd, map.name, MAP_NAME_SIZE);
	map.name[MAP_NAME_SIZE-1] = 0;
	if(count != MAP_NAME_SIZE) RETURN_ERR(MAP_ERR_READ);

	//краткое описание
	count = read(fd,map.brief, MAP_BRIEF_SIZE);
	map.brief[MAP_BRIEF_SIZE-1] = 0;
	if(count != MAP_BRIEF_SIZE) RETURN_ERR(MAP_ERR_READ);

	//чтение карты
	count = read(fd, map.map, MAP_SX * MAP_SY);
	if(count != MAP_SX * MAP_SY) RETURN_ERR(MAP_ERR_READ);

	_eof = false;
	bool player_spawn_exist = false;
	for(;;)
	{
		mobj_type_t mobj_type = map_file_class_get(fd);
		if(map_eof(fd)) break;
		switch(mobj_type)
		{
		case MAP_SPAWN_PLAYER: player_spawn_exist = true;
		case MAP_SPAWN_ENEMY :
		case MAP_SPAWN_BOSS  :
			ret = map_load_spawn(fd, mobj_type);
			break;
		case MAP_ITEM_HEALTH :
		case MAP_ITEM_ARMOR  :
		case MAP_ITEM_STAR   :
		case MAP_ITEM_ROCKET :
		case MAP_ITEM_MINE   :
			ret = map_load_item(fd, mobj_type);
			break;
		case MAP_OBJ_EXIT    :
		case MAP_OBJ_MESS    :
			ret = map_load_obj(fd, mobj_type);
			break;
		default:
			game_console_send("map load error: no class %d", mobj_type);
			map_clear();
			RETURN_ERR(MAP_ERR_READ);
		}
	}
	close(fd);
	if(!player_spawn_exist)
	{
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
	Z_free(map._file);
	map_spawn_removeall();
	map_item_removeall();
	map_obj_removeall();
}
/*
 * рисование объектов на карте
 */
void map_draw(camera_t * cam)
{
	item_t * item;
	obj_t * obj;
	int x, y;
	int x0,x1;
	int y0,y1;

	item = map.items;
	while(item)
	{
		if(
				(item->exist) &&
				(cam->pos.x-cam->sx/2 <= item->orig.x+(c_i_MDL_box/2))&&(item->orig.x - (c_i_MDL_box/2)<=cam->pos.x+cam->sx/2) &&
				(cam->pos.y-cam->sy/2 <= item->orig.y+(c_i_MDL_box/2))&&(item->orig.y - (c_i_MDL_box/2)<=cam->pos.y+cam->sy/2)
			)
		{
			gr2D_setimage0(
					round(cam->x+item->orig.x-(cam->pos.x-cam->sx/2))+c_i_MDL_pos,
					round(cam->y-item->orig.y+(cam->pos.y+cam->sy/2))+c_i_MDL_pos,
					item->img
			);
		}
		item = item->next;
	}

	obj = map.objs;
	while(obj){
		if(
				(obj->img) &&
				(cam->pos.x-cam->sx/2<=obj->orig.x+(c_o_MDL_box/2))&&(obj->orig.x-(c_o_MDL_box/2)<=cam->pos.x+cam->sx/2) &&
				(cam->pos.y-cam->sy/2<=obj->orig.y+(c_o_MDL_box/2))&&(obj->orig.y-(c_o_MDL_box/2)<=cam->pos.y+cam->sy/2)
		)
		{
			gr2D_setimage0(
					round(cam->x+obj->orig.x-(cam->pos.x-cam->sx/2))+c_o_MDL_pos,
					round(cam->y-obj->orig.y+(cam->pos.y+cam->sy/2))+c_o_MDL_pos,
					obj->img
			);
		};
		obj = obj->next;
	};

	x0 = trunc((cam->pos.x-(cam->sx / 2))/8);
	if(x0 < 0) x0 = 0;
	y0 = trunc((cam->pos.y-(cam->sy / 2))/8);
	if(y0 < 0) y0 = 0;
	x1 = x0+(cam->sx / 8)+1;
	if(MAP_SX<=x1) x1 = MAP_SX-1;
	y1 = y0+(cam->sy / 8)+1;
	if(MAP_SY<=y1) y1 = MAP_SY-1;

	for(y = y0; y <= y1; y++)
	{
		for(x = x0; x <= x1; x++ )
		{
			char map_block = map.map[y][x] & 0x0F;
			item_img_t * img = NULL;
			switch(map_block)
			{
			case c_m_w_w0   : img = game.w_w0; break;
			case c_m_w_w1   : img = game.w_w1; break;
			case c_m_w_brick: img = game.w_brick; break;
			case c_m_water  : img = game.w_water[xrand(3)]; break;
			}
			if(img)
			{
				int pos_x = round(cam->x + (x  ) * 8 - (cam->pos.x - cam->sx / 2));
				int pos_y = round(cam->y - (y+1) * 8 + (cam->pos.y + cam->sy / 2));
				gr2D_setimage0(pos_x, pos_y, img);
			}
		}
	}
};
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
	char * str = Z_calloc(1, str_size);
	if(!str)
	{
		game_halt("Maps list is empty");
	}
	char * map = NULL;

	if ((fconf = fopen(BASEDIR FILENAME_MAPSLIST, "r")) == NULL)
	{
		game_halt("Could not load %s", FILENAME_MAPSLIST);
	}

	mapList = NULL;
	bool isString = false;
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
