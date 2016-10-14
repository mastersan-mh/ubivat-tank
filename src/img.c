/*
 *        Ubivat Tank
 *        работа с изображениями
 * by Master San
 */

#include <defs.h>

#include <errno.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/*
 * чтение рисунка
 * @return = 0 успешно
 * @return = 1 файл не найден
 * @return = 2 ошибка формата
 * @return = 3 ошибка чтения
 */
static int BII_load(const char * path, TBIIpic * pic)
{
	char BIIheader[3];
	int fd;
	char * fname;
	fname = Z_malloc(strlen(path)+4+1);
	strcpy(fname, path);
	strcat(fname, ".bii");
	fd = open(fname, O_RDONLY);
	Z_free(fname);
	//файл не найден
	if(fd < 0) return 1;
	//читаем заголовок
	read(fd, BIIheader, 3);
	if(errno) return 3;
	//проверяем прочитаный префикс
	if(
		(BIIheader[0] != c_BIIheader[0]) ||
		(BIIheader[1] != c_BIIheader[1]) ||
		(BIIheader[2] != c_BIIheader[2])
	)
		return 2; //ошибка формата
	//читаем размер X, Y
	read(fd, &pic->sx, 2);
	read(fd, &pic->sy, 2);
	pic->sx = pic->sx+1;
	pic->sy = pic->sy+1;
	//выделим память
	size_t size = pic->sx*pic->sy;
	pic->pic = Z_malloc(size);
	//читаем рисунок
	read(fd, pic->pic, size);
	if(errno)
	{
		close(fd);
		return 3;
	}
	close(fd);
	return 0;
};

static void BII_free(TBIIpic * pic)
{
	Z_free(pic->pic);
}

/*
 * поиск модели в существующем списке
 * @return =NULL  -модель не найдена
 * @return != NULL -указатель на модель
 */
item_img_t * IMG_find(item_img_t * HEAD, const char * IMGname)
{
	while(HEAD)
	{
		if(!strcmp(HEAD->IMGname, IMGname)) return HEAD;
		HEAD = HEAD->next;
	}
	return NULL;
};
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
int IMG_add(item_img_t ** HEAD, const char * path, const char * IMGname)
{
	int error;
	item_img_t * p;
	item_img_t * find;

	find = IMG_find(*HEAD, IMGname); //ищем в списке
	if(find) return 4;               //нашли
	//не нашли, прочитаем новую
	p = Z_malloc(sizeof(*p));
	error = BII_load(path, &(p->IMG));
	if(error)
	{
		//ошибка чтения
		Z_free(p);
		return error;
	}
	//прочитали успешно, добавим в список
	p->next = *HEAD;
	strcpy(p->IMGname, IMGname);
	*HEAD = p;
	return 0;
}
/*
 * удаление всех моделей из списка
 */
void IMG_removeall(item_img_t ** HEAD)
{
	item_img_t * p;
	while(*HEAD)
	{
		p = *HEAD;
		BII_free(&p->IMG);
		*HEAD = (*HEAD)->next;
		Z_free(p);
	}
}
/*
 * соединение объекта с моделью
 * @return =NULL  соединение не удалось
 * @return != NULL адрес изображения
 */
item_img_t * IMG_connect(item_img_t * HEAD, const char * IMGname)
{
	return IMG_find(HEAD, IMGname);
};

