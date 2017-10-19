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

#include <string.h>

/*
static vardata_t defaultvardata_int = {
		.name = "(null)",
		.type = VARTYPE_INTEGER,
		.value = {.i64 = 0}
};
static vardata_t defaultvardata_float = {
		.name = "(null)",
		.type = VARTYPE_FLOAT,
		.value = {.f = 0.0f}
};
static vardata_t defaultvardata_string = {
		.name = "(null)",
		.type = VARTYPE_STRING,
		.value = {.string = ""}
};
*/

var_t * var_create(var_t ** root, const char *varname, vartype_t vartype)
{
	vardata_t * vardata = Z_malloc(sizeof(vardata_t));
	strncpy(vardata->name, varname, VARNAME_SIZE);
	vardata->type = vartype;
	switch(vardata->type)
	{
		case VARTYPE_INTEGER: vardata->value.i64 = 0; break;
		case VARTYPE_FLOAT  : vardata->value.f   = 0.0f; break;
		case VARTYPE_STRING : vardata->value.string = NULL; break;
	}
	return (var_t *)tree_node_insert(root, HASH32(varname), vardata);
}

var_t * var_find(const var_t * root, const char * varname)
{
	return (var_t *)tree_node_find(root, HASH32(varname));
}

static void vardata_delete(void * data)
{
	vardata_t * vardata = data;
	switch(vardata->type)
	{
		case VARTYPE_INTEGER: vardata->value.i64 = 0; break;
		case VARTYPE_FLOAT  : vardata->value.f   = 0.0f; break;
		case VARTYPE_STRING   :
			VARIABLE_STRING_ERASE(vardata->value.string);
			break;
	}
	Z_free(data);
}

void vars_delete(var_t ** root)
{
	tree_delete((*root), vardata_delete);
	(*root) = NULL;
}

void vars_foreach(var_t * root, void (*action_cb)(vardata_t * vardata, void * args), void * args)
{
	tree_foreach(root, (void (*)(void *, void *)) action_cb, args);
}

void vars_dump(var_t * root, const char * title)
{
	if(title)
	{
		game_console_send(title);
	}

	static const char * list[] =
	{
			"INTEGER",
			"FLOAT",
			"STRING",
	};

	void var_dump(vardata_t * vardata, void * args)
	{
		switch(vardata->type)
		{
			case VARTYPE_INTEGER:
				game_console_send("  var dump: (%s)%s = %ld",
					list[vardata->type], vardata->name, (long)vardata->value.i64);
				break;
			case VARTYPE_FLOAT:
				game_console_send("  var dump: (%s)%s = %f",
					list[vardata->type], vardata->name, vardata->value.f);
				break;
			case VARTYPE_STRING:
				game_console_send("  var dump: (%s)%s = %s",
					list[vardata->type], vardata->name, vardata->value.string);
				break;
		}
	}

	vars_foreach(root, var_dump, NULL);
}



