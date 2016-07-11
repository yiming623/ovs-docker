/*
 * docker_tcpserver.c
 *
 *  Created on: Jun 27, 2015
 *      Author: flowvisor
 */

#include "docker_test.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "docker_config.h"
#include "ofproto-provider.h"
#include "ofproto-dpif-xlate.h"

extern neighbor_table g_stNeighborList;
extern u8 g_aucLocalEthAddr [6];
extern FILE* g_fp;

u16 sendingFakeNeighbor(u8* pBuffer, char* eid){
	u16 usOffset = 0;
	beacon_header header;
	u8 ucNumofService;
	header.ucVersion = 0x02;
	header.ucFlag = 0x0b;
	s32 n;
	char* service1type = "udpcl";
	char* service1port = "port=32000;";
	char* service2type = "tcpcl";
	char* service2port = "port=32001;";
	struct sockaddr_in servaddr, cliaddr;
	bzero(pBuffer,sizeof(pBuffer));
	usOffset = 0;
//	header.usSequenceNo = htons(lSequenceNo);
	memcpy(pBuffer, &header, sizeof(header));
	usOffset+=sizeof(header);
	pBuffer[usOffset] = strlen(eid); usOffset++;
	strcpy(pBuffer+usOffset, eid); usOffset+=strlen(eid);
	pBuffer[usOffset] = 1; usOffset++;//number of service
	pBuffer[usOffset] = strlen(service1type); usOffset++;
	strcpy(pBuffer+usOffset, service1type); usOffset+=strlen(service1type);
	pBuffer[usOffset] = strlen(service1port); usOffset++;
	strcpy(pBuffer+usOffset, service1port); usOffset+=strlen(service1port);
	pBuffer[usOffset] = strlen(service2type); usOffset++;
	strcpy(pBuffer+usOffset, service2type); usOffset+=strlen(service2type);
	pBuffer[usOffset] = strlen(service2port); usOffset++;
	strcpy(pBuffer+usOffset, service2port); usOffset+=strlen(service2port);
	pBuffer[usOffset] = 0x05; usOffset++;//time interval
	return usOffset;
}

void* test_sender_entry(void* arg)
{
	char str[10000];
	sprintf(str+strlen(str),"--=== entering test_sender_entrys ===--\n");
	write(g_fp, str, strlen(str));
	bzero(str, strlen(str));
	u8 ucIndex;
	u8* pBuffer = malloc(1024);
	bzero(pBuffer, 1024);
	u16 usPacketSize = sendingFakeNeighbor(pBuffer,"dtn://ovs0");
	u8 aucAppServerEthAddr [] = {0x00,0x00,0x00,0x00,0x00,0x02};
	u8 aucAppClientEthAddr [] = {0x00,0x00,0x00,0x00,0x00,0x01};
	u16 usSequenceNo = 0;
	while(1)
	{
		beacon_header* pBeaconHeader = (beacon_header*)pBuffer;
		pBeaconHeader->usSequenceNo = usSequenceNo;
		controller_udp_packet_out("s1",1,aucAppClientEthAddr,aucAppServerEthAddr/*h2*/,
				"192.168.2.100","224.0.0.1",4551,4551,pBuffer,usPacketSize);
		for(ucIndex = 0;ucIndex<g_stNeighborList.ulNeighborNo;ucIndex++){
			struct in_addr ip_addr;
			ip_addr.s_addr = g_stNeighborList.astNeighborEntities[ucIndex].ulIpAddr;
			printf("The IP address is %s\n", inet_ntoa(ip_addr));
			controller_udp_packet_out("s1",g_stNeighborList.astNeighborEntities[ucIndex].usOFPNo
					,g_aucLocalEthAddr,g_stNeighborList.astNeighborEntities[ucIndex].aucEthAddr/*h2*/,
				"192.168.2.100",inet_ntoa(ip_addr),4551,4551,pBuffer,usPacketSize);
		}
		usSequenceNo++;
		sleep(5);
	}
	return NULL;
}//

void* test_recver_entry(void* arg)
{
	char str[10000];
	s32 lMsgSize;
	s32 lMsgId;
	void *pMsg;

	while(1)
	{
		lMsgSize = srv_msg_rcv(TID_TEST_RECVER,&lMsgId,&pMsg);
		if(lMsgSize>0){
			sprintf(str+strlen(str),"--=== test message received in port = %u ===--\n",lMsgSize);
			write(g_fp, str, strlen(str));
			bzero(str, strlen(str));
		}
	}
	return SRV_OK;
}

