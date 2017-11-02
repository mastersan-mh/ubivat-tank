/*
 * g_conf.h
 *
 * game configuration file
 *
 *  Created on: 4 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_G_CONF_H_
#define SRC_G_CONF_H_

typedef struct
{
	/*
	кому принадлежит привязка клавиши:
	0 - игрок 1
	1 - игрок 2
	...
	 */
	int playerId;
	int key;
	char * action;
} keybind_t;

extern void gconf_newbinds();
extern int gconf_key_get(int player, const char * action);
extern int gconf_save();
extern int gconf_load();

#endif /* SRC_G_CONF_H_ */
