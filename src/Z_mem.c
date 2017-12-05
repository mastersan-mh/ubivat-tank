/*
 * Z_mem.c
 *
 *  Created on: 14 окт. 2016 г.
 *      Author: mastersan
 */

#include "Z_mem.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#if !defined(_ZMEM_DEBUG)
#	define _ZMEM_ARGS3_DECL
#	define _ZMEM_ARGS3
#	define _ZMEM_ARGS3STRNDUP
#	define Z_MEM_DUMP()
#else
#	define _ZMEM_ARGS3_DECL size_t __size, const char * __file, int __line
#	define _ZMEM_ARGS3 __size, __file, __line
#	define _ZMEM_ARGS3STRNDUP __size + 1, __file, __line
#	if !defined(_ZMEM_DEBUG_MEMDUMP)
#		define Z_MEM_DUMP()
#	else
#		define Z_MEM_DUMP() Z_mem_dump()
#	endif
#endif



typedef struct Z_block_s
{
	struct Z_block_s * next;
#if defined(_ZMEM_DEBUG)
#	define _ZMEM_FILENAME_LEN 1023
	int id;
	size_t size;
	char file[_ZMEM_FILENAME_LEN + 1];
	int line;
#endif
	void * ptr;
}Z_block_t;


static Z_block_t * Z_block = NULL;

#include <sys/types.h>
#include <unistd.h>


#if defined(_ZMEM_DEBUG)
static int block_id = 0;
#endif

#if defined(_ZMEM_DEBUG_MEMDUMP)

static int mem_dump_i = 0;

void Z_mem_dump()
{
#define BUF_SIZE 256

	if(mem_dump_i == 0)
	{
		unlink("ubivat-tank-dump.log");
	}

	static char buf[BUF_SIZE];
	size_t size;
	FILE * fd = fopen("ubivat-tank-dump.log", "a");
	if(fd == NULL)return;
	fseek(fd, 0, SEEK_END);
	Z_block_t * block;
	block = Z_block;
	fprintf(fd, "[ MEM DUMP %d ]\n", mem_dump_i);
	int i = 0;
	while(block)
	{
		fprintf(fd, "    %d:%d; id : %d , file: %s, line: %d, alloc size: %ld\n", mem_dump_i, i, block->id, block->file, block->line, block->size);
		size = block->size > BUF_SIZE ? BUF_SIZE : block->size;
		memcpy(buf, block->ptr, size);
		buf[BUF_SIZE-1] = 0;
		fprintf(fd, "        string: \"%s\"\n", buf);
		block = block->next;
		i++;
	}

	fclose(fd);
	mem_dump_i++;
}
#endif

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
	Z_MEM_DUMP();
	exit(1);
}

static Z_block_t * _create_block(_ZMEM_ARGS3_DECL)
{
	Z_block_t * block;
	block = calloc(1, sizeof(*block));
	block->next = Z_block;
#if defined(_ZMEM_DEBUG)
	block->id = block_id++;
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

#if defined(_ZMEM_DEBUG) && defined(_ZMEM_DEBUG_TRACE)
	printf("Z_malloc(): %ld bytes @ ptr %p at %s: %d\n", (long) __size, block->ptr, __file, __line);
#endif

	Z_MEM_DUMP();
	return block->ptr;
}

void * Z_Z_calloc(size_t __count, size_t __eltsize _ZMEM_ARGS2_DECL)
{
#if defined(_ZMEM_DEBUG)
	size_t __size = __count * __eltsize;
#endif
	Z_block_t * block = _create_block(_ZMEM_ARGS3);
	block->ptr = calloc(__count, __eltsize);

#if defined(_ZMEM_DEBUG) && defined(_ZMEM_DEBUG_TRACE)
	printf("Z_calloc(): %ld bytes @ ptr %p at %s: %d\n", (long) __size, block->ptr, __file, __line);
#endif

	Z_MEM_DUMP();
	return block->ptr;
}


void * Z_Z_realloc(void * __ptr, size_t __size _ZMEM_ARGS2_DECL)
{
	if(!__ptr)
	{
		return Z_Z_malloc(__size _ZMEM_ARGS2);
	}
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

#if defined(_ZMEM_DEBUG) && defined(_ZMEM_DEBUG_TRACE)
	printf("Z_realloc(): ptr %p -> %ld bytes @ ptr %p at %s: %d\n", __ptr, (long) __size, block->ptr, __file, __line);
#endif

			return tmp;
		}
		prev = block;
		block = block->next;
	}
	return NULL;
}


void Z_Z_free(void * __ptr _ZMEM_ARGS2_DECL)
{
	if(!__ptr)
		return;
#if defined(_ZMEM_DEBUG) && defined(_ZMEM_DEBUG_TRACE)
	printf("Z_free(): %p at %s: %d\n", __ptr, __file, __line);
#endif
	Z_block_t * block;
	Z_block_t * prev = NULL;
	block = Z_block;
	while(block)
	{
		if(block->ptr == __ptr)
		{
			if(prev)
				prev->next = block->next;
			else
				Z_block = block->next;
			free(block->ptr);
			free(block);
			Z_MEM_DUMP();
			return;
		}
		prev = block;
		block = block->next;
	}
#if !defined(_ZMEM_DEBUG)
#	define _TEXT "Block %p not found (already deallocated?)", __ptr
#else
#	define _TEXT "Block %p not found (already deallocated?) at %s: %d", __ptr, __file, __line
#endif
	Z_halt(_TEXT);
}

char * Z_Z_strdup(const char * __str _ZMEM_ARGS2_DECL)
{
#if defined(_ZMEM_DEBUG)
	size_t __size = strlen(__str)+1;
#endif
	Z_block_t * block = _create_block(_ZMEM_ARGS3);
	char * dup = strdup(__str);
	block->ptr = dup;

#if defined(_ZMEM_DEBUG) && defined(_ZMEM_DEBUG_TRACE)
	printf("Z_strdup(): %ld bytes @ ptr %p at %s: %d\n", (long) __size, block->ptr, __file, __line);
#endif

	Z_MEM_DUMP();
	return dup;
}

char * Z_Z_strndup(const char * __str, size_t __size _ZMEM_ARGS2_DECL)
{
	Z_block_t * block = _create_block(_ZMEM_ARGS3STRNDUP);
	char * dup = strndup(__str, __size);
	block->ptr = dup;

#if defined(_ZMEM_DEBUG) && defined(_ZMEM_DEBUG_TRACE)
	printf("Z_strndup(): %ld bytes @ ptr %p at %s: %d\n", (long) __size, block->ptr, __file, __line);
#endif

	Z_MEM_DUMP();
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
