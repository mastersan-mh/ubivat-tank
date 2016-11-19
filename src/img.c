/*
 *        Ubivat Tank
 *        работа с изображениями
 * by Master San
 */

#include <img.h>
#include <_gr2D.h>
#include <Z_mem.h>
#include <video.h>

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

typedef struct
{
	char R,G,B;
} T2DRGBcolor;

typedef T2DRGBcolor T2Dpal[256];

int BII_errno;

item_img_t * images = NULL;
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
static BIIpic_t * BII_load(const char * fname)
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

static void BII_free(BIIpic_t * pic)
{
	Z_free(pic->pic);
	Z_free(pic);
}

/*
 * поиск модели в существующем списке
 * @return =NULL  -модель не найдена
 * @return != NULL -указатель на модель
 */
item_img_t * IMG_find(const char * name)
{
	item_img_t * img = images;
	while(img)
	{
		if(!strcmp(img->name, name)) return img;
		img = img->next;
	}
	return NULL;
};

GLsizei make_gl_size(int size)
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
			data[iindex++] = pal[pindex].R * intencity;
			data[iindex++] = pal[pindex].G * intencity;
			data[iindex++] = pal[pindex].B * intencity;
			data[iindex++] = (pindex == TRANSPARENT_COLOR_INDEX) ? COLOR_ALPHA_TRANSPARENT : COLOR_ALPHA_OPAQUE;
		}
		iindex += (bitmap_sx - sx)*COLOR_COMPONENT_AMOUNT;
	}
	return data;

}


static int __create_from_bitmap(
	void * bitmap,
	int sx,
	int sy,
	const char * IMGname
)
{

	item_img_t * p;

	p = Z_malloc(sizeof(*p));
	p->img_sx = sx;
	p->img_sy = sy;
	p->texture_sx = make_gl_size(sx);
	p->texture_sy = make_gl_size(sy);
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

	p->name = Z_strdup(IMGname);
	p->next = images;
	images = p;
	return 0;
}


int IMG_create_from_bitmap(
	void * bitmap,
	int sx,
	int sy,
	const char * IMGname
)
{
	if(IMG_find(IMGname)) return -1;
	int ret = __create_from_bitmap(
		bitmap,
		sx,
		sy,
		IMGname
	);
	return ret;
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
int IMG_create_from_bii(const char * path, const char * IMGname)
{
	if(IMG_find(IMGname)) return -1;

	//не нашли, прочитаем новую
	BIIpic_t * bii_image = BII_load(path);
	if(!bii_image) return -1;
	int ret = __create_from_bitmap(
		bii_image->pic,
		bii_image->sx,
		bii_image->sy,
		IMGname
	);
	BII_free(bii_image);
	return ret;
}




/*
 * удаление всех изображений из списка
 */
void IMG_removeall()
{
	item_img_t * img;
	while(images)
	{
		img = images;

		glDeleteTextures(1, &img->texture);

		images = images->next;
		Z_free(img->data);
		Z_free(img->name);
		Z_free(img);
	}
}
/*
 * соединение объекта с моделью
 * @return =NULL  соединение не удалось
 * @return != NULL адрес изображения
 */
item_img_t * IMG_get(const char * IMGname)
{
	return IMG_find(IMGname);
};

