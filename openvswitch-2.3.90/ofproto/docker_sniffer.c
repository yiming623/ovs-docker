/*
 * docker_sniffer.c
 *
 *  Created on: Jul 11, 2016
 *      Author: root
 */

#include "docker_config.h"

#include <pcap.h>
#include <stdio.h>
#include <stdlib.h> // for exit()
#include <string.h> //for memset

#include <sys/socket.h>
#include <arpa/inet.h> // for inet_ntoa()
#include <net/ethernet.h>
#include <netinet/ip_icmp.h>   //Provides declarations for icmp header
#include <netinet/udp.h>   //Provides declarations for udp header
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>    //Provides declarations for ip header



//FILE *logfile;
extern FILE* g_fp;

void process_packet(u8 *args, const struct pcap_pkthdr *header, const u8 *buffer, u32 ulSnifferId)
{
    int size = header->len;
    if(size<=0){
    	fprintf(g_fp, "ERROR: frame empty \n");
    }
    u8* pPacket;
    pPacket = srv_msg_alloc(size);
	memcpy(pPacket, buffer, size);
	if(srv_msg_snd(TID_SNIFFER_1+ulSnifferId,TID_TEST_RECVER,SRV_MSG_NORMAL_PRIO,1,pPacket)!=SRV_OK){
		fprintf(g_fp, "ERROR: Packet sending error in TID_SNIFFER_%d \n",ulSnifferId+1);
	}
}

void* sniffer_entry(void* arg)
{
    pcap_if_t *alldevsp , *device;
    pcap_t *handle; //Handle of the device that shall be sniffed
    s8 cmdstr [1000];
    s8 devname [100];
    bzero(cmdstr, sizeof(cmdstr));
    bzero(devname, sizeof(devname));
    if(arg==NULL){
    	fprintf(g_fp, "ERROR: sniffer id missing \n");
    	return;
    }

    u32 ulSnifferId = *(u32 *)arg;
    printf("--=== docker sniffer %d start ===-- \n");
    sprintf(cmdstr, "ip link add name s-sniffer%d type veth peer name h-sniffer%d", ulSnifferId, ulSnifferId);
    system(cmdstr);
    bzero(cmdstr, sizeof(cmdstr));
    sprintf(cmdstr, "ifconfig s-sniffer%d up", ulSnifferId);
    system(cmdstr);
    bzero(cmdstr, sizeof(cmdstr));
    sprintf(cmdstr, "ifconfig h-sniffer%d up", ulSnifferId);
    system(cmdstr);
    bzero(cmdstr, sizeof(cmdstr));
    sprintf(cmdstr, "ifconfig s-sniffer%d up", ulSnifferId);


    char errbuf[100];
    int count = 1 , n;

    sprintf(devname, "h-sniffer%d",ulSnifferId);
    handle = pcap_open_live(devname , 65536 , 1 , 0 , errbuf);

    if (handle == NULL)
    {
        fprintf(g_fp, "Couldn't open device %s : %s\n" , devname , errbuf, ulSnifferId);
        exit(1);
    }

    //Put the device in sniff loop
    pcap_loop(handle , -1 , process_packet , NULL);

    return 0;
}



