/*
 * srv_list.h
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

#ifndef SRV_LIST_H_
#define SRV_LIST_H_

#include <stdio.h>
#include "srv_common.h"

typedef struct SRV_LIST_NODE_
{
	struct SRV_LIST_NODE_* pNext;
	struct SRV_LIST_NODE_* pPrev;
	void*                  pMySelf;
}SRV_LIST_NODE;

#define SRV_LIST_INIT_ROOT(R) (R)->pMySelf = NULL; \
                              (R)->pNext = (R); \
                              (R)->pPrev = (R)

#define SRV_LIST_INIT_NODE(E, S) (E)->pMySelf = (S); \
                                 (E)->pNext = NULL; \
                                 (E)->pPrev = NULL

#define SRV_LIST_INSERT_AFTER(P, E) (E)->pNext = (P)->pNext; \
	                                (E)->pPrev = (P); \
	                                (E)->pPrev->pNext = (E); \
	                                (E)->pNext->pPrev = (E)

#define SRV_LIST_INSERT_BEFORE(N, E) (E)->pNext = (N); \
	                                 (E)->pPrev = (N)->pPrev; \
	                                 (E)->pPrev->pNext = (E); \
	                                 (E)->pNext->pPrev = (E)

#define SRV_LIST_REMOVE(E) (E)->pPrev->pNext = (E)->pNext; \
	                       (E)->pNext->pPrev = (E)->pPrev; \
	                       (E)->pPrev = NULL; \
	                       (E)->pNext = NULL

#define SRV_LIST_GET_NEXT(E) ((E)->pNext)
#define SRV_LIST_GET_PREV(E) ((E)->pPrev)

#define SRV_LIST_GET_ENTRY(E) ((E)->pMySelf)

#define SRV_LIST_IS_EMPTY(R) ((R)->pNext == (R))

#define SRV_LIST_FOR_EACH(E, R) for ((E)=(R)->pNext; (E)!=(R); (E)=(E)->pNext)

#define SRV_LIST_FOR_EACH_ENTRY(E, R, S) for ((E)=(R)->pNext, (S)=(E)->pMySelf; (E)!=(R); (E)=(E)->pNext)

#endif /* SRV_LIST_H_ */
