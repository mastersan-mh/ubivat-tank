/*
 * x10_str.h
 *
 *  Created on: 11 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_X10_STR_H_
#define SRC_X10_STR_H_

char * bytetostr(char bts, char * s);
char * wordtostr(int wts, char * s);
char * inttostr(int its, char * s);
char * longinttostr(long lits, char * s);
char * realtostr(float rts, int width, int decimals, char * s);

char * strZ_addch(char * s0, char ch);
char strZ_UPcheng(char c);
char * strZ_UPeng(char * s);


#endif /* SRC_X10_STR_H_ */
