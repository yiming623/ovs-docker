/*
 * docker_kernel.h
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */
#ifndef DOCKER_KERNEL_H_
#define DOCKER_KERNEL_H_

#include "srv_common.h"
#include "ofproto/ofproto-provider.h"
#include "ofproto/ofproto.h"
#include "ofp-actions.h"
#include "lib/flow.h"


typedef unsigned char 	uint8_t;
typedef unsigned short 	uint16_t;
typedef unsigned int 	uint32_t;
typedef signed char		int8_t;
typedef signed short 	int16_t;
typedef signed int 		int32_t;

#define MAX_UDP_SEND_NUM       300
#define MAX_UDP_RECV_NUM       300

//#define MAX_NODE_NUM           1000
//#define MAX_VAR_NUM            200
//#define MAX_IMAGE_NUM          1000
//#define MAX_VAR_NUM_PER_MSG    30
//#define MAX_KEYIE_NUM          50
//#define MAX_ARG_NUM            10   //for command table use
//#define MAX_CMD_NUM            1500

#define OFP_HEADER_LENGTH		24
#define ETH_HEADER_LENGTH		14


typedef struct NEIGHBOR_ENTITY
{
	u8 	aucEthAddr [6];
	u32 ulIpAddr;
	u16 usPortNo;
	u16 usOFPNo;
}neighbor_entity;

typedef struct NEIGHBOR_TABLE
{
	u32 ulNeighborNo;
	neighbor_entity astNeighborEntities [256];
}neighbor_table;

void controller_add_flow(char* name, struct flow match, struct ofpact ofpacts);
void controller_del_flow(char* name, struct flow match, struct ofpact ofpacts);
void test();

void controller_udp_packet_out(char* switchName, uint16_t usOutPort,
		uint8_t srcMAC[6], uint8_t dstMAC[6],
		char* srcIP, char* dstIP,
		uint16_t usSrcPort, uint16_t usDstPort,
		uint8_t* pData, uint16_t usDataLength);


void* kernel_entry(void *arg);

#endif /* DOCKER_KERNEL_H_ */
