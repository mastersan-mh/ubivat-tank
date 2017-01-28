/*
 * hash.h
 *
 *  Created on: 25 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_COMMON_HASH_H_
#define SRC_COMMON_HASH_H_

#include <inttypes.h>

uint32_t hash_ly(const char * str);

#define HASH32(x) hash_ly(x)


#endif /* SRC_COMMON_HASH_H_ */
