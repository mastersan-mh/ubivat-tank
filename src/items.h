/*
 * items.h
 *
 *  Created on: 3 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ITEMS_H_
#define SRC_ITEMS_H_

#include "types.h"
#include "mobjs.h"

// предмет не используется
#define ITEM_AMOUNT_NA  (0)
#define ITEM_AMOUNT_INF (-1)

//сколько очков нужно набрать для повышения уровня танка
#define ITEM_SCOREPERCLASS 200

typedef enum
{
	ITEM_SCORES,
	ITEM_HEALTH,
	ITEM_ARMOR,
	ITEM_AMMO_ARTILLERY,
	ITEM_AMMO_MISSILE,
	ITEM_AMMO_MINE,
	__ITEM_NUM
}itemtype_t;

/*
 * предметы
 */
typedef struct
{
	itemtype_t type;
	// количество
	int amount;
	// флаг присутствия
	bool exist;
} item_t;

#define ENT_ITEM(mobj) ((item_t *) (mobj)->data)

itemtype_t items_mobjtype_to_itemtype(mobj_type_t mobjtype);

void mobj_items_init();

#endif /* SRC_ITEMS_H_ */
