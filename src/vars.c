/*
 * vars.c
 *
 *  Created on: 3 февр. 2017 г.
 *      Author: mastersan
 */

#include "vars.h"
#include "Z_mem.h"
#include "common/common_hash.h"
#include "game.h"

#include "progs/helpers.h"

#include <stddef.h>
#include <string.h>

const var_descr_t * var_find(const var_descr_t * vars_descr, size_t vars_descr_num, const char * name)
{
    for(size_t i = 0; i < vars_descr_num; i++)
    {
        const var_descr_t * var_descr = &vars_descr[i];
        if(strncmp(var_descr->name, name, VARNAME_SIZE) == 0)
            return var_descr;
    }
    return NULL;
}

int var_set(
    void * vars,
    const var_descr_t * vars_descr,
    size_t vars_descr_num,
    const char * key,
    const char * value
)
{
    const var_descr_t * vd = var_find(vars_descr, vars_descr_num, key);
    if(!vd)
    {
        game_cprint("Error: On var set variable \"%s\" not found.", key);
        return -1;
    }

    VECTOR2 vvector2;
    void * ofs = vars + vd->ofs;
    switch(vd->type)
    {
        case VARTYPE_BOOL:
            *(BOOL*)ofs = (strcmp(value, "true") == 0 || atoi(value) != 0);
            break;
        case VARTYPE_INTEGER:
            *(INTEGER*)ofs = atol(value);
            break;
        case VARTYPE_FLOAT:
            *(FLOAT*)ofs = atof(value);
            break;
        case VARTYPE_VECTOR2:
            sscanf(value, "%f %f", &vvector2[0], &vvector2[1]);
            ((vec_t*)ofs)[0] = vvector2[0];
            ((vec_t*)ofs)[1] = vvector2[1];
            break;
        case VARTYPE_DIRECTION:
            if(strcmp("0", value) == 0 || strcmp("up", value) == 0 || strcmp("north", value) == 0)
                *(DIRECTION*)ofs = DIR_UP;
            else if(strcmp("1", value) == 0 || strcmp("down", value) == 0 || strcmp("south", value) == 0)
                *(DIRECTION*)ofs = DIR_DOWN;
            else if(strcmp("2", value) == 0 || strcmp("left", value) == 0 || strcmp("west", value) == 0)
                *(DIRECTION*)ofs = DIR_LEFT;
            else if(strcmp("3", value) == 0 || strcmp("right", value) == 0 || strcmp("east", value) == 0)
                *(DIRECTION*)ofs = DIR_RIGHT;
            else
                *(DIRECTION*)ofs = DIR_UP;
            break;
        case VARTYPE_STRING:
            *(STRING*)ofs = Z_strdup(value);
            break;
    }
    return 0;
}

/**
 * @brief Очистка памяти переменных
 */
void vars_free(void * vars, const var_descr_t * vars_descr, size_t vars_descr_num)
{
    for(size_t i = 0; i < vars_descr_num; i++)
    {
        const var_descr_t * var_descr = &vars_descr[i];
        if(var_descr->type == VARTYPE_STRING)
        {
            char * str = *(STRING*)(vars + var_descr->ofs);
            Z_free(str);
        }
    }
}

/**
 * @brief Вычислить размер буфера для сохранения переменых в двоичном виде
 */
size_t var_buffersize_calculate(const var_descr_t * vars_descr, size_t vars_descr_num)
{
    size_t max_offset = 0;
    size_t max_offset_size = 0;
    for(size_t i = 0; i < vars_descr_num; i++)
    {
        const var_descr_t * var_descr = &vars_descr[i];
        /* считаем так, потому что данные в структуре не упакованы*/
        if(var_descr->ofs > max_offset)
        {
            max_offset = var_descr->ofs;
            max_offset_size = vars_descr[i].size;
        }
    }
    return max_offset + max_offset_size;
}

bool vars_descr_eq(const var_descr_t * vds1, const var_descr_t * vds2, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        const var_descr_t * vd1 = &vds1[i];
        const var_descr_t * vd2 = &vds2[i];
        if(
                vd1->type != vd2->type ||
                vd1->ofs  != vd2->ofs  ||
                vd1->size != vd2->size ||
                strncmp(vd1->name, vd2->name, VARNAME_SIZE) != 0
        )
            return false;

    }
    return true;
}

void vars_dump(void * vars, const var_descr_t * vars_descr, size_t vars_descr_num)
{
    static const char * list[] =
    {
            "BOOL",
            "INTEGER",
            "FLOAT",
            "VECTOR2",
            "DIRECTION",
            "STRING",
    };

    size_t i;
    for( i = 0; i < vars_descr_num; i++)
    {
        char buf[64];
        const var_descr_t * var_descr = &vars_descr[i];

        void * ofs = vars + var_descr->ofs;

        switch(var_descr->type)
        {
            case VARTYPE_BOOL:
                strcpy(buf, (*(BOOL*)ofs) == false ? "false" : "true");
                break;
            case VARTYPE_INTEGER:
                sprintf(buf, "%lld", (long long)(*(INTEGER*)ofs));
                break;
            case VARTYPE_FLOAT:
                sprintf(buf, "%f", (float)(*(FLOAT*)ofs));
                break;
            case VARTYPE_VECTOR2:
                sprintf(buf, "%lf %lf", (*(VECTOR2*)ofs)[0], (*(VECTOR2*)ofs)[1]);
                break;
            case VARTYPE_DIRECTION:
                switch(*(DIRECTION*)ofs)
                {
                    case DIR_UP   : strcpy(buf, "up"   ); break;
                    case DIR_DOWN : strcpy(buf, "down" ); break;
                    case DIR_LEFT : strcpy(buf, "left" ); break;
                    case DIR_RIGHT: strcpy(buf, "right"); break;
                    default: strcpy(buf, "???");
                }
                break;
            case VARTYPE_STRING:
                strncpy(buf, *(STRING*)ofs, 64);
                break;
        }
        game_cprint("  var dump binary: (%s)%s = %s", list[var_descr->type], var_descr->name, buf);
    }

}

#define CHAR_IS(xcharacter, xstr) \
    (strchr((xstr), (xcharacter)) != NULL)

static bool char_is_delimiter(char c)
{
    return CHAR_IS(c, " \t\v\n\r");
}


static void str_skip_delimiters(const char ** str)
{
    while((**str) && char_is_delimiter(**str))
        (*str)++;
}


bool infovars_get_next_key(
    const char ** info,
    const char ** key_begin,
    const char ** key_end,
    const char ** value_begin,
    const char ** value_end
    )
{
#define PARSEERROR(format, ...) \
    do { \
        game_cprint("Parse error: "format, ##__VA_ARGS__); \
        return false; \
    }while (0)

    *key_begin = NULL;
    *key_end = NULL;
    *value_begin = NULL;
    *value_end = NULL;

    if(info == NULL)
        PARSEERROR("Context is NULL.");
    if(*info == NULL)
        PARSEERROR("Context is NULL.");

#define CTX (*info)

    /* key parse */
    str_skip_delimiters(info);
    *key_begin = CTX;
    if(!**key_begin)
        return false; /* return with no error, normal end */

#define CHAR_IS_CTRL(xcharacter) CHAR_IS(xcharacter, ":\"{},")

    while(*CTX && !char_is_delimiter(*CTX) && !CHAR_IS_CTRL(*CTX))
            CTX++;
    *key_end = CTX;

    while(*CTX && char_is_delimiter(*CTX))
            CTX++;
    if(*CTX != ':')
        PARSEERROR("':' expected.");

    /* value parse */
    CTX++;

    str_skip_delimiters(info);

    char expected = '\0';
    bool expectedok = false;

    switch(*CTX)
    {
        case '"': expected = '"'; CTX++; break;
        case '{': expected = '}'; CTX++; break;
        case ':':
        case '}':
        case ',': PARSEERROR("Unexpected '%c'.", *CTX);
        case '\0': PARSEERROR("Unexpected end.");
    }

    *value_begin = CTX;
    while(*CTX &&
            !(expectedok = (
                    (expected == '\0' && *CTX == ',') ||
                    (expected != '\0' && expected == *CTX))
            )
    )
        CTX++;

    *value_end = CTX;

    if(expected != '\0')
        CTX++;

    if(!expectedok)
    {
        if(expected == '\0')
        {
            if(CTX == *value_begin)
                PARSEERROR("Unexpected end");
        }
        else
            PARSEERROR("Expected '%c'.", expected);
    }

    str_skip_delimiters(info);
    if(*CTX == ',')
        CTX++;

    return true;
}


void vars_dump_info(const char * info)
{
    const char * key_begin;
    const char * key_end;
    const char * value_begin;
    const char * value_end;
    size_t key_size;
    size_t value_size;
    size_t key_buf_size;
    size_t value_buf_size;
    while(infovars_get_next_key(
        &info,
        &key_begin,
        &key_end,
        &value_begin,
        &value_end))
    {
#define KEYSIZE 32
#define VALUESIZE 32
        static char key[KEYSIZE];
        static char value[VALUESIZE];

        key_size = key_end - key_begin;
        key_buf_size = MIN(key_size + 1, KEYSIZE);
        strlcpy(key, key_begin, key_buf_size);

        value_size = value_end - value_begin;
        value_buf_size = MIN(value_size + 1, VALUESIZE);
        strlcpy(value, value_begin, value_buf_size);

#define STRBUFSIZE 32
char str[STRBUFSIZE];

        game_cprint("  var dump info: %s%s = \"%s%s\"",
            key,
            key_size > key_buf_size ? s_snprintf(str, STRBUFSIZE, "...[%ld]", (long)(key_size - key_buf_size)) : "",
            value,
            value_size > value_buf_size ? s_snprintf(str, STRBUFSIZE, "...[%ld]", (long)(value_size - value_buf_size)) : ""
            );
    }
}


