/*
 * srv_buffer.c
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

#include <stdlib.h>
#include "srv_buffer.h"
#include "srv_mutex.h"
#include "srv_list.h"
#include "srv_bitmap.h"

typedef struct BUF_HEAD_
{
    SRV_LIST_NODE link;
    s32           poolid;
}BUF_HEAD;

typedef struct BUF_POOL_
{
    SRV_LIST_NODE root;
    s32           blocksize;
    s32           blocknr;
    u8*           pBufAddr;
    SRV_HANDLE    hMutex;
    SRV_HANDLE    bitmapId;//Add by yxf
}BUF_POOL;


static BUF_POOL* g_pstBufPool;
static s32       g_nPoolSize;


s32 srv_buf_init(const SRV_BUF_POOL* pstBufPool, s32 poolsize)
{
    s32       i, j, realsize;
    u8*       tmp;
    BUF_HEAD* head;

    if (pstBufPool==NULL || poolsize<=0)
        return SRV_ERR;

    g_pstBufPool = (BUF_POOL*)malloc(sizeof(BUF_POOL)*poolsize);
    if (g_pstBufPool == NULL)
        return SRV_ERR;

    for (i=0; i<poolsize; i++)
    {
        // attach a head for each block
        realsize = pstBufPool[i].blocksize + sizeof(BUF_HEAD);

        g_pstBufPool[i].pBufAddr = (u8*)malloc(pstBufPool[i].blocknr*realsize);
        if (g_pstBufPool[i].pBufAddr == NULL)
            return SRV_ERR;

        // link all blocks
        SRV_LIST_INIT_ROOT(&(g_pstBufPool[i].root));
        for (j=0; j<pstBufPool[i].blocknr; j++)
        {
            tmp = g_pstBufPool[i].pBufAddr + j * realsize;

            head = (BUF_HEAD*)tmp;
            head->poolid = i;

            SRV_LIST_INIT_NODE(&(head->link), tmp);
            SRV_LIST_INSERT_BEFORE(&(g_pstBufPool[i].root), &(head->link));
        }

        g_pstBufPool[i].blocksize = pstBufPool[i].blocksize;
        g_pstBufPool[i].blocknr   = pstBufPool[i].blocknr;

        //Init the bitmap
		g_pstBufPool[i].bitmapId = srv_bmp_create(g_pstBufPool[i].blocknr);
		if(g_pstBufPool[i].bitmapId == SRV_INVALID)
		{
			//need to free the buffer by malloc
			return SRV_ERR;
		}

        // create mutex
        g_pstBufPool[i].hMutex = srv_mutex_create();
        if (g_pstBufPool[i].hMutex == SRV_INVALID)
        	return SRV_ERR;
    }

    g_nPoolSize = poolsize;

    return SRV_OK;
}

void* srv_buf_alloc(s32 size)
{
    s32            i;
    SRV_LIST_NODE* link;
    u8*            buf;


    // I think you can use binary search to improve efficiency
    for (i=0; i<g_nPoolSize; i++)
    {
        srv_mutex_lock(g_pstBufPool[i].hMutex);

        if (size<=g_pstBufPool[i].blocksize && !SRV_LIST_IS_EMPTY(&(g_pstBufPool[i].root)))
        {
            link = SRV_LIST_GET_NEXT(&(g_pstBufPool[i].root));
            buf = SRV_LIST_GET_ENTRY(link);

            SRV_LIST_REMOVE(link);


            //Add bitmap operation by yxf
            srv_bmp_set_bit(g_pstBufPool[i].bitmapId,(buf - g_pstBufPool[i].pBufAddr) / (sizeof(BUF_HEAD) + g_pstBufPool[i].blocksize),1);

            srv_mutex_unlock(g_pstBufPool[i].hMutex);

            return (void*)(buf+sizeof(BUF_HEAD));
        }

        srv_mutex_unlock(g_pstBufPool[i].hMutex);
    }

    return NULL;
}

void srv_buf_free(void* buf)
{
    BUF_HEAD* pBufHead;
    s32       poolid;

    if (buf == NULL)
        return;

    pBufHead = (BUF_HEAD*)(buf-sizeof(BUF_HEAD));
    poolid   = pBufHead->poolid;

    if (poolid >= g_nPoolSize)
        return;

    //Add bitmap operation by yxf
    if(0 == srv_bmp_get_bit(g_pstBufPool[poolid].bitmapId,((u8 *)pBufHead - g_pstBufPool[poolid].pBufAddr) / (sizeof(BUF_HEAD) + g_pstBufPool[poolid].blocksize)))
    {
    	printf("Buf has been freed.\r\n");
    	return;
    }

    srv_mutex_lock(g_pstBufPool[poolid].hMutex);
    SRV_LIST_INSERT_BEFORE(&(g_pstBufPool[poolid].root), &(pBufHead->link));
    //Add bitmap operation by yxf
    srv_bmp_set_bit(g_pstBufPool[poolid].bitmapId,((u8 *)pBufHead - g_pstBufPool[poolid].pBufAddr) / (sizeof(BUF_HEAD) + g_pstBufPool[poolid].blocksize),0);
    srv_mutex_unlock(g_pstBufPool[poolid].hMutex);
}

