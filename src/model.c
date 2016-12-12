/*
 * model.c
 *
 *  Created on: 11 дек. 2016 г.
 *      Author: mastersan
 */

#include "model.h"
#include "video.h"
#include "img.h"

#include <stdio.h>

vec2_t vertexes_common[VERTEXES_COMMON_NUM] =
{
		{-1.0, -1.0},
		{-1.0,  1.0},
		{ 1.0,  1.0},
		{ 1.0, -1.0}
};

vec2_t texcoord_common[VERTEXES_COMMON_NUM] =
{
		{ 0.0, 0.0},
		{ 0.0, 1.0},
		{ 1.0, 1.0},
		{ 1.0, 0.0}
};

model_triangle_t trianges_common[TRIANGLES_COMMON_NUM] =
{
		{0, 1, 2},
		{0, 2, 3}
};

static void model_render(
	camera_t * cam,
	vec2_t pos,
	model_t * model,
	vec_t modelscale,
	vec2_t translation
)
{

	item_img_t * texture = image_get(model->itexture);

	vec_t tr_x = ( cam->x + cam->sx / 2 + (pos.x - cam->pos.x) + translation.x ) * VIDEO_SCALEX;
	vec_t tr_y = ( cam->y + cam->sy / 2 - (pos.y - cam->pos.y) + translation.y ) * VIDEO_SCALEY;

	vec_t modelscale_x = modelscale * VIDEO_SCALEX;
	vec_t modelscale_y = modelscale * VIDEO_SCALEY;

	glBlendFunc(texture->sfactor, texture->dfactor);
	glBindTexture(GL_TEXTURE_2D, texture->texture);

	glLoadIdentity();
	glTranslatef(tr_x, tr_y, 0.0f);

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
		glTexCoord2f(
			model->texcoord[v].x,
			model->texcoord[v].y
		);
		glVertex2f(
			model->vertexes[v].x * modelscale_x,
			model->vertexes[v].y * modelscale_y
			);
		}
	}
	glEnd();
}

void ent_models_render(camera_t * cam, vec2_t pos, ent_model_t * ent_models)
{
	if(!ent_models) return;

	int i = 0;
	ent_model_t * ent_model;
	while( (ent_model = &ent_models[i++])->model != NULL )
	{
		model_render(
			cam,
			pos,
			ent_model->model,
			ent_model->modelscale,
			ent_model->translation
		);
	}
}


