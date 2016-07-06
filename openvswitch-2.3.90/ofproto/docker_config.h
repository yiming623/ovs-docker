/*
 * docker_config.h
 *
 *  Created on: Jun 27, 2015
 *      Author: flowvisor
 */
#ifndef DOCKER_CONFIG_H_
#define DOCKER_CONFIG_H_

#include "srv_common.h"
#include "srv_buffer.h"
#include "srv_task.h"
#include "srv_trace.h"
#include "srv_socket.h"
#include "docker_kernel.h"
#include "docker_tcpsender.h"
#include "docker_test.h"
#include "docker_udp.h"

enum DOCKER_TID
{
	TID_OFPROTO,
	TID_KERNEL,
	TID_TEST_SENDER,
	TID_TEST_RECVER,
	TID_MAX
};

enum DOCKER_TRACE_MID
{
	MID_KERNEL,
	MID_TCPSERVER,
	MID_UDP,
	MID_TCPSENDER,
	MID_L3RECVER,
	MID_MAX
};

enum DOCKER_TRACE_LID
{
	LID_INFO,
	LID_WARNING,
	LID_ERROR,
	LID_MAX
};


void
docker_init(void);


#endif /* DOCKER_CONFIG_H_ */
