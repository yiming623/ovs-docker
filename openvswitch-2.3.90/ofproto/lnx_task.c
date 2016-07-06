/*
 * lnx_task.c
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

//#ifdef ARCH_LNX
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "srv_task.h"

static void* task_entry(void* arg);

SRV_HANDLE srv_task_create(SRV_TASK* pstTask)
{
	pthread_t thdid;

    if (pstTask == NULL)
        return SRV_INVALID;

    if (pthread_create(&thdid,NULL,task_entry,(void*)pstTask) != 0)
        return SRV_INVALID;

    return (SRV_HANDLE)thdid;
}

void srv_task_sleep(u32 usec)
{
	struct timespec requestTime, remainTime;
	requestTime.tv_sec = usec / 1000;
	requestTime.tv_nsec = (usec % 1000) * 1000000;
	while (nanosleep(&requestTime,&remainTime) == -1)
	{
	    if (errno != EINTR)
	    	break;

	    requestTime = remainTime;
	}
}


static void* task_entry(void* arg)
{
    SRV_TASK* pTask = (SRV_TASK*)arg;

    if (arg == NULL)
        return NULL;

    pTask->fpEntry((void*)pTask);

    return NULL;
}
//#endif


