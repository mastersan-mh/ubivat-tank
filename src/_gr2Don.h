/*
 * _gr2Don.h
 *
 * Created on: 11 окт. 2016 г.
 * Author: mastersan
 */

#ifndef SRC__GR2DON_H_
#define SRC__GR2DON_H_

enum text_orient_e
{
	orient_horiz,
	orient_vert
};

void gr2Don_setchar(int px, int py, char ch);
void gr2Don_settext(int px, int py, enum text_orient_e orientation, char * s);

#endif /* SRC__GR2DON_H_ */
