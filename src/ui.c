/*
 * ui.c
 *
 *  Created on: 11 янв. 2017 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"
#include "ui.h"
#include "Z_mem.h"
#include "_gr2D.h"

#include <stdarg.h>

typedef struct uient_s
{
	struct uient_s * prev;
	struct uient_s * next;
	void (*cb)(camera_t * cam, entity_t * entity);

} uient_t;

uient_t * uients = NULL;

void ui_init()
{
}

void ui_done()
{
	uient_t * uient;
	while(uients)
	{
		uient = uients;
		uients = uients->next;
		Z_free(uient);
	}
}

void ui_draw(camera_t * cam, entity_t * entity)
{
	uient_t * uient;
	LIST2_FOREACH(uients, uient)
	{
		uient->cb(cam, entity);
	}

}

void ui_register(
	void (*cb)(camera_t * cam, entity_t * entity)
)
{
	uient_t * uient = Z_malloc(sizeof(uient_t));
	uient->cb = cb;
	LIST2_PUSH(uients, uient);
}

void ui_drawimage(camera_t * cam, int x, int y, image_index_t iimage)
{
	video_image_draw(cam->x + x, cam->y + y, iimage);
}

void ui_printf(camera_t * cam, int x, int y, const char * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	video_vprintf(cam->x + x, cam->y + y, format, argptr);
	va_end(argptr);
}
