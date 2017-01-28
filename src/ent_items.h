/*
 * ent_items.h
 *
 *  Created on: 3 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_ITEMS_H_
#define SRC_ENT_ITEMS_H_

#include "types.h"
#include "entity.h"

/* предмет не используется */
#define ITEM_AMOUNT_NA  (0)
/* бесконечно */
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
	ITEM_NUM
} itemtype_t;

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

#define ENT_ITEM(entity) ((item_t *) (entity)->data)

itemtype_t items_enttype_to_itemtype(const char * entname);

extern void entity_items_init(void);

#endif /* SRC_ENT_ITEMS_H_ */
