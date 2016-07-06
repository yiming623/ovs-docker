/*
 * docker_udp.h
 *
 *  Created on: Jun 27, 2015
 *      Author: flowvisor
 */
#ifndef DOCKER_UDP_H_
#define DOCKER_UDP_H_
#include "srv_common.h"

#define MAX_UDP_LEN     1500

void* udp_entry(void* arg);

void udp_recv_msg();

#endif /* DOCKER_UDP_H_ */
