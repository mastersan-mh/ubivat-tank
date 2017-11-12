/*
 * ui.h
 *
 *  Created on: 11 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_UI_H_
#define SRC_UI_H_

#include "types.h"
#include "entity_internal.h"

extern void ui_init();

extern void ui_done();

extern void ui_draw(camera_t * cam, entity_t * entity);

extern void ui_register(
	void (*cb)(camera_t * cam, ENTITY entity)
);

extern void ui_drawimage(camera_t * cam, int x, int y, image_index_t iimage);
extern void ui_printf(camera_t * cam,int x, int y, const char * format, ...);

#endif /* SRC_UI_H_ */
