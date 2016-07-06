/*
 * lnx_mutex.c
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

//#ifdef ARCH_LNX
#include <pthread.h>
#include <stdlib.h>
#include "srv_mutex.h"

SRV_HANDLE srv_mutex_create()
{
    pthread_mutex_t* mutex;

    mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if (mutex == NULL)
    	return SRV_INVALID;

    if (pthread_mutex_init(mutex,NULL) != 0)
    {
    	free(mutex);
        return SRV_INVALID;
    }

    return (SRV_HANDLE)mutex;
}

s32 srv_mutex_destroy(SRV_HANDLE hMutex)
{
    pthread_mutex_t* mutex = (pthread_mutex_t*)hMutex;

    if (hMutex == SRV_INVALID)
        return SRV_ERR;

    if (pthread_mutex_destroy(mutex) != 0)
        return SRV_ERR;

	free(mutex);

    return SRV_OK;
}

s32 srv_mutex_lock(SRV_HANDLE hMutex)
{
    pthread_mutex_t* mutex = (pthread_mutex_t*)hMutex;

    if (hMutex == SRV_INVALID)
        return SRV_ERR;

    if (pthread_mutex_lock(mutex) != 0)
        return SRV_ERR;

    return SRV_OK;
}

s32 srv_mutex_unlock(SRV_HANDLE hMutex)
{
    pthread_mutex_t* mutex = (pthread_mutex_t*)hMutex;

    if (hMutex == SRV_INVALID)
        return SRV_ERR;

    if (pthread_mutex_unlock(mutex) != 0)
        return SRV_ERR;

    return SRV_OK;
}
//#endif

