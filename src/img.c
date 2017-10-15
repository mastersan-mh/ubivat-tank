/*
 *        Ubivat Tank
 *        работа с изображениями
 * by Master San
 */

#include "game.h"
#include "img.h"
#include "Z_mem.h"
#include "video.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_image.h>

#define IMG_SFACTOR GL_SRC_ALPHA
#define IMG_DFACTOR GL_ONE_MINUS_SRC_ALPHA

static char * image_files[IMG__NUM] = {
	"/menu/mhlogo.bii"  , /* IMG_MENU_LOGO */
	"/menu/conback.bii" , /* IMG_MENU_CONBACK */
	"/menu/interlv.bii" , /* IMG_MENU_I_INTERLV */
	"/menu/game.bii"    , /* IMG_MENU_GAME */
	"/menu/new_p1.bii"  , /* IMG_MENU_G_NEW_P1 */
	"/menu/new_p2.bii"  , /* IMG_MENU_G_NEW_P2 */
	"/menu/load.bii"    , /* IMG_MENU_G_LOAD */
	"/menu/save.bii"    , /* IMG_MENU_G_SAVE */
	"/menu/case.bii"    , /* IMG_MENU_CASE */
	"/menu/server-connect.png", /* IMG_MENU_CASE_SERVERCONNECT */
	"/menu/options.bii" , /* IMG_MENU_OPTIONS */
	"/menu/about.bii"   , /* IMG_MENU_ABOUT */
	"/menu/abort.bii"   , /* IMG_MENU_ABORT */
	"/menu/quit.bii"    , /* IMG_MENU_QUIT */
	"/menu/cur_0.bii"   , /* IMG_MENU_CUR_0 */
	"/menu/cur_1.bii"   , /* IMG_MENU_CUR_1 */
	"/menu/arrowl.bii"  , /* IMG_MENU_ARROWL */
	"/menu/arrowr.bii"  , /* IMG_MENU_ARROWR */
	"/menu/linel.bii"   , /* IMG_MENU_LINEL */
	"/menu/linem.bii"   , /* IMG_MENU_LINEM */
	"/menu/liner.bii"   , /* IMG_MENU_LINER */


	"/pics/tank0.bii"   , /* TANK0 */
	"/pics/tank1.bii"   , /* TANK1 */
	"/pics/tank2.bii"   , /* TANK2 */
	"/pics/tank3.bii"   , /* TANK3 */
	"/pics/tank4.bii"   , /* TANK4 */

	"/pics/f_rus.bii"   , /* F_RUS */
	"/pics/f_usa.bii"   , /* F_USA */
	"/pics/f_white.bii" , /* F_WHITE */

	"/pics/world_w0.bii"      , /* W_W0 */
	"/pics/world_w1.bii"      , /* W_W1 */
	"/pics/world_brick.bii"   , /* W_BRICK */
	"/pics/world_water0.bii"  , /* WATER0 */
	"/pics/world_water1.bii"  , /* WATER1 */
	"/pics/world_water2.bii"  , /* WATER2 */
	"/pics/i_health.bii"      , /* I_HEALTH */
	"/pics/i_armor.bii"       , /* I_ARMOR */
	"/pics/i_star.bii"        , /* I_STAR */
	"/pics/o_exit.bii"        , /* O_EXIT */
	"/pics/ammo_artillery.bii", /* W_BULL */
	"/pics/ammo_missile.bii"  , /* W_ROCKET */
	"/pics/ammo_mine.bii"     , /* W_MINE */
	"/pics/bull_bull.bii"     , /* B_BULL */
	"/pics/bull_missile.bii"  , /* B_ROCKET */
	"/pics/bull_mine.bii"     , /* B_MINE */
	"/pics/explode_small.bii" , /* E_SMALL */
	"/pics/explode_big.bii"   , /* E_BIG */
	"/pics/hud_icon_tank0.bii", /* IMG_HUD_ICON_TANK0*/
	"/pics/hud_icon_tank1.bii", /* IMG_HUD_ICON_TANK1*/
	"/pics/hud_icon_tank2.bii", /* IMG_HUD_ICON_TANK2*/
	"/pics/hud_icon_tank3.bii", /* IMG_HUD_ICON_TANK3*/
	"/pics/hud_icon_tank4.bii", /* IMG_HUD_ICON_TANK4*/
};


static item_img_t image_table[IMG__NUM];

typedef struct
{
	char R,G,B;
} T2DRGBcolor;

typedef T2DRGBcolor T2Dpal[256];

int BII_errno;

//палитра
static T2Dpal _palette;
#define TRANSPARENT_COLOR_INDEX (255)

int img_palette_read(const char * filename)
{
#define RETURN(x) \
		do{ \
			ret = (x); \
			goto end; \
		}while(0);

	int ret = 0;
	int fd;
	fd = open(filename, O_RDONLY);
	if(fd < 0)
		RETURN(1);
	if(read(fd, _palette, 768) != 768)
		RETURN(2);
	end:
	switch(ret)
	{
		case 0:
		case 2:
			close(fd);
		case 1:;
	}
	return ret ? -1 : 0;
}

static char *BII_err_list[] =
{
		"No error",
		"Fatal error",
		"File error",
		"Read error",
		"Format error",
		"Unknown error"
};

char * IMG_errorGet()
{
	if(BII_errno < 0 || BII_errno >= BII_UNKNOWN) BII_errno = BII_UNKNOWN;
	return BII_err_list[BII_errno];
}

/*
 * чтение рисунка
 * @return = 0 успешно
 * @return = 1 фатальная ошибка
 * @return = 2 файл не найден
 * @return = 3 ошибка формата
 * @return = 4 ошибка чтения
 */
static BIIpic_t * __BII_load(const char * fname)
{
#define BII_HEADER     "BII"
#define BII_BUF_SIZE (3+2+2)
#define BII_OFFSET_SX 3
#define BII_OFFSET_SY (3+2)
	int __errno_;
#define PUSH_ERRNO() __errno_ = errno
#define POP_ERRNO() errno = __errno_

	BII_errno = 0;
	int ret;
	ssize_t rret;
	int fd;
	BIIpic_t * img;
	char buf[BII_BUF_SIZE];

	fd = open(fname, O_RDONLY);
	if(fd < 0)
	{
		BII_errno = BII_FILEERR;
		return NULL;
	}
	rret = read(fd, buf, BII_BUF_SIZE);
	if(rret != BII_BUF_SIZE)
	{
		BII_errno = BII_READERR;
		PUSH_ERRNO();
		close(fd);
		POP_ERRNO();
		return NULL;
	}
	//проверяем заголовок
	ret = memcmp(buf, BII_HEADER, 3);
	if(ret)
	{
		BII_errno = BII_FORMATERR;
		PUSH_ERRNO();
		close(fd);
		POP_ERRNO();
		return NULL;
	}
	img = Z_malloc(sizeof(*img));
	if(!img)
	{
		BII_errno = BII_FATAL;
		PUSH_ERRNO();
		close(fd);
		POP_ERRNO();
		return NULL;
	}

	img->sx = (int) (*((uint16_t *)( buf + BII_OFFSET_SX ))) + 1;
	img->sy = (int) (*((uint16_t *)( buf + BII_OFFSET_SY ))) + 1;

	//выделим память
	size_t size = img->sx*img->sy;
	img->pic = Z_malloc(size);
	if(!img->pic)
	{
		BII_errno = BII_FATAL;
		PUSH_ERRNO();
		Z_free(img);
		close(fd);
		POP_ERRNO();
		return NULL;
	}
	//читаем рисунок
	rret = read(fd, img->pic, size);
	if(rret != size)
	{
		BII_errno = BII_READERR;
		PUSH_ERRNO();
		Z_free(img->pic);
		Z_free(img);
		close(fd);
		POP_ERRNO();
		return NULL;
	}
	close(fd);
	return img;
};

static void __BII_free(BIIpic_t * pic)
{
	Z_free(pic->pic);
	Z_free(pic);
}

static GLsizei __make_gl_size(int size)
{
	return size;
/*
	GLsizei size2;
	if(size <= 64)return 64;
	//if(size <= 128)return 128;
	//if(size <= 256)return 256;
	int remainder = size % 2;
	if(remainder == 0)return size;
	size2 = 64;
	do
	{
		size2 <<=1;
	}while(size2 < size);
	return size2;
	*/
}

static void * __create_image_from_bitmap8(
	void * palette,
	uint8_t intencity,
	uint8_t * bitmap,
	int sx,
	int sy,
	int bitmap_sx,
	int bitmap_sy
)
{
	size_t size = bitmap_sx * bitmap_sy;
	struct
	{
		uint8_t R;
		uint8_t G;
		uint8_t B;
	} * pal = palette;
	int i = 0;
	int iindex = 0;
#define COLOR_COMPONENT_AMOUNT 4
	uint8_t * data = Z_malloc(size*COLOR_COMPONENT_AMOUNT);
	for(int y = 0; y < sy; y++)
	{
		for(int x = 0;x < sx;x++)
		{
			int pindex = bitmap[i++];
			if(pindex == TRANSPARENT_COLOR_INDEX)
			{
				data[iindex++] = 255;
				data[iindex++] = 255;
				data[iindex++] = 255;
				data[iindex++] = COLOR_ALPHA_TRANSPARENT;
			}
			else
			{
				data[iindex++] = pal[pindex].R * intencity;
				data[iindex++] = pal[pindex].G * intencity;
				data[iindex++] = pal[pindex].B * intencity;
				data[iindex++] = COLOR_ALPHA_OPAQUE;
			}
		}
		iindex += (bitmap_sx - sx)*COLOR_COMPONENT_AMOUNT;
	}
	return data;

}


static int __create_from_bitmap(
	void * bitmap,
	int sx,
	int sy,
	image_index_t i
)
{

	item_img_t * p = &image_table[i];

	p->sfactor = IMG_SFACTOR;
	p->dfactor = IMG_DFACTOR;
	p->img_sx = sx;
	p->img_sy = sy;
	p->texture_sx = __make_gl_size(sx);
	p->texture_sy = __make_gl_size(sy);
	p->data = __create_image_from_bitmap8(
		_palette,
		4,
		bitmap,
		sx,
		sy,
		p->texture_sx,
		p->texture_sy
		);

	GL_CHECK(glGenTextures(1, &p->texture));
	if(!p->texture) return -1;

	glBindTexture(GL_TEXTURE_2D, p->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(
		GL_TEXTURE_2D,    //GLenum  target
		0,                //GLint  level
		GL_RGBA, //4,     //GLint  internalFormat
		p->texture_sx,    //GLsizei  width
		p->texture_sy,    //GLsizei  height
		0,                //GLint  border
		GL_RGBA,          //GLenum  format
		GL_UNSIGNED_BYTE, //GLenum  type
		p->data           //const GLvoid * data
	);

	p->data = NULL;

	return 0;
}


/**
 * загрузка изображений
 * @param[in] указ.на_голову_списка;
 * @param[in] путь;
 * @param[in] ID_изображения
 * @return = 0 нет ошибок
 * @return = 1 файл не найден
 * @return = 2 ошибка формата
 * @return = 3 изображение уже загружено
 * @return = 4 изображение уже загружено
 */
static int __create_image_from_bii(const char * path, image_index_t i)
{
	//не нашли, прочитаем новую
	BIIpic_t * bii_image = __BII_load(path);
	if(!bii_image) return -1;
	int ret = __create_from_bitmap(
		bii_image->pic,
		bii_image->sx,
		bii_image->sy,
		i
	);
	__BII_free(bii_image);
	return ret;
}


static int __create_image_from_jpeg_png(const char * path, image_index_t i)
{

	//Uint32 dest_pixel_format = SDL_PIXELFORMAT_RGBA8888;
	Uint32 dest_pixel_format = SDL_PIXELFORMAT_ABGR8888;
	SDL_Surface * img_tmp = IMG_Load(path);
	if(!img_tmp)
	{
		return -1;
	}
	SDL_Surface * img = NULL;
	if(img_tmp->format->format == dest_pixel_format)
	{
		img = img_tmp;
		img_tmp = NULL;
	}
	else
	{
		img = SDL_ConvertSurfaceFormat(img_tmp, dest_pixel_format, 0);
	}
	SDL_FreeSurface(img_tmp);

	item_img_t * p = &image_table[i];

	int sx = img->w;
	int sy = img->h;

	//SDL_PixelFormat format = img_tmp->format;

	p->sfactor = IMG_SFACTOR;
	p->dfactor = IMG_DFACTOR;
	p->img_sx = sx;
	p->img_sy = sy;
	p->texture_sx = __make_gl_size(sx);
	p->texture_sy = __make_gl_size(sy);
	p->data = (uint8_t *) img->pixels;

	GL_CHECK(glGenTextures(1, &p->texture));
	if(!p->texture) return -1;

	glBindTexture(GL_TEXTURE_2D, p->texture);

	//p->sfactor = GL_ONE;
	//p->dfactor = GL_ONE_MINUS_SRC_ALPHA;
	//p->sfactor = GL_ONE;
	//p->dfactor = GL_SRC_ALPHA;
	//p->sfactor = GL_ZERO;
	//p->dfactor = GL_ONE_MINUS_SRC_ALPHA;
	p->sfactor = GL_SRC_ALPHA;
	p->dfactor = GL_ONE_MINUS_SRC_ALPHA;


/*
	p->sfactor = GL_ZERO;
	p->dfactor = GL_SRC_ALPHA;
*/

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(
		GL_TEXTURE_2D,    //GLenum  target
		0,                //GLint  level
		GL_RGBA, //4,     //GLint  internalFormat
		p->texture_sx,    //GLsizei  width
		p->texture_sy,    //GLsizei  height
		0,                //GLint  border
		GL_RGBA,          //GLenum  format
		GL_UNSIGNED_BYTE, //GLenum  type
		p->data           //const GLvoid * data
	);

	p->data = NULL;

	SDL_FreeSurface(img);

	return 0;
}



/*
 * соединение объекта с моделью
 * @return =NULL  соединение не удалось
 * @return != NULL адрес изображения
 */
/*
 * открытие файлов с рисунками меню
 */
static void __images_precache()
{

	int ret;
	size_t i = 0;
	size_t path_len = 0;
	char * path = NULL;

	for(i = 0; i < IMG__NUM; i++)
	{
		char * filename = image_files[i];
		size_t len = strlen(BASEDIR) + strlen(filename);
		if(!path || path_len < len)
		{
			char * tmp_path = Z_realloc(path, len + 1);
			if(!tmp_path) game_halt("Image load: out of memory");
			path = tmp_path;
			path_len = len;
		}
		strcpy(path, BASEDIR);
		strcat(path, filename);

		ret = __create_image_from_jpeg_png(path, i);
		if(ret)
			ret = __create_image_from_bii(path, i);
		if(ret)
			game_halt("Image \"%s\" load error %s", path, IMG_errorGet());
	}
	Z_free(path);
};

/*
 * удаление всех изображений из списка
 */
static void __cache_free()
{
	int i;
	for(i = 0; i < IMG__NUM; i++)
	{
		item_img_t * img = &image_table[i];

		glDeleteTextures(1, &img->texture);

		Z_free(img->data);
	}
}

/*
 * открытие файлов с рисунками меню
 */
int image_create_from_bitmap(
	void * bitmap,
	int sx,
	int sy,
	image_index_t iimage
)
{
	int ret = __create_from_bitmap(
		bitmap,
		sx,
		sy,
		iimage
	);
	return ret;
}

void images_init()
{
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	__images_precache();

};

void images_done()
{
	__cache_free();
	IMG_Quit();
}

const item_img_t * image_get(image_index_t iimage)
{
	return &image_table[iimage];
};



