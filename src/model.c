/*
 * model.c
 *
 *  Created on: 11 дек. 2016 г.
 *      Author: mastersan
 */

#include "model.h"
#include "video.h"
#include "img.h"
#include "game.h"

#include <stdio.h>

/* вершины, коордитаты и треугольники для модели по-умолчанию */
vec2_t vertexes_common[VERTEXES_COMMON_NUM] =
{
		{-1.0, -1.0},
		{-1.0,  1.0},
		{ 1.0,  1.0},
		{ 1.0, -1.0}
};

model_triangle_t trianges_common[TRIANGLES_COMMON_NUM] =
{
		{0, 1, 2},
		{0, 2, 3}
};

vec2_t texcoord_common[VERTEXES_COMMON_NUM] =
{
		{ 0.0, 0.0},
		{ 0.0, 1.0},
		{ 1.0, 1.0},
		{ 1.0, 0.0}
};

modelframe_t model_frames_common[MODEL_FRAMES_COMMON_NUM] =
{
		{ .texcoord = texcoord_common }
};


void model_render(
	camera_t * cam,
	vec2_t pos,
	model_t * model,
	vec_t modelscale,
	vec2_t translation,
	float angle, /* degrees*/
	unsigned int iframe
)
{

	if(iframe >= model->frames_num)
	{
		game_console_send("Model %s: iframe = %d is greater than or equal to frames_num = %d.", model->name, iframe, model->frames_num);
		return;
	}

	item_img_t * texture = image_get(model->itexture);

	vec_t tr_x = ( cam->x + cam->sx / 2 + (pos.x - cam->pos.x) + translation.x ) * VIDEO_SCALEX;
	vec_t tr_y = ( cam->y + cam->sy / 2 - (pos.y - cam->pos.y) + translation.y ) * VIDEO_SCALEY;

	vec_t modelscale_x = modelscale * VIDEO_SCALE;
	vec_t modelscale_y = modelscale * VIDEO_SCALE;

	glBlendFunc(texture->sfactor, texture->dfactor);
	glBindTexture(GL_TEXTURE_2D, texture->texture);

	glLoadIdentity();
	glTranslatef(tr_x, tr_y, 0.0f);

	glRotatef(angle, 0, 0, 1);

	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 1.0f, 1.0f);

	int itriangle;
	for( itriangle = 0; itriangle < model->triangles_num; itriangle++ )
	{
		int ivertex;
		for( ivertex = 0; ivertex < 3; ivertex ++)
		{
			//model_triangle_t *triangle =
			int v = model->triangles[itriangle][ivertex];
			modelframe_t * frame = &model->frames[iframe];
			glTexCoord2f(
				frame->texcoord[v].x,
				frame->texcoord[v].y
			);
			glVertex2f(
				model->vertexes[v].x * modelscale_x,
				model->vertexes[v].y * modelscale_y
			);
		}
	}
	glEnd();
}



/**
 * @description загнать значение в диапазон
 * @param[in] val - значение
 * @param[in] min - минимум
 * @param[in] max - максимум
 * @param[out] coerced - == NULL - не использовать параметр
 *                       != NULL - параметр используется:
 *                           == -1 - поправлена нижняя граница
 *                           == 0  - значение val в диапазоне
 *                           == 1  - поправлена верхняя граница
 * @return значение в диапазоне [min; max]
 */
unsigned int coerce_uint(unsigned int val, unsigned int min, unsigned int max, int * coerced)
{
    if(val < min)
    {
        if(coerced) *coerced = -1;
        return min;
    }
    if(val > max)
    {
        if(coerced) *coerced = +1;
        return max;
    }
    if(coerced) *coerced = 0;
    return val;
}

