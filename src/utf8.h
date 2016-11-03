/*
 * utf8.h
 *
 *  Created on: 2 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_UTF8_H_
#define SRC_UTF8_H_


ssize_t utf8len(char * __str8, int * __wchar);
ssize_t utf8nlen(char * __str8, size_t __maxlen, int * __wchar);

int utf8stringloop(
	const char * __string,
	int (*__charhandlecb)(int, uint32_t, int, int, void * ),
	void * __userdata
);
int utf8nstringloop(
	const char * __string,
	size_t __size, int (*__charhandlecb)(int, uint32_t, int, int, void *),
	void * __userdata
);

void char_cp866_to_utf8(char cp866, char * string, int * clen);

size_t strn_cpp866_to_utf8(
	char * __volatile __dest_utf8,
	size_t __dest_size,
	const char * __volatile __src_cp866
);


#endif /* SRC_UTF8_H_ */
