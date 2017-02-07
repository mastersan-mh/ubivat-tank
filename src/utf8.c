/*
 * utf8.c
 *
 *  Created on: 2 нояб. 2016 г.
 *      Author: mastersan
 */

#include "utf8.h"

#include <limits.h>
#include <string.h>
#include <fontconfig/fontconfig.h>


static ssize_t __utf8_len(const char * __string, size_t __size, int * __wchar)
{
	int nchar;
	int wchar;
	size_t size;
	if(__size == 0)
	{
		wchar = 0;
		nchar = 0;
	}
	else
	{
		size = __size & INT_MAX;
		if(__size != size) return -1;
		if(!FcUtf8Len((FcChar8 *) __string, size, &nchar, &wchar)) return -1;
	}
	if(__wchar) *__wchar = wchar;
	return nchar;
}


/**
 * @description длина строки utf8
 * @param[in]  __string - Строка utf8.
 * @param[out] __wchar  - Ширина самого широкого символа. Если NULL, тогда не используется.
 * @return Длина строки без NULL. Если меньше 0, ошибка.
 */
ssize_t utf8len(const char * __string, int * __wchar)
{
	size_t size = strlen(__string);
	return __utf8_len(__string, size, __wchar);
}

/**
 * @description длина строки utf8
 * @param[in]  __string - Строка utf8.
 * @param[in]  __size   - Максимальное количество байт в utf8 строке без NULL.
 * @param[out] __wchar  - Ширина самого широкого символа. Если NULL, тогда не используется.
 * @return Длина строки без NULL. Если меньше 0, ошибка.
 */
ssize_t utf8nlen(const char * __string, size_t __size, int * __wchar)
{
	size_t size = strnlen(__string, __size - 1);
	return __utf8_len(__string, size, __wchar);
}

/**
 * @description проход по строке
 */
static int __utf8_stringloop(
	const char * __string,
	size_t __size,
	int (*__charhandlecb)(int, ucs4_t, int, int, void *),
	void * __userdata
)
{
	int size = __size & INT_MAX;
	if(__size != size) return -1;

	int i;
	int lchar;
	int wchar;
	FcChar32 c;

	i = 0;
	while(size)
	{
		lchar = FcUtf8ToUcs4((FcChar8 *) __string, &c, size);
		if (lchar <= 0)	/* malformed UTF8 string */
			return -1;
		__string += lchar;
		size -= lchar;

		if (c >= 0x10000)
			wchar = 4;
		else if (c >= 0x100)
			wchar = 2;
		else
			wchar = 1;

		int ret = __charhandlecb(i, c, lchar, wchar, __userdata);
		if(ret)return -1;
		i++;
	}
	return 0;
}

/**
 * @description проход по строке
 * @param[in] __string - Строка
 * @param[in] void (*callback)(int __i, usc4_t __ucs4, int __lchar, int __wchar, void * __userdata)
 *            Функция обработки символа строки
 *            __i     - номер символа
 *            __ucs4  - символ в формате UCS4
 *            __lchar - длина символа в строке, байт
 *            __wchar - ширина символа, байт
 * @return 0  - Успешно
 *         -1 - Ошибка
 */
int utf8stringloop(
	const char * __string,
	int (*__charhandlecb)(int, ucs4_t, int, int, void *),
	void * __userdata
)
{
	size_t size = strlen(__string);
	return __utf8_stringloop(__string, size, __charhandlecb, __userdata);
}

/**
 * @description проход по строке
 * @param[in] __string - Строка
 * @param[in] void (*callback)(int __i, usc4_t __ucs4, int __lchar, int __wchar, void * __userdata)
 *            Функция обработки символа строки
 *            __i     - номер символа
 *            __ucs4  - символ в формате UCS4
 *            __lchar - ширина символа, байт
 *            __wchar - ширина символа, байт
 * @param[in]  __size   - Максимальное количество байт в utf8 строке.
 * @return 0  - Успешно
 *         -1 - Ошибка
 */
int utf8nstringloop(
	const char * __string,
	size_t __size,
	int (*__charhandlecb)(int, ucs4_t, int, int, void *),
	void * userdata
)
{
	size_t size = strnlen(__string, __size - 1);
	return __utf8_stringloop(__string, size, __charhandlecb, userdata);
}

void char_cp866_to_utf8(char cp866, char * string, int * clen)
{
	typedef struct
	{
		uint64_t utf8;
		int lchar;
	} code_t;
	code_t code_table[128] =
	{
			{ 37072, 2 }, /* А */ /* А 128 */
			{ 37328, 2 }, /* Б */
			{ 37584, 2 }, /* В */
			{ 37840, 2 }, /* Г */
			{ 38096, 2 }, /* Д */
			{ 38352, 2 }, /* Е */ /* Е 133 */
			{ 38608, 2 }, /* Ж */ /* Ж 134 */
			{ 38864, 2 }, /* З */
			{ 39120, 2 }, /* И */
			{ 39376, 2 }, /* Й */
			{ 39632, 2 }, /* К */
			{ 39888, 2 }, /* Л */
			{ 40144, 2 }, /* М */
			{ 40400, 2 }, /* Н */
			{ 40656, 2 }, /* О */
			{ 40912, 2 }, /* П */
			{ 41168, 2 }, /* Р */
			{ 41424, 2 }, /* С */
			{ 41680, 2 }, /* Т */
			{ 41936, 2 }, /* У */
			{ 42192, 2 }, /* Ф */
			{ 42448, 2 }, /* Х */
			{ 42704, 2 }, /* Ц */
			{ 42960, 2 }, /* Ч */
			{ 43216, 2 }, /* Ш */
			{ 43472, 2 }, /* Щ */
			{ 43728, 2 }, /* Ъ */
			{ 43984, 2 }, /* Ы */
			{ 44240, 2 }, /* Ь */
			{ 44496, 2 }, /* Э */
			{ 44752, 2 }, /* Ю */
			{ 45008, 2 }, /* Я */ /* Я 159 */
			{ 45264, 2 }, /* а */ /* А 160 */
			{ 45520, 2 }, /* б */
			{ 45776, 2 }, /* в */
			{ 46032, 2 }, /* г */
			{ 46288, 2 }, /* д */
			{ 46544, 2 }, /* е */ /* е 165 */
			{ 46800, 2 }, /* ж */ /* ж 166 */
			{ 47056, 2 }, /* з */
			{ 47312, 2 }, /* и */
			{ 47568, 2 }, /* й */
			{ 47824, 2 }, /* к */
			{ 48080, 2 }, /* л */
			{ 48336, 2 }, /* м */
			{ 48592, 2 }, /* н */
			{ 48848, 2 }, /* о */
			{ 49104, 2 }, /* п */ /* п 175 */
			/* GAP1 */
			{ 32977, 2 }, /* р */ /* р 224 */
			{ 33233, 2 }, /* с */
			{ 33489, 2 }, /* т */
			{ 33745, 2 }, /* у */
			{ 34001, 2 }, /* ф */
			{ 34257, 2 }, /* х */
			{ 34513, 2 }, /* ц */
			{ 34769, 2 }, /* ч */
			{ 35025, 2 }, /* ш */
			{ 35281, 2 }, /* щ */
			{ 35537, 2 }, /* ъ */
			{ 35793, 2 }, /* ы */
			{ 36049, 2 }, /* ь */
			{ 36305, 2 }, /* э */
			{ 36561, 2 }, /* ю */
			{ 36817, 2 }, /* я */ /* я 239 */
			{ 33232, 2 }, /* Ё */ /* Ё 240 */
			{ 37329, 2 }  /* ё */ /* ё 241 */
			/* GAP2 */

	};
#define __CP866 ((unsigned char)cp866)
#define GAP1START (176 - 128)
#define GAP1END   (224 - 128)
#define GAP1LEN   (GAP1END - GAP1START)
#define GAP2START (242 - 128)
#define GAP2END   (256 - 128)
	if(__CP866 < 128)
	{
		*string = cp866;
		*clen = 1;
		return;
	}

	code_t * code;

	cp866 -= 128;
	if(__CP866 < GAP1START)
	{
		code = &code_table[__CP866];
		//FcUcs4ToUtf8 (code->utf8, (FcChar8 *) string);
		memcpy(string, &code->utf8, code->lchar);
		*clen = code->lchar;
		return;
	}
	if(__CP866 < GAP1END)
	{
		*string = 32; /* space */
		*clen = 1;
	}
	if(__CP866 < GAP2START)
	{
		code = &code_table[__CP866 - GAP1LEN];
		//FcUcs4ToUtf8 (code->utf8, (FcChar8 *) string);
		memcpy(string, &code->utf8, code->lchar);
		*clen = code->lchar;
		return;
	}
	/* __CP866 < GAP2END */
	*string = 32; /* space */
	*clen = 1;

}

/**
 * @description строку cp866 -> utf8
 * @return Количество скопированных байт в строке utf8 без NULL-символа
 */
size_t strn_cpp866_to_utf8(
	char * __volatile __dest_utf8,
	size_t __dest_size,
	const char * __volatile __src_cp866
)
{
	uint64_t tmp;
	size_t len = 0;
	int clen;
	if(__dest_size == 0)return 0;
	__dest_size--; /* резервируем место для символа 0 */
	while(*__src_cp866)
	{
		char_cp866_to_utf8(*__src_cp866, (char * )&tmp, &clen);
		if(len + clen > __dest_size)break;
		memcpy(__dest_utf8, &tmp, clen);
		__src_cp866++;
		__dest_utf8 += clen;
		len += clen;
	}
	*__dest_utf8 = 0;
	return len;
}
