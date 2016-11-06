/*
 * img.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_IMG_H_
#define SRC_IMG_H_

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#define BII_NOERR     0
#define BII_FATAL     1
#define BII_FILEERR   2
#define BII_READERR   3
#define BII_FORMATERR 4
#define BII_UNKNOWN   5

//рисунок
typedef struct
{
	//рисунок
	void * pic;
	//размер_рисунка_X
	int sx;
	//размер_рисунка_Y
	int sy;
} BIIpic_t;

/*
 * изображение
 */
typedef struct image_s
{
	struct image_s *next;
	/* идентификатор рисунка */
	char * name;
	//изображение
	BIIpic_t * IMG;
	GLuint textures;
	GLsizei sx;
	GLsizei sy;
	uint8_t * data;
} item_img_t;

extern int BII_errno;

GLsizei make_gl_size(int size);

char * IMG_errorGet();

int img_palette_read(const char * filename);

item_img_t * IMG_find(const char * name);
int IMG_add(const char * path, const char * IMGname);
void IMG_removeall();
item_img_t * IMG_connect(const char * IMGname);


#endif /* SRC_IMG_H_ */
