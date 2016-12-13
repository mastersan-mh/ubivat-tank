/*
 * model.h
 *
 *  Created on: 11 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_MODEL_H_
#define SRC_MODEL_H_

#include "types.h"
#include "img.h"

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

extern vec2_t vertexes_common[VERTEXES_COMMON_NUM];
extern model_triangle_t trianges_common[TRIANGLES_COMMON_NUM];
extern vec2_t texcoord_common[VERTEXES_COMMON_NUM];
extern modelframe_t model_frames_common[MODEL_FRAMES_COMMON_NUM];

#include "mobjs.h"

void model_render(
	camera_t * cam,
	vec2_t pos,
	model_t * model,
	vec_t modelscale,
	vec2_t translation,
	float angle, /* degrees*/
	unsigned int iframe
);


#endif /* SRC_MODEL_H_ */
