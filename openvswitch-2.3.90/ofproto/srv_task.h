/*
 * srv_task.h
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

#ifndef SRV_TASK_H_
#define SRV_TASK_H_
#include "srv_common.h"


#define SRV_TASK_HIGH_PRIO   0
#define SRV_TASK_NORMAL_PRIO 1
#define SRV_TASK_LOW_PRIO    2

#define SRV_MSQ_ENABLE   1
#define SRV_MSQ_DISABLE  0

#define SRV_MSG_HIGH_PRIO   0
#define SRV_MSG_NORMAL_PRIO 1
#define SRV_MSG_LOW_PRIO    2

typedef struct SRV_TASK_
{
    s32       tid;
    char*     pName;
    void*     (*fpEntry)(void*);
    void*     arg;
    s32       prio;
    s32       isMsqEnable; // 1: create a message queue for this task, 0: not create
}SRV_TASK;

s32 srv_task_init(s32 maxtablesize);

s32 srv_task_register_table(const SRV_TASK* pstTaskTbl, s32 tblsize);
s32 srv_task_start_table();

s32 srv_task_register(const SRV_TASK* pstTask);
s32 srv_task_start(s32 tid);

void srv_task_sleep(u32 usec);

void* srv_msg_alloc(s32 size);
void srv_msg_free(void* pMsg);

s32 srv_msg_snd(s32 stid, s32 dtid, s32 prio, s32 msgid, void* pMsg);
s32 srv_msg_rcv(s32 tid, s32* pMsgid, void** ppMsg);

s32 srv_msg_getsrctid(void *pMsg);

s32 srv_get_msg_num(s32 tid);

#endif /* SRV_TASK_H_ */
