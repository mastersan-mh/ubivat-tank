/*
 * bull.h
 *
 *  Created on: 30 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_BULL_H_
#define SRC_BULL_H_

typedef enum
{
	BULL_ARTILLERY,
	BULL_MISSILE,
	BULL_MINE,
	__BULL_NUM
} bulltype_t;

typedef struct
{
	//повреждение
	int damage;
	//повреждение
	int selfdamage;
	//дальность
	vec_t range;
	//начальная скорость пули
	vec_t speed;
	//bodybox
	vec_t bodybox;
	//изображение оружия
	image_index_t icon;
} bullinfo_t;

/*
 * пуля
 */
typedef struct
{
	//игрок, выпустивший пулю
	struct mobj_s * owner;
	//тип пули
	bulltype_t type;
	//изменение расстояния
	vec_t delta_s;
	//время
	float frame;
} bull_t;

#define BULL(x) ((bull_t *)(x)->data)

extern bullinfo_t bullinfo_table[__BULL_NUM];

void mobj_bull_init();

void bull_draw(camera_t * cam, mobj_t * bull);

#endif /* SRC_BULL_H_ */