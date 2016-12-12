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

typedef struct
{
	char * name;
	int vertexes_num;
	vec2_t * vertexes;
	int triangles_num;
	model_triangle_t * triangles;
	image_index_t itexture;
	vec2_t * texcoord;
} model_t;

typedef struct
{
	model_t * model;
	vec_t modelscale;
	vec2_t translation;
} ent_model_t;

void ent_models_render(camera_t * cam, vec2_t pos, ent_model_t * ent_models);

#define VERTEXES_COMMON_NUM (4)
#define TRIANGLES_COMMON_NUM (2)

extern vec2_t vertexes_common[VERTEXES_COMMON_NUM];
extern vec2_t texcoord_common[VERTEXES_COMMON_NUM];
extern model_triangle_t trianges_common[TRIANGLES_COMMON_NUM];


#endif /* SRC_MODEL_H_ */
