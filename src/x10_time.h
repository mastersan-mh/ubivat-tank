/*
 * x10_time.h
 *
 *  Created on: 11 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_X10_TIME_H_
#define SRC_X10_TIME_H_

typedef struct
{
	//часы
	int h;
	int m;                                                //минуты
	int s;                                                //секунды
	int hs;                                                //сотые_доли_секунды
} Tsystime;

void time_Sget();

extern Tsystime time;


#endif /* SRC_X10_TIME_H_ */
