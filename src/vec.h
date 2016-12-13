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

#define VEC_ABS(x) fabsf((x))
#define VEC_SQRT(x) sqrtf((x))
#define VEC_TRUNC(x) truncf((x))
#define VEC_ROUND(x) roundf((x))

#endif /* SRC_VEC_H_ */
