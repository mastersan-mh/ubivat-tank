/*
 * vars.h
 *
 *  Created on: 3 февр. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_VARS_H_
#define SRC_VARS_H_

#include "common_bstree.h"

#define VARNAME_SIZE (64)

/* тип переменной entity */
typedef enum
{
	VARTYPE_INTEGER,
	VARTYPE_FLOAT,
	VARTYPE_STRING
} vartype_t;

typedef int64_t var_int_t;
typedef float var_float_t;
typedef char var_char_t;

/* значение переменной */
typedef union
{
	var_int_t i64;
	var_float_t f;
	var_char_t *string;
} varvalue_t;

/* переменная entity */
typedef struct vardata_s
{
	//size_t index; /* номер переменной в списке entityvardata_t.vars */
	char name[VARNAME_SIZE];
	vartype_t type;
	varvalue_t value;
} vardata_t;

#define VARIABLE_STRING_DUP(text) \
	Z_strdup((text))
#define VARIABLE_STRING_ERASE(text) \
	do { Z_free((text)); (text) = NULL; } while(0)

typedef node_t var_t;
extern var_t * var_create(var_t ** root, const char *varname, vartype_t vartype);
extern var_t * var_find(const var_t * root, const char * varname);
extern void vars_delete(var_t ** root);
extern void vars_foreach(var_t * root, void (*action_cb)(vardata_t * vardata, void * args), void * args);
extern void vars_dump(var_t * root, const char * title);

#endif /* SRC_VARS_H_ */
