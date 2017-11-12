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

/**
 * @brief Очистка памяти переменных
 */
void vars_free(void * vars, const var_descr_t * vars_descr, size_t vars_descr_num)
{
    vars_dump(vars, vars_descr, vars_descr_num, "==== vars_free():");

    for(size_t i = 0; i < vars_descr_num; i++)
    {
        const var_descr_t * var_descr = &vars_descr[i];
        if(var_descr->type == VARTYPE_STRING)
        {
            char * str;
            memcpy(&str, vars + var_descr->ofs, var_descr->size);
            Z_free(str);
        }
    }
}

/**
 * @brief Вычислить размер буфера для сохранения переменых в двоичном виде
 */
size_t var_buffersize_calculate(const var_descr_t * vars_descr, size_t vars_descr_num)
{
    size_t bufsize = 0;
    for(size_t i = 0; i < vars_descr_num; i++)
    {
        bufsize += vars_descr[i].size;
    }
    return bufsize;
}


void vars_dump(void * vars, const var_descr_t * vars_descr, size_t vars_descr_num, const char * title)
{
    if(title)
    {
        game_console_send(title);
    }

    static const char * list[] =
    {
            "BOOL",
            "INTEGER",
            "FLOAT",
            "VECTOR1",
            "VECTOR2",
            "DIRECTION",
            "STRING",
    };

    size_t i;
    for( i = 0; i < vars_descr_num; i++)
    {
        char buf[64];
        const var_descr_t * var_descr = &vars_descr[i];

        char * value = (char*)vars + var_descr->ofs;

#define MIN(a, b) ((a)<(b) ? (a) : (b))

        switch(var_descr->type)
        {
            case VARTYPE_BOOL:
                strcpy(buf, value != 0 ? "TRUE" : "FALSE");
                break;
            case VARTYPE_INTEGER:
            {
                INTEGER v;
                memcpy(&v, value, MIN(var_descr->size, sizeof(INTEGER)));
                sprintf(buf, "%lld", (long long)v);
                break;
            }
            case VARTYPE_FLOAT:
            {
                FLOAT v;
                memcpy(&v, value, MIN(var_descr->size, sizeof(FLOAT)));
                sprintf(buf, "%lf", (double)v);
                break;
            }
            case VARTYPE_VECTOR1:
            {
                VECTOR1 v;
                memcpy(&v, value, MIN(var_descr->size, sizeof(VECTOR1)));
                sprintf(buf, "%lf", (double)v);
                break;
            }
            case VARTYPE_VECTOR2:
            {
                VECTOR2 v;
                memcpy(&v, value, MIN(var_descr->size, sizeof(VECTOR2)));
                sprintf(buf, "%lf %lf", v[0], v[1]);
                break;
            }
            case VARTYPE_DIRECTION:
            {
                DIRECTION v;
                memcpy(&v, value, MIN(var_descr->size, sizeof(DIRECTION)));
                switch(v)
                {
                    case DIR_UP   : strcpy(buf, "UP"   ); break;
                    case DIR_DOWN : strcpy(buf, "DOWN" ); break;
                    case DIR_LEFT : strcpy(buf, "LEFT" ); break;
                    case DIR_RIGHT: strcpy(buf, "RIGHT"); break;
                    default: strcpy(buf, "???");
                }
                break;
            }
            case VARTYPE_STRING:
            {
                char * str;
                memcpy(&str, value, MIN(var_descr->size, sizeof(STRING)));
                strncpy(buf, str, 64);
                break;
            }
        }
        game_console_send("  var dump: (%s)%s = %s", list[var_descr->type], var_descr->name, buf);
    }

}



