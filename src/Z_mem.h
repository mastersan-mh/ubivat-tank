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

#ifdef __GNUC__
#	define ATTR_MALLOC __attribute__ ((malloc))
#endif

void * Z_malloc(size_t size)
ATTR_MALLOC;

void * Z_calloc(size_t count, size_t eltsize)
ATTR_MALLOC;

char * Z_strdup(const char * str);

void Z_free(void * ptr);

void Z_freeMemory();

#endif /* SRC_Z_MEM_H_ */
