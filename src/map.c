/*
 *        Ubivat Tank
 *        обработка карт
 * by Master San
 */

#include <defs.h>
#include <game.h>
#include <img.h>
#include <_gr2D.h>
#include <_gr2Don.h>
#include <x10_time.h>
#include <x10_str.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>

//список карт
maplist_t * mapList;

//карта
map_t map;

void map_close();

/*
 * добавление точки спавна
 */
static void map_spawn_add(TDATAspawn * DATA)
{
	spawn_t * spawn;
	spawn = Z_malloc(sizeof(*spawn));
	memcpy(&spawn->data, DATA, sizeof(*DATA));
	spawn->next   = map.HEADspawn;
	map.HEADspawn = spawn;
}
/*
 * удаление всех точек спавна
 */
static void map_spawn_removeall()
{
	spawn_t * spawn;
	while(map.HEADspawn)
	{
		spawn         = map.HEADspawn;
		map.HEADspawn = map.HEADspawn->next;
		Z_free(spawn);
	}
}
/*
 * проверка спавн-поинтов
 *
 * @return = 0 -спавн-поинт найден
 * @return = 1 -не найден PLAYER спавн-поинт
 */
int map_spawn_checkspawnpoints()
{
	spawn_t * spawn = map.HEADspawn;
	while(spawn)
	{
		//присутствует PLAYER спавн
		if(spawn->data.class & c_s_PLAYER)
			return 0;
		spawn = spawn->next;
	}
	return 1;
}
/*
 * добавление предмета на карту
 */
static int map_item_add(TDATAitem * DATA)
{
	static char *itemList[] = {
			"I_HEALTH",
			"I_ARMOR" ,
			"I_STAR"  ,
			"W_ROCKET",
			"W_MINE"
	};
	item_t * item = Z_malloc(sizeof(*item));
	memcpy(&item->data, DATA, sizeof(*DATA));
	//флаг присутствия
	item->present = true;
	int i;
	switch(DATA->class)
	{
	case c_i_health: i = 0; break;
	case c_i_armor : i = 1; break;
	case c_i_star  : i = 2; break;
	case c_i_rocket: i = 3; break;
	case c_i_mine  : i = 4; break;
	default: return 0;
	};
	item->img = IMG_connect(game.HEADimg, itemList[i]);
	item->next   = map.HEADitem;
	map.HEADitem = item;
	return -1;
}
/*
 * удаление всех предметов
 */
static void map_item_removeall()
{
	item_t * item;
	while(map.HEADitem)
	{
		item         = map.HEADitem;
		map.HEADitem = map.HEADitem->next;
		Z_free(item);
	}
}

/*
 * добавление объекта на карту
 */
static void map_obj_add(obj_data_t * DATA)
{
	obj_t * obj;
	obj = Z_malloc(sizeof(*obj));
	memcpy(&obj->data, DATA, sizeof(*obj));
	switch(DATA->class)
	{
	case c_o_exit: obj->img = IMG_connect(game.HEADimg, "O_EXIT");break;
	case c_o_mess: obj->img = NULL                               ;break;
	};
	obj->next   = map.HEADobj;
	map.HEADobj = obj;
}
/*
 * удаление всех объектов
 */
static void map_obj_removeall()
{
	obj_t * obj;
	while(map.HEADobj)
	{
		obj         = map.HEADobj;
		map.HEADobj = map.HEADobj->next;
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
void map_clip_find_near(pos_t * orig, float box, int dir, char mask, float DISTmax, float * dist)
{
	char wall;
	float c;
	box = box/2;
	*dist = box/2;
	switch(dir)
	{
	case c_DIR_up:
		if(c_MAP_sy*8<DISTmax) DISTmax = c_MAP_sy * 8;
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
	case c_DIR_dn:
		if(c_MAP_sy*8<DISTmax) DISTmax = c_MAP_sy*8;
		do
		{
			(*dist)++;
			c = -box+1;
			do
			{
				wall = map.map[(int)trunc((orig->y-(*dist))/8)][(int)trunc((orig->x+c   )/8)];
				c++;
			}while(!((+box<=c)||((wall & mask) != 0)));
		}while(!((DISTmax<=(*dist))||((wall & mask) != 0)));
		break;
	case c_DIR_lf:
		if(c_MAP_sx*8<DISTmax) DISTmax = c_MAP_sx*8;
		do
		{
			(*dist)++;
			c = -box+1;
			do
			{
				wall = map.map[(int)trunc((orig->y+c   )/8)][(int)trunc((orig->x-(*dist))/8)];
				c++;
			}while(!( (+box<=c)||((wall && mask) != 0)));
		}while(!( (DISTmax<=(*dist))||((wall && mask) != 0)));
		break;
	case c_DIR_rt:
		if(c_MAP_sx*8<DISTmax) DISTmax = c_MAP_sx*8;
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
		}while(!( (c_MAP_sy*8<=(*dist))||((*wall & 0xF0) != 0)));
		break;
	case 1:
		do
		{
			*wall = map.map[(int)trunc((orig->y-(*dist))/8)][(int)trunc((orig->x     )/8)];
			(*dist)++;
		}while(!( (c_MAP_sy*8<=(*dist))||((*wall & 0xF0) != 0)));
		break;
	case 2:
		do
		{
			*wall = map.map[(int)trunc((orig->y     )/8)][(int)trunc((orig->x-(*dist))/8)];
			(*dist)++;
		}while(!( (c_MAP_sx*8<=(*dist))||((*wall & 0xF0) != 0)));
		break;
	case 3:
		do
		{
			*wall = map.map[(int)trunc((orig->y     )/8)][(int)trunc((orig->x+(*dist))/8)];
			(*dist)++;
		}while(!( (c_MAP_sx*8<=(*dist))||((*wall & 0xF0) != 0)));
		break;
	}
}
/*
 * чтение класса предмета
 */
char * map_file_class_get(int fd, char * class)
{
	char ch;
	int i = 0;
	int c;
	do
	{
		c = read(fd, &ch, 1);
		class[i] = ch;
		i++;
	}while(c != 0 && ch!=0 );
	return class;
}
/*
 * попытка создания спавн-поита
 */
static bool map_try_load_spawn(int fd, const char *class_string)
{
	TDATAspawn DATA;
	if(strcmp(class_string,c_MAP_s_player)==0) DATA.class = c_s_PLAYER;
	else
		if(strcmp(class_string,c_MAP_s_enemy )==0) DATA.class = c_s_ENEMY;
		else
			if(strcmp(class_string,c_MAP_s_boss  )==0) DATA.class = c_s_BOSS;
			else {
				return false;
			};
	read(fd, &DATA.orig.x, 2);
	read(fd, &DATA.orig.y, 2);
	read(fd, &DATA.scores, 4);
	read(fd, &DATA.health, 2);
	read(fd, &DATA.armor , 2);
	map_spawn_add(&DATA);
	return true;
}
/********попытка создания предмета********/
static bool map_try_load_item(int fd, const char * class_string)
{
	TDATAitem DATA;
	if(strcmp(class_string,c_MAP_i_health)==0) DATA.class = c_i_health;
	else
		if(strcmp(class_string,c_MAP_i_armor )==0) DATA.class = c_i_armor;
		else
			if(strcmp(class_string,c_MAP_i_star  )==0) DATA.class = c_i_star;
			else
				if(strcmp(class_string,c_MAP_i_rocket )==0) DATA.class = c_i_rocket;
				else
					if(strcmp(class_string,c_MAP_i_mine   )==0) DATA.class = c_i_mine;
					else {
						return false;
					};
	read(fd, &DATA.orig.x, 2);
	read(fd, &DATA.orig.y, 2);
	read(fd, &DATA.amount, 2);
	map_item_add(&DATA);
	return true;
}
/*
 * попытка создания объекта
 */
static bool map_try_load_obj(int fd, const char * class_string)
{
	obj_data_t DATA;
	if(strcmp(class_string,c_MAP_o_exit)==0) DATA.class = c_o_exit;
	else
		if(strcmp(class_string,c_MAP_o_mess)==0) DATA.class = c_o_mess;
		else {
			return false;
		};
	read(fd, &DATA.orig.x,2);
	read(fd, &DATA.orig.y,2);
	read(fd, &DATA.message,65);
	map_obj_add(&DATA);
	return true;
};
/*
 * чтение заголовка карты
 *
 * @return = 0 - успешно
 * @return = 1 - ошибка чтения
 * @return = 2 - неверный заголовок
 */
static int map_load_header(int fd)
{
	int c;
	char MAPheader[3];
	c = read(fd, MAPheader, 3);
	if(c != 3) return 1;
	if(
			(MAPheader[0] != c_MAPheader[0])||
			(MAPheader[1] != c_MAPheader[1])||
			(MAPheader[2] != c_MAPheader[2])
	)
		return 2;
	c = read(fd,map.name , 17);map.name [ 16] = 0;                   //название карты
	if(c != 17) return 1;
	c = read(fd,map.brief,129);map.brief[128] = 0;                   //краткое описание
	if(c != 129) return 1;
	return 0;
}
/********открытие и чтение карты********/
//map_load=0 -нет ошибок
//map_load=1 -файл карты не найден
//map_load=2 -ошибка формата
//map_load=3 -ошибка чтения(файл поврежден)
//map_load=4 -не найден спавн-поинт для GAME игры
//map_load=5 -не найден спавн-поинт для CASE игры
int map_load(const char * mapname)
{
	int fd;
	TstrZ16 class_string;
	int error;
	char *path;

	map_close();
	strcpy(map._file, mapname);
	path = Z_malloc(
			strlen(BASEDIR)+
			strlen(MAPSDIR)+
			strlen(map._file)+
			strlen(c_MAPext)+
			1
	);
	strcpy(path, BASEDIR);
	strcat(path, MAPSDIR);
	strcat(path, map._file);
	strcat(path, c_MAPext);
	fd = open(path, O_RDONLY);
	Z_free(path);
	if(fd < 0) return 1;
	//чтение заголовка
	error = map_load_header(fd);
	if(error == 1) { close(fd);return 3;}
	if(error == 2) { close(fd);return 2;}
	//чтение карты
	read(fd, map.map, c_MAP_sx*c_MAP_sy);
	if(errno) { close(fd);return 3;}
	while(!(eof(fd)))
	{
		map_file_class_get(fd,class_string);
		if(map_try_load_spawn(fd,class_string) == false)
		{
			if(map_try_load_item(fd,class_string) == false)
			{
				if(map_try_load_obj(fd,class_string) == false)
				{
						gr2D.color.current = 15;
						gr2Don_settextZ(0,0,"ER: ");
						gr2Don_settextZ(32,0,class_string);
						gr2D_BUFcurrent2screen();
						getchar();
						gr2D.color.current = 0;
						gr2D_BUFcurrentfill();
				}
			}
		}
	}
	close(fd);
	error = map_spawn_checkspawnpoints();
	if(error)
	{
		map_close();
		return error+4;
	}
	return 0;
};
/*
 * закрытие карты
 */
void map_close()
{
	map_spawn_removeall();
	map_item_removeall();
	map_obj_removeall();
	game._win_ = false;
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
	char map_block;

	gr2D.WIN.x0 = cam->x;gr2D.WIN.x1 = cam->x+cam->sx-1;
	gr2D.WIN.y0 = cam->y;gr2D.WIN.y1 = cam->y+cam->sy-1;
	item = map.HEADitem;
	while(item)
	{
		if(
				(item->present) &&
				(cam->orig.x-cam->sx/2<=item->data.orig.x+(c_i_MDL_box/2))&&(item->data.orig.x-(c_i_MDL_box/2)<=cam->orig.x+cam->sx/2) &&
				(cam->orig.y-cam->sy/2<=item->data.orig.y+(c_i_MDL_box/2))&&(item->data.orig.y-(c_i_MDL_box/2)<=cam->orig.y+cam->sy/2)
			)
		{
			gr2D_setimage0(
					round(cam->x+item->data.orig.x-(cam->orig.x-cam->sx/2))+c_i_MDL_pos,
					round(cam->y-item->data.orig.y+(cam->orig.y+cam->sy/2))+c_i_MDL_pos,
					item->img->IMG.sx,
					item->img->IMG.sy,
					item->img->IMG.pic
			);
		}
		item = item->next;
	}

	obj = map.HEADobj;
	while(obj){
		if(
				(obj->img) &&
				(cam->orig.x-cam->sx/2<=obj->data.orig.x+(c_o_MDL_box/2))&&(obj->data.orig.x-(c_o_MDL_box/2)<=cam->orig.x+cam->sx/2) &&
				(cam->orig.y-cam->sy/2<=obj->data.orig.y+(c_o_MDL_box/2))&&(obj->data.orig.y-(c_o_MDL_box/2)<=cam->orig.y+cam->sy/2)
		)
		{
			gr2D_setimage0(
					round(cam->x+obj->data.orig.x-(cam->orig.x-cam->sx/2))+c_o_MDL_pos,
					round(cam->y-obj->data.orig.y+(cam->orig.y+cam->sy/2))+c_o_MDL_pos,
					obj->img->IMG.sx,
					obj->img->IMG.sy,
					obj->img->IMG.pic
			);
		};
		obj = obj->next;
	};

	x0 = trunc((cam->orig.x-(cam->sx / 2))/8);
	if(x0 < 0) x0 = 0;
	y0 = trunc((cam->orig.y-(cam->sy / 2))/8);
	if(y0 < 0) y0 = 0;
	x1 = x0+(cam->sx / 8)+1;
	if(c_MAP_sx<=x1) x1 = c_MAP_sx-1;
	y1 = y0+(cam->sy / 8)+1;
	if(c_MAP_sy<=y1) y1 = c_MAP_sy-1;

	for(y = y0; y <= y1; y++)
	{
		for(x = x0; x <= x1; x++ )
		{
			map_block = map.map[y][x] & 0x0F;
			if(map_block==c_m_w_w0   ) {
				gr2D_setimage0(
						round(cam->x+(x)*8-(cam->orig.x-cam->sx/2)),round(cam->y-(y+1)*8+(cam->orig.y+cam->sy/2)),
						game.w_w0->IMG.sx,game.w_w0->IMG.sy,game.w_w0->IMG.pic
				);
			}
			else
				if(map_block==c_m_w_w1   )
				{
					gr2D_setimage0(
							round(cam->x+(x)*8-(cam->orig.x-cam->sx/2)),round(cam->y-(y+1)*8+(cam->orig.y+cam->sy/2)),
							game.w_w1->IMG.sx,game.w_w1->IMG.sy,game.w_w1->IMG.pic
					);
				}
				else
					if(map_block==c_m_w_brick)
					{
						gr2D_setimage0(
								round(cam->x+(x)*8-(cam->orig.x-cam->sx/2)),round(cam->y-(y+1)*8+(cam->orig.y+cam->sy/2)),
								game.w_brick->IMG.sx,game.w_brick->IMG.sy,game.w_brick->IMG.pic
						);
					}
					else
						if(map_block==c_m_water)
						{
							gr2D_setimage0(
									round(cam->x+(x)*8-(cam->orig.x-cam->sx/2)),round(cam->y-(y+1)*8+(cam->orig.y+cam->sy/2)),
									game.w_water[0]->IMG.sx,game.w_water[0]->IMG.sy,game.w_water[xrand(3)]->IMG.pic
							);
						};
		};
	};
	gr2D.WIN.x0 = 0;
	gr2D.WIN.x1 = gr2D_SCR_sx-1;
	gr2D.WIN.y0 = 0;
	gr2D.WIN.y1 = gr2D_SCR_sy-1;
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
	game.casemap = mapList;
	game.gamemap = mapList;
}
/*
 * удаление списка карт
 */
void map_list_removeall()
{
	maplist_t * mapEnt;
	game.casemap = NULL;
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
int map_load_list()
{
	FILE *fconf;

	size_t str_size = 256;
	char * str = Z_calloc(1, str_size);
	if(!str)
	{
		return -1;
	}
	char * map = NULL;

	if ((fconf = fopen(BASEDIR FILENAME_MAPSLIST, "r")) == NULL)
	{
		return -1;
	}

	mapList = NULL;
	bool isString = false;
	while(fconf)
	{

		size_t str_len = strlen(str);
		if(str_size <= str_len + 1)
		{
			char * tmp = realloc(str, str_size + 256);
			if (tmp)
			{
				str = tmp;
				str_size += 256;
			}
			else
			{
				//netsnmp_config_error("Failed to allocate memory\n");
				Z_free(str);
				fclose(fconf);
				return -1;
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
					return -1;
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
						return -1;
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
		return -1;
	}
	return 0;
}
