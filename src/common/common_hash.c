/*
 * hash.c
 *
 *  Created on: 25 янв. 2017 г.
 *      Author: mastersan
 */

#include "common_hash.h"

/* Линейный конгруэнтный метод */
uint32_t hash_ly(const char * str)
{

	uint32_t hash = 0;

	for(; *str; str++)
		hash = (hash * 1664525) + (unsigned char)(*str) + 1013904223;

	return hash;

}
