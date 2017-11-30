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

static const model_t ** models = NULL;
static size_t models_size = 0;
static size_t models_num = 0;

/**
 * @description получить зарегестрированную модель
 */
const model_t * model_get(const char * modelname)
{
    const char * name;
    if(strncmp(modelname, ":/", 2) == 0)
        name = modelname + 2; /* внутренняя модель */
    else
        return NULL; /* внешние модели не реализованы */

    size_t i;
    for(i = 0; i < models_num; i++)
    {
        if(strncmp(models[i]->name, name, 64) == 0)
            return models[i];
    }
    return NULL;
}

/**
 * @description регистрация модели
 */
void model_register(const model_t * model)
{
	const model_t ** tmp;
	if(model == NULL)
	{
		game_console_send("Model registration failed: Model data is NULL.");
		return;
	}

	if(model->name == NULL || strlen(model->name) == 0)
	{
		game_console_send("Model registration failed: Model name is empty.");
		return;
	}

	if(model_get(model->name) != NULL)
	{
			game_console_send("Model \"%s\" registration failed: Duplicate model name.", model->name);
			return;
	}

	if(model->frames_num == 0)
		game_console_send("Model \"%s\" registration warning: .frames_num == 0.", model->name);

    if(models_size < models_num + 1)
    {
        if(models_size == 0)
            models_size = 1;
        else
            models_size *= 2;
        tmp = Z_realloc(models, sizeof(model_t*) * models_size);
        if(!tmp)
            game_halt("Model registration failed: Out of memory");
        models = tmp;
    }
    models[models_num] = model;
    models_num++;
    game_console_send("Model \"%s\" registered.", model->name);
}

void model_render(
	const camera_t * cam,
	vec2_t pos,
	const model_t * model,
	vec_t modelscale,
	const vec2_t translation,
	float angle, /* degrees*/
	unsigned int iframe
)
{

	if(!model)
	{
		game_console_send("Error: No entity model.");
		return;
	}

	if(iframe >= model->frames_num)
	{
		game_console_send("Model %s: iframe = %d is greater than or equal to frames_num = %d.", model->name, iframe, model->frames_num);
		return;
	}

	const item_img_t * texture = image_get(model->itexture);

	vec_t tr_x = ( cam->x + cam->sx / 2 + (pos[0] - cam->origin[0]) + translation[0] ) * VIDEO_SCALE;
	vec_t tr_y = ( cam->y + cam->sy / 2 - (pos[1] - cam->origin[1]) + translation[1] ) * VIDEO_SCALE;

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
				frame->texcoord[v][0],
				frame->texcoord[v][1]
			);
			glVertex2f(
				model->vertexes[v][0] * modelscale_x,
				model->vertexes[v][1] * modelscale_y
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

