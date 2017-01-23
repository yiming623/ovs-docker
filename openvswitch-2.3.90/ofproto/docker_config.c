/*
 * docker_config.c
 *
 *  Created on: Jun 27, 2015
 *      Author: flowvisor
 */

#include "stdio.h"
#include "docker_config.h"


static SRV_BUF_POOL g_astBufPool[] =
{
	{32,     1000},
    {64,     1000},
    {128,    1000},
    {256,    1000},
    {512,    1000},
    {1024,   500},
    {2048,   500},
    {4096,   500},
    {8192,   500},
    {16384,  300},
    {32768,  300},
    {65536,  300},
    {131072, 300},
    {262144, 50},
    {524288, 20},
    {1048576,20}
};

u32 g_ulSnifferIds [5] = {0,1,2,3,4};

static SRV_TASK g_astTaskTbl[] =
{
	{TID_KERNEL,    "KERNEL",    kernel_entry,    NULL, SRV_TASK_NORMAL_PRIO, SRV_MSQ_ENABLE},
	{TID_TEST_SENDER, "TEST_SENDER", test_sender_entry, NULL, SRV_TASK_NORMAL_PRIO, SRV_MSQ_ENABLE},
	{TID_TEST_RECVER, "TEST_RECVER", test_recver_entry, NULL, SRV_TASK_NORMAL_PRIO, SRV_MSQ_ENABLE}
//	{TID_UDP,       "UDP",       udp_entry,       NULL, SRV_TASK_NORMAL_PRIO, SRV_MSQ_ENABLE},
//	{TID_TCPSENDER, "TCPSENDER", tcpsender_entry,       NULL, SRV_TASK_NORMAL_PRIO, SRV_MSQ_ENABLE},
//	{TID_L3RECVER,	"L3EMSGRECVER",tcpsender_entry,NULL, SRV_TASK_NORMAL_PRIO, SRV_MSQ_ENABLE}
};

static SRV_TRACE_MODULE g_astTraceMdl[] =
{
	{MID_KERNEL, "KERNEL", 1},
	{MID_TCPSERVER,"TCPSERVER",1},
	{MID_UDP,"UDP",1}
};

static SRV_TRACE_LEVEL g_astTraceLvl[] =
{
	{LID_INFO, "INFO", 1},
	{LID_WARNING, "WARNING", 1},
	{LID_ERROR, "ERROR", 1}
};

void docker_init(void)
{
	s32 ret;

	ret = srv_buf_init(g_astBufPool, sizeof(g_astBufPool)/sizeof(SRV_BUF_POOL));
	if(SRV_OK != ret)
	{
		printf("Buffer init failed.\r\n");
	}

	ret = srv_trace_init(g_astTraceMdl, sizeof(g_astTraceMdl) / sizeof(SRV_TRACE_MODULE),
						   g_astTraceLvl, sizeof(g_astTraceLvl) / sizeof(SRV_TRACE_LEVEL));
	if(SRV_OK != ret)
	{
		printf("Trace init failed.\r\n");
	}

	ret = srv_task_init(TID_MAX);
	if(SRV_OK != ret)
	{
		printf("Task init failed.\r\n");
	}

	ret = srv_task_register_table(g_astTaskTbl, sizeof(g_astTaskTbl)/sizeof(SRV_TASK));
	if(SRV_OK != ret)
	{
		printf("Task register failed.\r\n");
	}

	 ret = srv_task_start_table();
	 if(SRV_OK != ret)
	 {
		 printf("Task start failed.\r\n");
	 }

}
