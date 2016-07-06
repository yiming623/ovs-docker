/*
 * lnx_msq.c
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */


//#ifdef ARCH_LNX
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "srv_common.h"

typedef struct MSG_BLOCK_
{
    s32   id;
    s32   size;
    void* body;
}MSG_BLOCK;

typedef struct MSG_QUEUE_
{
    s32             size;
    s32             front;
    s32             rear;
    MSG_BLOCK*      pMsg;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
}MSG_QUEUE;


SRV_HANDLE srv_msq_create(s32 queuesize)
{
    MSG_QUEUE* pstQueue;

    pstQueue = (MSG_QUEUE*)malloc(sizeof(MSG_QUEUE));
    if (pstQueue == NULL)
        return SRV_INVALID;

    if (pthread_mutex_init(&(pstQueue->lock),NULL) != 0)
    {
        free(pstQueue);
        return SRV_INVALID;
    }

    if (pthread_cond_init(&(pstQueue->cond),NULL) != 0)
    {
        pthread_mutex_destroy(&(pstQueue->lock));
        free(pstQueue);
        return SRV_INVALID;
    }

    pstQueue->size = queuesize + 1;

    pstQueue->pMsg = (MSG_BLOCK*)malloc(sizeof(MSG_BLOCK)*pstQueue->size);
    if (pstQueue->pMsg == NULL)
    {
        pthread_mutex_destroy(&(pstQueue->lock));
        pthread_cond_destroy(&(pstQueue->cond));
        free(pstQueue);
        return SRV_INVALID;
    }

    pstQueue->front = pstQueue->rear = 0;

    return (SRV_HANDLE)pstQueue;
}

s32 srv_msq_destroy(SRV_HANDLE hMsq)
{
    MSG_QUEUE* pstQueue = (MSG_QUEUE*)hMsq;

    if (hMsq == SRV_INVALID)
        return SRV_ERR;

    pthread_mutex_destroy(&(pstQueue->lock));
    pthread_cond_destroy(&(pstQueue->cond));

    free(pstQueue->pMsg);
    free(pstQueue);

    return SRV_OK;
}

extern FILE* g_fp;

s32 srv_msq_snd(SRV_HANDLE hMsq, s32 prio, s32 msgid, void* pMsg, s32 msgsize)
{
    MSG_QUEUE* pstQueue = (MSG_QUEUE*)hMsq;
    s32        index;

    if (hMsq==SRV_INVALID || pMsg==NULL)
        return SRV_ERR;

    pthread_mutex_lock(&(pstQueue->lock));

    index = (pstQueue->rear+1) % pstQueue->size;

    // see if queue is full. if so, exit.
    if (index == pstQueue->front)
    {
        pthread_mutex_unlock(&(pstQueue->lock));
        return SRV_ERR;
    }

    pstQueue->pMsg[pstQueue->rear].id   = msgid;
    pstQueue->pMsg[pstQueue->rear].body = pMsg;
    pstQueue->pMsg[pstQueue->rear].size = msgsize;

    pstQueue->rear = index;

    pthread_cond_signal(&(pstQueue->cond));

    pthread_mutex_unlock(&(pstQueue->lock));

    return SRV_OK;
}

s32 srv_msq_rcv(SRV_HANDLE hMsq, s32* pMsgid, void** ppMsg)
{
    MSG_QUEUE* pstQueue = (MSG_QUEUE*)hMsq;
    s32        msgsize;
    char str [1000000];
	bzero(str, sizeof(str));

    if (hMsq==SRV_INVALID || pMsgid==NULL || ppMsg==NULL)
        return SRV_ERR;

    pthread_mutex_lock(&(pstQueue->lock));

    // see if queue is empty. if so, exit.
    while (pstQueue->front == pstQueue->rear){
        pthread_cond_wait(&(pstQueue->cond), &(pstQueue->lock));
    }


    pthread_mutex_unlock(&(pstQueue->lock));

    pthread_mutex_lock(&(pstQueue->lock));

    *ppMsg  = pstQueue->pMsg[pstQueue->front].body;
    *pMsgid = pstQueue->pMsg[pstQueue->front].id;
    msgsize = pstQueue->pMsg[pstQueue->front].size;

    pstQueue->front = (pstQueue->front+1) % pstQueue->size;

    pthread_mutex_unlock(&(pstQueue->lock));

    return msgsize;
}

//Add by yxf 20091012 query message number in message queue
s32 srv_msq_num(SRV_HANDLE hMsq)
{
	s32 num;
	MSG_QUEUE* pstQueue = (MSG_QUEUE*)hMsq;
	if(hMsq == SRV_INVALID)
	{
		return SRV_ERR;
	}
	pthread_mutex_lock(&(pstQueue->lock));
	num = (pstQueue->rear + pstQueue->size + 1 - pstQueue->front) % (pstQueue->size + 1);
	pthread_mutex_unlock(&(pstQueue->lock));
	return num;
}

//#endif

