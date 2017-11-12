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
typedef vec_t vec2_t[2];

#define VEC_ABS(x)   ( (vec_t)fabsf((x)) )
#define VEC_SQRT(x)  ( (vec_t)sqrtf((x)) )
#define VEC_TRUNC(x) ( (vec_t)truncf((x)) )
#define VEC_ROUND(x) ( (vec_t)roundf((x)) )

#define DOT_PRODUCT2(a, b) ( (a)[0] * (b)[0] + (a)[1] * (b)[1] )
#define VEC2_CLEAR(a)      ( (a)[0] = (a)[1] = 0 )
#define VEC2_COMPARE(a, b) ( ((a)[0] == (b)[0]) && ((a)[1] == (b)[1]) )
#define VEC2_COPY(dest, src)   ( (dest)[0] = (src)[0], (dest)[1] = (src)[1] )
#define VEC2_NEGATE(dest, src) ( (dest)[0] = -((src)[0]), (dest)[1] = -((src)[1]) )
#define VEC2_SET(dest, x, y)   ( (dest)[0] = (x), (dest)[1] = (y) )
#define VEC2_SCALE(dest, scale, src) ((dest)[0] = (src)[0] * (scale), (dest)[1] = (src)[1] * (scale))
#define VEC2_NORMALIZE2(dest, v) \
		{ \
			vec_t ilength = VEC_SQRT( DOT_PRODUCT2((v),(v)) ); \
			if (ilength) \
				ilength = 1.0f / ilength; \
			dest[0] = (v)[0] * ilength; \
			dest[1] = (v)[1] * ilength; \
		}

#define VEC2_ADD(dest, src1, src2) ( (dest)[0] = (src1)[0] + (src2)[0], (dest)[1] = (src1)[1] + (src2)[1] )
#define VEC2_SUB(dest, src1, src2) ( (dest)[0] = (src1)[0] - (src2)[0], (dest)[1] = (src1)[1] - (src2)[1] )


#endif /* SRC_VEC_H_ */
