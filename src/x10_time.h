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
	//минуты
	int m;
	//секунды
	int s;
	//сотые_доли_секунды
	int hs;
} Tsystime;

void time_Sget();

extern Tsystime time;


#endif /* SRC_X10_TIME_H_ */
