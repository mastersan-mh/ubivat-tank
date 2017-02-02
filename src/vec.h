/*
 * vec.h
 *
 *  Created on: 13 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_VEC_H_
#define SRC_VEC_H_

typedef float vec_t;

//координаты
typedef struct
{
	vec_t x;
	vec_t y;
}vec2_t;

#define VEC_ABS(x)   ( (vec_t)fabsf((x)) )
#define VEC_SQRT(x)  ( (vec_t)sqrtf((x)) )
#define VEC_TRUNC(x) ( (vec_t)truncf((x)) )
#define VEC_ROUND(x) ( (vec_t)roundf((x)) )

#endif /* SRC_VEC_H_ */
