/*
 *        Ubivat Tank
 *        работа с изображениями
 * by Master San
 */

#include <img.h>
#include <_gr2D.h>
#include <Z_mem.h>

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

item_img_t * images = NULL;

int BII_errno;


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
int IMG_add(const char * path, const char * IMGname)
{
	item_img_t * p;
	item_img_t * find;

	find = IMG_find(IMGname); //ищем в списке
	if(find) return 4;               //нашли
	//не нашли, прочитаем новую
	BIIpic_t * IMG = BII_load(path);
	if(!IMG)
	{
		return -1;
	}
	p = Z_malloc(sizeof(*p));
	p->IMG = IMG;
	//прочитали успешно, добавим в список

	uint8_t * buf = IMG->pic;

	p->sx = make_gl_size(IMG->sx);
	p->sy = make_gl_size(IMG->sy);

	size_t size = p->sx * p->sy;
	int x,y;
	int i = 0;
	int iindex = 0;
#define COLOR_COMPONENT_AMOUNT 4
	p->data = Z_malloc(size*COLOR_COMPONENT_AMOUNT);
	uint8_t intencity = 4;
	for(y = 0; y < IMG->sy; y++)
	{
		for(x = 0;x < IMG->sx;x++)
		{
			int pindex = buf[i++];
			p->data[iindex++] = gr2D.PAL[pindex].R*intencity;
			p->data[iindex++] = gr2D.PAL[pindex].G*intencity;
			p->data[iindex++] = gr2D.PAL[pindex].B*intencity;

			//p->data[iindex++] = (pindex == 255) ? 0 : 255;
			//p->data[iindex++] = (pindex == 255) ? 255 : 0;
			p->data[iindex++] = (pindex == 255) ? 255 : 0;
		}
		iindex += (p->sx - IMG->sx)*COLOR_COMPONENT_AMOUNT;
	}

	// Загрузка картинки
	// Создание текстуры
	glGenTextures(1, &p->textures[0]);
	GLenum error = glGetError();
	if(error != GL_NO_ERROR || p->textures[0] == 0)
	{
		abort();
	}
	glBindTexture(GL_TEXTURE_2D, p->textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(
		GL_TEXTURE_2D,		//GLenum  target
		0,					//GLint  level
		GL_RGBA, //4,					//GLint  internalFormat
		p->sx,			//GLsizei  width
		p->sy,			//GLsizei  height
		0,					//GLint  border
		GL_RGBA,				//GLenum  format
		GL_UNSIGNED_BYTE,	//GLenum  type
		p->data		//const GLvoid * data
	);

	p->next = images;
	p->name = Z_strdup(IMGname);
	images = p;




	return 0;
}
/*
 * удаление всех моделей из списка
 */
void IMG_removeall()
{
	item_img_t * img;
	while(images)
	{
		img = images;

		glDeleteTextures(1, img->textures);

		BII_free(img->IMG);
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
item_img_t * IMG_connect(const char * IMGname)
{
	return IMG_find(IMGname);
};

