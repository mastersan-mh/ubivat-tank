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

/**
 * @description константа с координатами текстур, где кадры в текстуре располагаются вертикально
 * model_<prefix>
 * frame      - номер кадра = [0; frames_num)
 * frames_num - количество кадров
 */
#define MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(prefix, frame, frames_num) \
vec2_t model_texcoord_ ## prefix [VERTEXES_COMMON_NUM] =   \
{                                              \
		{ 0.0f, ( (float)frame        ) / ( (float)frames_num ) }, \
		{ 0.0f, ( (float)frame + 1.0f ) / ( (float)frames_num ) }, \
		{ 1.0f, ( (float)frame + 1.0f ) / ( (float)frames_num ) }, \
		{ 1.0f, ( (float)frame        ) / ( (float)frames_num ) }  \
}

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

#define VERTEXES_COMMON_NUM (4)
#define TRIANGLES_COMMON_NUM (2)
#define MODEL_FRAMES_COMMON_NUM (1)

extern vec2_t model_vertexes_common[VERTEXES_COMMON_NUM];
extern model_triangle_t model_trianges_common[TRIANGLES_COMMON_NUM];
extern vec2_t model_texcoord_common[VERTEXES_COMMON_NUM];
extern modelframe_t model_frames_common[MODEL_FRAMES_COMMON_NUM];

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
