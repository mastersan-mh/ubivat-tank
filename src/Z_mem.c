/*
 * Z_mem.c
 *
 *  Created on: 14 окт. 2016 г.
 *      Author: mastersan
 */

#include <Z_mem.h>

#include <stdlib.h>
#include <string.h>

typedef struct Z_block_s
{
	struct Z_block_s * next;
	void * ptr;
}Z_block_t;


static Z_block_t * Z_block = NULL;


static Z_block_t * _create_block()
{
	Z_block_t * block;
	block = calloc(1, sizeof(*block));
	block->next = Z_block;
	Z_block = block;
	return block;
}


void * Z_malloc(size_t size)
{
	Z_block_t * block = _create_block();
	block->ptr = calloc(1, size);
	return block->ptr;
}

void * Z_calloc(size_t count, size_t eltsize)
{
	Z_block_t * block = _create_block();
	block->ptr = calloc(count, eltsize);
	return block->ptr;
}


void * Z_realloc(void * ptr, size_t size)
{
	if(!ptr)return NULL;
	Z_block_t * block;
	Z_block_t * prev = NULL;
	block = Z_block;
	while(block)
	{
		if(block->ptr == ptr)
		{
			void *tmp = realloc(ptr, size);
			if(!tmp)
			{
				if(prev)
					prev->next = block->next;
				else
					Z_block = block->next;
				free(block->ptr); // ?????
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


void Z_free(void * ptr)
{
	if(!ptr)return;
	Z_block_t * block;
	Z_block_t * prev = NULL;
	block = Z_block;
	while(block)
	{
		if(block->ptr == ptr)
		{
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
}

char * Z_strdup(const char * str)
{
	Z_block_t * block = _create_block();
	char * dup = strdup(str);
	block->ptr = dup;
	return dup;
}

char * Z_strndup(const char * str, size_t size)
{
	Z_block_t * block = _create_block();
	char * dup = strndup(str, size);
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
