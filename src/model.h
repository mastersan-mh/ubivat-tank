/*
 * model.h
 *
 *  Created on: 11 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_MODEL_H_
#define SRC_MODEL_H_

#include "vec.h"
#include "types.h"
#include "img.h"

/* треугольник: номера вершин */
typedef int model_triangle_t[3];

/* кадр модели */
typedef struct
{
	/* координаты текстуры */
	vec2_t * texcoord;
} modelframe_t;

typedef struct
{
	char * name;
	unsigned int vertexes_num;
	vec2_t * vertexes;
	unsigned int triangles_num;
	model_triangle_t * triangles;
	image_index_t itexture;
	/* frames per second */
	unsigned int fps;
	/* количество кадров текстуры модели*/
	unsigned int frames_num;
	modelframe_t * frames;
} model_t;

const model_t * model_get(const char * name);

void model_register(const model_t * model);

void model_render(
	const camera_t * cam,
	vec2_t pos,
	const model_t * model,
	vec_t modelscale,
	vec2_t translation,
	float angle, /* degrees*/
	unsigned int iframe
);


#endif /* SRC_MODEL_H_ */
