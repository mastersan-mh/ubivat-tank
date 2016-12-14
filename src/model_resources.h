/*
 * model_resources.h
 *
 *  Created on: 14 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_MODEL_RESOURCES_H_
#define SRC_MODEL_RESOURCES_H_


#define BULL_FPS (8)
#define BULL_MISSILE_MODEL_FRAMES (2)
#define BULL_MINE_MODEL_FRAMES (2)

//скорость проигрывания кадров взрыва
#define EXPLODE_FPS   (28)
//количество кадров взрыва
#define EXPLODE_FRAMES_NUM (8)

#define TANK_FPS (20)
#define TANK_FRAMES_NUM (4)

void model_resources_register();

#endif /* SRC_MODEL_RESOURCES_H_ */
