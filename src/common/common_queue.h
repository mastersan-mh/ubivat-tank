/*
 * common_queue.h
 *
 *  Created on: 29 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_COMMON_SYS_COMMON_QUEUE_H_
#define SRC_COMMON_SYS_COMMON_QUEUE_H_

#include <sys/queue.h>

#ifdef CIRCLEQ_NULL
#  error CIRCLEQ_NULL already defined!
#endif
#define CIRCLEQ_NULL(head) ((void *)(head))

#ifdef CIRCLEQ_END
#  error CIRCLEQ_END already defined!
#endif
#define CIRCLEQ_END(elm, head) ((elm) == CIRCLEQ_NULL(head))


#endif /* SRC_COMMON_SYS_COMMON_QUEUE_H_ */
