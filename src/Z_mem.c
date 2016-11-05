/*
 * Z_mem.c
 *
 *  Created on: 14 окт. 2016 г.
 *      Author: mastersan
 */

#include <Z_mem.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#if !defined(_ZMEM_DEBUG)
#	define _ZMEM_ARGS3_DECL
#	define _ZMEM_ARGS3
#else
#	define _ZMEM_ARGS3_DECL size_t __size, const char * __file, int __line
#	define _ZMEM_ARGS3 __size, __FILE__, __LINE__
#endif



typedef struct Z_block_s
{
	struct Z_block_s * next;
#if defined(_ZMEM_DEBUG)
#	define _ZMEM_FILENAME_LEN 1023
	size_t size;
	char file[_ZMEM_FILENAME_LEN + 1];
	int line;
#endif
	void * ptr;
}Z_block_t;


static Z_block_t * Z_block = NULL;

#define MAX_MESSAGE_SIZE 2048
static void Z_halt(const char * error, ...)
{
	int __errno_ = errno;
	static char errmsg[MAX_MESSAGE_SIZE];
	va_list argptr;
	va_start(argptr, error);
#ifdef HAVE_VSNPRINTF
	vsnprintf(errmsg, MAX_MESSAGE_SIZE, error, argptr);
#else
	vsprintf(errmsg, error, argptr);
#endif
	va_end(argptr);

	if(__errno_)
	{
		fprintf(stderr, "Z-mem: %s: %s\n", errmsg, strerror(__errno_));
	}
	else
	{
		fprintf(stderr, "Z-mem: %s\n", errmsg);
	}
	exit(1);
}

static Z_block_t * _create_block(_ZMEM_ARGS3_DECL)
{
	Z_block_t * block;
	block = calloc(1, sizeof(*block));
	block->next = Z_block;
#if defined(_ZMEM_DEBUG)
	block->size = __size;
	strncpy(block->file, __file, _ZMEM_FILENAME_LEN);
	block->line = __line;
#endif
	Z_block = block;
	return block;
}


void * Z_Z_malloc(size_t __size _ZMEM_ARGS2_DECL)
{

	Z_block_t * block = _create_block(_ZMEM_ARGS3);
	block->ptr = calloc(1, __size);
	return block->ptr;
}

void * Z_Z_calloc(size_t __count, size_t __eltsize _ZMEM_ARGS2_DECL)
{
#if defined(_ZMEM_DEBUG)
	size_t __size = __count * __eltsize;
#endif
	Z_block_t * block = _create_block(_ZMEM_ARGS3);
	block->ptr = calloc(__count, __eltsize);
	return block->ptr;
}


void * Z_Z_realloc(void * __ptr, size_t __size _ZMEM_ARGS2_DECL)
{
	if(!__ptr)return NULL;
	Z_block_t * block;
	Z_block_t * prev = NULL;
	block = Z_block;
	while(block)
	{
		if(block->ptr == __ptr)
		{
			void *tmp = realloc(__ptr, __size);
			if(!tmp)
			{
				if(prev)
					prev->next = block->next;
				else
					Z_block = block->next;
				free(block->ptr);
				free(block);
				return NULL;
			}
			block->ptr = tmp;
			return tmp;
		}
		prev = block;
		block = block->next;
	}
	return NULL;
}


void Z_Z_free(void * __ptr _ZMEM_ARGS2_DECL)
{
	int block_found = 0;
	if(!__ptr)return;
	Z_block_t * block;
	Z_block_t * prev = NULL;
	block = Z_block;
	while(block)
	{
		if(block->ptr == __ptr)
		{
			block_found = 1;
			if(prev)
				prev->next = block->next;
			else
				Z_block = block->next;
			free(block->ptr);
			free(block);
			return;
		}
		prev = block;
		block = block->next;
	}
	if(!block_found)
	{
#if !defined(_ZMEM_DEBUG)
#	define _TEXT "Block %p not found (already deallocated?)", __ptr
#else
#	define _TEXT "Block %p not found (already deallocated?) at %s: %d", __ptr, __file, __line
#endif
		Z_halt(_TEXT);
	}
}

char * Z_Z_strdup(const char * __str _ZMEM_ARGS2_DECL)
{
#if defined(_ZMEM_DEBUG)
	size_t __size = strlen(__str)+1;
#endif
	Z_block_t * block = _create_block(_ZMEM_ARGS3);
	char * dup = strdup(__str);
	block->ptr = dup;
	return dup;
}

char * Z_Z_strndup(const char * __str, size_t __size _ZMEM_ARGS2_DECL)
{
	Z_block_t * block = _create_block(_ZMEM_ARGS3);
	char * dup = strndup(__str, __size);
	block->ptr = dup;
	return dup;
}

/**
 * очистка всей выделенной памяти
 */
void Z_freeMemory()
{
	Z_block_t * block;
	while(Z_block)
	{
		block = Z_block;
		Z_block= Z_block->next;
		if(block->ptr) free(block->ptr);
		free(block);
	}
}
