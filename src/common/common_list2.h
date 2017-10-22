/*
 * helper_list2.h
 *
 *  Created on: 9 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_COMMON_LIST2_H_
#define SRC_COMMON_LIST2_H_

/* добавить элемент в начало списка (не в цикле!) */
#define LIST2_PUSH(list, ent) \
		do \
		{  \
			(ent)->prev = NULL;   \
			(ent)->next = (list); \
			if( (list) != NULL )  \
				(list)->prev = (ent); \
			(list) = (ent);       \
		} while(0)

/* исключение элемента из списка (не в цикле!) */
#define LIST2_UNLINK(list, ent) \
		do \
		{  \
			if((ent) == (list))        \
				(list) = (list)->next; \
			if((ent)->prev)            \
				(ent)->prev->next = (ent)->next; \
			if((ent)->next)            \
				(ent)->next->prev = (ent)->prev; \
		} while(0)

/* добавить элемент в начало списка (не в цикле!) */
#define LIST2_IS_EMPTY(list) \
		(list == NULL)


/* проход по элементам без удаления элементов */
#define LIST2_FOREACH(list, ent) \
	for((ent) = (list); (ent) != NULL; (ent) = (ent)->next)

/* проход по элементам без удаления элементов в обратном направлении*/
#define LIST2_FOREACHR(list, ent) \
	for( (ent) = (list); (ent) != NULL && (ent)->next != NULL ; (ent) = (ent)->next ) ; \
	for( ; (ent) != NULL; (ent) = (ent)->prev )

#define LIST2_FOREACH_I(list, ent, i) \
	for((i) = 0, (ent) = (list); (ent) != NULL; (ent) = (ent)->next, (i)++)

/* получить элемент ent с номером itarget*/
#define LIST2_LIST_TO_IENT(list, ent, i, itarget) \
		for((ent) = (list), (i) = 0; ent != NULL && (i) < (itarget); (ent) = (ent)->next, (i)++)


/* проход по элементам с возможностью исключения элементов */
#define LIST2_FOREACHM(list, ent, erased) \
		for((ent) = (list), (erased) = NULL; (ent) != NULL; (erased) = NULL)

/* переход к следующему элементу в списке для цикла с возможностью удаления */
#define LIST2_FOREACHM_NEXT(ent, erased) \
		if(!(erased) && (ent)) \
			(ent) = (ent)->next;

/* извлечение элемнта из списка во время прохождения в цикле */
#define LIST2_FOREACHM_EXCLUDE(list, ent, erased) \
		do \
		{  \
			(erased) = (ent); \
			(ent) = (ent)->next; \
			if((erased) == (list)) \
				(list) = (list)->next; \
			if((erased)->prev) \
				(erased)->prev->next = (erased)->next; \
			if((erased)->next) \
				(erased)->next->prev = (erased)->prev; \
		} while(0)

#endif /* SRC_COMMON_LIST2_H_ */
