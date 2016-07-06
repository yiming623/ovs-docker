/*
 * docker_beacon.h
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

#ifndef DOCKER_BEACON_H_
#define DOCKER_BEACON_H_

#include "srv_common.h"

typedef struct BEACON_HEADER{
	u8 	ucVersion;
	u8 	ucFlag;
	u16 usSequenceNo;
}beacon_header;

void* test_sender_entry(void* arg);
void* test_recver_entry(void* arg);
s32 docker_recvdata();

#endif /* DOCKER_BEACON_H_ */
