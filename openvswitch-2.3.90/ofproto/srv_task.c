/*
 * srv_task.c
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

#include <stdio.h>
#include "srv_task.h"
#include "srv_buffer.h"

#define MAX_MSQ_SIZE 500

typedef struct TASK_BLOCK_
{
    s32        tid;
    char*      pName;
    void*      (*fpEntry)(void*);
    void*      arg;
    s32        prio;
    s32        isMsqEnable;

    SRV_HANDLE hTask;
    SRV_HANDLE hMsq;
}TASK_BLOCK;

typedef struct MSG_HDR_
{
    s32 stid;
    s32 dtid;
    s32 bodysize;
    s32 reserve;
}MSG_HDR;

extern SRV_HANDLE srv_task_create(SRV_TASK* pstTask);
extern SRV_HANDLE srv_msq_create(s32 queuesize);
extern s32 srv_msq_destroy(SRV_HANDLE hMsq);
extern s32 srv_msq_snd(SRV_HANDLE hMsq, s32 prio, s32 msgid, void* pMsg, s32 msgsize);
extern s32 srv_msq_rcv(SRV_HANDLE hMsq, s32* pMsgid, void** ppMsg);
extern s32 srv_msq_num(SRV_HANDLE hMsq);

static TASK_BLOCK* g_pstTaskTbl = NULL;
static s32         g_nMaxTblSize = 0;


s32 srv_task_init(s32 maxtablesize)
{
	s32 i;

    // see if the task table has already exist. if so, exit.
    if (g_pstTaskTbl != NULL)
    	return SRV_ERR;

    g_pstTaskTbl = (TASK_BLOCK*)srv_buf_alloc(sizeof(TASK_BLOCK)*maxtablesize);
    if (g_pstTaskTbl == NULL)
    	return SRV_ERR;

    // initialize the task table
    for (i=0; i<maxtablesize; i++)
    {
    	g_pstTaskTbl[i].tid   = SRV_INVALID;
    	g_pstTaskTbl[i].hTask = SRV_INVALID;
    	g_pstTaskTbl[i].hMsq  = SRV_INVALID;
    }

    g_nMaxTblSize = maxtablesize;

	return SRV_OK;
}

s32 srv_task_register_table(const SRV_TASK* pstTaskTbl, s32 tblsize)
{
    s32 i;

    if (pstTaskTbl==NULL || tblsize<=0)
        return SRV_ERR;

    for (i=0; i<tblsize; i++)
    {
        if (srv_task_register(&pstTaskTbl[i]) != SRV_OK)
            return SRV_ERR;
    }

    return SRV_OK;
}

s32 srv_task_start_table()
{
    s32 tid;

    if (g_pstTaskTbl == NULL)
        return SRV_ERR;

    for (tid=0; tid<g_nMaxTblSize; tid++)
        srv_task_start(tid);

    return SRV_OK;
}

s32 srv_task_register(const SRV_TASK* pstTask)
{
    if (pstTask==NULL || pstTask->tid>=g_nMaxTblSize || pstTask->fpEntry==NULL)
        return SRV_ERR;

    // see if the thread index has been used. if so, exit.
    if (g_pstTaskTbl[pstTask->tid].tid != SRV_INVALID)
        return SRV_ERR;

    g_pstTaskTbl[pstTask->tid].tid         = pstTask->tid;
    g_pstTaskTbl[pstTask->tid].pName       = pstTask->pName;
    g_pstTaskTbl[pstTask->tid].fpEntry     = pstTask->fpEntry;
    g_pstTaskTbl[pstTask->tid].arg         = pstTask->arg;
    g_pstTaskTbl[pstTask->tid].prio        = pstTask->prio;
    g_pstTaskTbl[pstTask->tid].isMsqEnable = pstTask->isMsqEnable;

    // see if user need a message queue. if so, create it.
    if (pstTask->isMsqEnable)
    {
    	g_pstTaskTbl[pstTask->tid].hMsq = srv_msq_create(MAX_MSQ_SIZE);
        if (g_pstTaskTbl[pstTask->tid].hMsq == SRV_INVALID)
        {
        	g_pstTaskTbl[pstTask->tid].tid = SRV_INVALID;
            return SRV_ERR;
        }
    }

    return SRV_OK;
}

s32 srv_task_start(s32 tid)
{
    if (tid>=g_nMaxTblSize || g_pstTaskTbl[tid].tid==SRV_INVALID)
        return SRV_ERR;

    // see if the task has already started. if so, exit.
    if (g_pstTaskTbl[tid].hTask != SRV_INVALID)
        return SRV_ERR;

    g_pstTaskTbl[tid].hTask = srv_task_create((SRV_TASK*)&g_pstTaskTbl[tid]);
    if (g_pstTaskTbl[tid].hTask == SRV_INVALID)
        return SRV_ERR;

    return SRV_OK;
}

void* srv_msg_alloc(s32 size)
{
    u8* pMsg;

    pMsg = (u8*)srv_buf_alloc(sizeof(MSG_HDR)+size);
    if (pMsg == NULL)
        return NULL;

    ((MSG_HDR*)pMsg)->bodysize = size;

    return (pMsg+sizeof(MSG_HDR));
}

void srv_msg_free(void* pMsg)
{
    if (pMsg == NULL)
        return;
    srv_buf_free(((u8*)pMsg-sizeof(MSG_HDR)));
}

s32 srv_msg_snd(s32 stid, s32 dtid, s32 prio, s32 msgid, void* pMsg)
{
    MSG_HDR* pHdr;

    if (pMsg==NULL || dtid>=g_nMaxTblSize)
        return SRV_ERR;

    pHdr = (MSG_HDR*)(pMsg-sizeof(MSG_HDR));

    pHdr->stid = stid;
    pHdr->dtid = dtid;

    return srv_msq_snd(g_pstTaskTbl[dtid].hMsq, prio, msgid, pHdr, (pHdr->bodysize+sizeof(MSG_HDR)));
}

s32 srv_msg_rcv(s32 tid, s32* pMsgid, void** ppMsg)
{
	void* pHdr;

    if (tid>=g_nMaxTblSize || pMsgid==NULL || ppMsg==NULL)
        return SRV_ERR;

    if (srv_msq_rcv(g_pstTaskTbl[tid].hMsq,pMsgid,&pHdr) <= 0)
    	return SRV_ERR;

    *ppMsg = (u8*)pHdr + sizeof(MSG_HDR);

    return ((MSG_HDR*)pHdr)->bodysize;
}

s32 srv_msg_getsrctid(void *pMsg)
{
	if(NULL == pMsg)
	{
		return SRV_INVALID;
	}
	return ((MSG_HDR *)((u8 *)pMsg - sizeof(MSG_HDR)))->stid;
}

s32 srv_get_msg_num(s32 tid)
{
	if(tid >= g_nMaxTblSize)
	{
		return SRV_ERR;
	}
	return srv_msq_num(g_pstTaskTbl[tid].hMsq);
}

