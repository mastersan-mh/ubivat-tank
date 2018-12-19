/*
 * vars.h
 *
 *  Created on: 3 февр. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_VARS_H_
#define SRC_VARS_H_

#include "types.h"
#include "vec.h"
#include "game.h"

#include <inttypes.h>
#include <stdint.h>

#include <unistd.h>
#include <stddef.h>

#define VARNAME_SIZE (64)

/* тип переменной entity */
typedef enum
{
    VARTYPE_BOOL,
    VARTYPE_INTEGER,
    VARTYPE_FLOAT,
    VARTYPE_VECTOR2,
    VARTYPE_DIRECTION,
    VARTYPE_STRING,
} vartype_t;

/** @brief описатель переменной entity */
typedef struct
{
    /** @brief Variable type */
    vartype_t type;
    /** @brief Variable name */
    const char * name;
    /** @brief Variable offset in structure */
    intptr_t ofs;
    /** @brief Variable size */
    size_t size;
} var_descr_t;

#define VAR_DESCR(VARTYPE, TYPE, NAME) { VARTYPE, #NAME, offsetof(TYPE, NAME), SIZEOF_MEMBER(TYPE, NAME) }

/* types */
#define BOOL bool
#define INTEGER int64_t
#define FLOAT float
#define VECTOR2 vec2_t
#define DIRECTION direction_t
#define STRING char *

#define VARIABLE_STRING_DUP(text) \
	Z_strdup((text))
#define VARIABLE_STRING_ERASE(text) \
	do { Z_free((text)); (text) = NULL; } while(0)

typedef struct
{
    /** @brief Variable type */
    vartype_t type;
    /** @brief Variable name */
    const char * name;
    union
    {
        char blob;
        BOOL vbool;
        INTEGER vinteger;
        FLOAT vfloat;
        VECTOR2 vvector2;
        DIRECTION vdirection;
        STRING vstring;
    };
} var_value_t;

#define VAR_VALUE_BOOL(NAME, VALUE) \
        { .type = VARTYPE_BOOL, .name = (NAME), .vbool = (VALUE) }

#define VAR_VALUE_INTEGER(NAME, VALUE) \
        { .type = VARTYPE_INTEGER, .name = (NAME), .vinteger = (VALUE) }

#define VAR_VALUE_FLOAT(NAME, VALUE) \
        { .type = VARTYPE_FLOAT, .name = (NAME), .vfloat = (VALUE) }

#define VAR_VALUE_VECTOR2(NAME, VALUE_X, VALUE_Y) \
        { .type = VARTYPE_VECTOR2, .name = (NAME), .vvector2 = {(VALUE_X), (VALUE_Y)} }

#define VAR_VALUE_DIRECTION(NAME, VALUE) \
        { .type = VARTYPE_DIRECTION, .name = (NAME), .vdirection = (VALUE) }

#define VAR_VALUE_STRING(NAME, VALUE) \
        { .type = VARTYPE_STRING, .name = (NAME), .vstring = (VALUE) }

const var_descr_t * var_find(const var_descr_t * vars_descr, size_t vars_descr_num, const char * name);

int var_set(
    void * vars,
    const var_descr_t * vars_descr,
    size_t vars_descr_num,
    const char * key,
    const char * value
);

extern void vars_free(void * vars, const var_descr_t * vars_descr, size_t vars_descr_num);

extern size_t var_buffersize_calculate(const var_descr_t * vars_descr, size_t vars_descr_num);

bool vars_descr_eq(const var_descr_t * vds1, const var_descr_t * vds2, size_t size);

void vars_dump(void * vars, const var_descr_t * vars_descr, size_t vars_descr_num);

bool infovars_get_next_key(
    const char ** info,
    const char ** key_begin,
    const char ** key_end,
    const char ** value_begin,
    const char ** value_end
    );

void vars_dump_info(const char * info);

#ifdef VARS_DUMP_ALLOW
#  define VARS_DUMP(vars, vars_descr, vars_descr_num, title, ...) \
        do { \
            if(title != NULL) game_cprint(title, ##__VA_ARGS__); \
            vars_dump((vars), (vars_descr), (vars_descr_num)); \
        } while (0)
#  define VARS_INFO_DUMP(info, title, ...) \
        do { \
            if(title) game_cprint((title), ##__VA_ARGS__); \
            vars_dump_info((info)); \
        } while (0)
#else
#  define VARS_DUMP(vars, vars_descr, vars_descr_num, title, ...)
#  define VARS_INFO_DUMP(info, title, ...)
#endif

#endif /* SRC_VARS_H_ */
