/*
 * entity_helpers.c
 *
 *  Created on: 9 янв. 2017 г.
 *      Author: mastersan
 */

#include "types.h"
#include "system.h"
#include "entity_helpers.h"

/**
 * получить любой объект из заданной группы
 */
entity_t * entity_get_random(const char * entityname)
{
	size_t count = 0;
	entity_t * entity;
	/* считаем количество */
	ENTITIES_FOREACH(entityname, entity)
	{
		count++;
	};
	if(count == 0)
		return NULL;
	count = xrand(count);

	/* выбираем случайным образом */
	ENTITIES_FOREACH(entityname, entity)
	{
		if(count == 0)
			return entity;
		count--;
	};
	return NULL;
}
