/*
 * Z_mem.h
 *
 *  Created on: 14 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_Z_MEM_H_
#define SRC_Z_MEM_H_

#include <stddef.h>
#include <sys/cdefs.h>

#define _ZMEM_DEBUG _DEBUG

#if !defined(_ZMEM_DEBUG)
#	define _ZMEM_ARGS2_DECL
#	define _ZMEM_ARGS2
#else
#	define _ZMEM_ARGS2_DECL , const char * __file, int __line
#	define _ZMEM_ARGS2 , __FILE__, __LINE__
#endif

#ifdef __GNUC__
#	define ATTR_MALLOC __attribute__ ((malloc))
#endif

#define Z_FREE(x) \
do \
{ \
	Z_free((x)); \
	(x) = NULL; \
}while(1);

void * Z_Z_malloc(size_t __size _ZMEM_ARGS2_DECL)
ATTR_MALLOC;

void * Z_Z_calloc(size_t __count, size_t __eltsize _ZMEM_ARGS2_DECL)
ATTR_MALLOC;

void * Z_Z_realloc(void *__ptr, size_t __size _ZMEM_ARGS2_DECL)
__THROW __attribute_warn_unused_result__;

char * Z_Z_strdup(const char * __str _ZMEM_ARGS2_DECL);
char * Z_Z_strndup(const char * __str, size_t __size _ZMEM_ARGS2_DECL);

void Z_Z_free(void * __ptr _ZMEM_ARGS2_DECL);



#define Z_malloc(__size)             Z_Z_malloc ((__size)               _ZMEM_ARGS2)
#define Z_calloc(__count, __eltsize) Z_Z_calloc ((__count), (__eltsize) _ZMEM_ARGS2);
#define Z_realloc(__ptr, __size)     Z_Z_realloc((__ptr), (__size)      _ZMEM_ARGS2);
#define Z_strdup(__str)              Z_Z_strdup ((__str)                _ZMEM_ARGS2);
#define Z_strndup(__str, __size)     Z_Z_strndup((__str), (__size)      _ZMEM_ARGS2);
#define Z_free(__ptr)                Z_Z_free   ((__ptr)                _ZMEM_ARGS2);



void Z_freeMemory();

#endif /* SRC_Z_MEM_H_ */
