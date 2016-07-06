/*
 * docker_kernel.c
 *
 *  Created on: Jun 27, 2015
 *      Author: flowvisor
 */

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include "docker_kernel.h"
#include "docker_config.h"


s32          g_EpollFd;

neighbor_table g_stNeighborList;
u8 g_aucLocalEthAddr [6] = {0x08,0x00,0x27,0x12,0x34,0x56};
extern FILE* g_fp;

typedef struct DOCKER_FLOW_MATCH_
{
	u16 usInPort;
	u8 aucMacSrc [6];
	u8 aucMacDst [6];
	u16 usDLType;
	u8 ucNWProto;
	u8 ucPad;
	u32 ulNWSrc;
	u32 ulNWDst;
	u16 usTPSrc;
	u16 usTPDst;
}flow_match;

typedef struct pseudo_header
{
	uint32_t source_address;
	uint32_t dest_address;
	uint8_t placeholder;
	uint8_t protocol;
	uint16_t udp_length;
};

void controller_add_flow(char* name, struct flow match, struct ofpact ofpacts){
	struct ofproto * ofp;
	ofp = ofproto_lookup(name);
	struct ofputil_flow_mod fm;
	struct ofpact_output ofp_act;
	struct ofp_header *oh;

	u8 bytes_zero [16];
	u8 bytes_mask [16];

	memset(bytes_zero, 0x00, 16);
	memset(bytes_mask, 0xFF, 16);

	bzero(&fm,sizeof(fm));
	match_init_catchall(&fm.match);
	fm.priority = 1;//have a higher priority than pre-install.
	/*IN_PORT*/
	if(memcmp(&match.in_port, bytes_zero, sizeof(fm.match.flow.in_port))){
		memcpy(&fm.match.flow.in_port,&match.in_port,sizeof(fm.match.flow.in_port));
		memcpy(&fm.match.wc.masks.in_port,bytes_mask,sizeof(fm.match.flow.in_port));
	}
	/*ARP*/
	if(memcmp(&match.arp_sha, bytes_zero, sizeof(fm.match.flow.arp_sha))){
		memcpy(&fm.match.flow.arp_sha,&match.arp_sha,sizeof(fm.match.flow.arp_sha));
		memcpy(&fm.match.wc.masks.arp_sha,bytes_mask,sizeof(fm.match.flow.arp_sha));
	}
	if(memcmp(&match.arp_tha, bytes_zero, sizeof(fm.match.flow.arp_tha))){
		memcpy(&fm.match.flow.arp_tha,&match.arp_tha,sizeof(fm.match.flow.arp_tha));
		memcpy(&fm.match.wc.masks.arp_tha,bytes_mask,sizeof(fm.match.flow.arp_tha));
	}
	/*ETH*/
	if(memcmp(&match.dl_dst, bytes_zero, sizeof(fm.match.flow.dl_dst))){
		memcpy(&fm.match.flow.dl_dst,&match.dl_dst,sizeof(fm.match.flow.dl_dst));
		memcpy(&fm.match.wc.masks.dl_dst,bytes_mask,sizeof(fm.match.flow.dl_dst));
	}
	if(memcmp(&match.dl_src, bytes_zero, sizeof(fm.match.flow.dl_src))){
		memcpy(&fm.match.flow.dl_src,&match.dl_src,sizeof(fm.match.flow.dl_src));
		memcpy(&fm.match.wc.masks.dl_src,bytes_mask,sizeof(fm.match.flow.dl_src));
	}
	if(memcmp(&match.dl_type, bytes_zero, sizeof(fm.match.flow.dl_type))){
		memcpy(&fm.match.flow.dl_type,&match.dl_type,sizeof(fm.match.flow.dl_type));
		memcpy(&fm.match.wc.masks.dl_type,bytes_mask,sizeof(fm.match.flow.dl_type));
	}
	if(memcmp(&match.vlan_tci, bytes_zero, sizeof(fm.match.flow.vlan_tci))){
		memcpy(&fm.match.flow.vlan_tci,&match.vlan_tci,sizeof(fm.match.flow.vlan_tci));
		memcpy(&fm.match.wc.masks.vlan_tci,bytes_mask,sizeof(fm.match.flow.vlan_tci));
	}
	/*IPV6*/
	if(memcmp(&match.ipv6_dst, bytes_zero, sizeof(fm.match.flow.ipv6_dst))){
		memcpy(&fm.match.flow.ipv6_dst,&match.ipv6_dst,sizeof(fm.match.flow.ipv6_dst));
		memcpy(&fm.match.wc.masks.ipv6_dst,bytes_mask,sizeof(fm.match.flow.ipv6_dst));
	}
	if(memcmp(&match.ipv6_src, bytes_zero, sizeof(fm.match.flow.ipv6_src))){
		memcpy(&fm.match.flow.ipv6_src,&match.ipv6_src,sizeof(fm.match.flow.ipv6_src));
		memcpy(&fm.match.wc.masks.ipv6_src,bytes_mask,sizeof(fm.match.flow.ipv6_src));
	}
	if(memcmp(&match.ipv6_label, bytes_zero, sizeof(fm.match.flow.ipv6_label))){
		memcpy(&fm.match.flow.ipv6_label,&match.ipv6_label,sizeof(fm.match.flow.ipv6_label));
		memcpy(&fm.match.wc.masks.ipv6_label,bytes_mask,sizeof(fm.match.flow.ipv6_label));
	}
	if(memcmp(&match.nd_target, bytes_zero, sizeof(fm.match.flow.nd_target))){
		memcpy(&fm.match.flow.nd_target,&match.nd_target,sizeof(fm.match.flow.nd_target));
		memcpy(&fm.match.wc.masks.nd_target,bytes_mask,sizeof(fm.match.flow.nd_target));
	}
	/*IPV4*/
	if(memcmp(&match.igmp_group_ip4, bytes_zero, sizeof(fm.match.flow.igmp_group_ip4))){
		memcpy(&fm.match.flow.igmp_group_ip4,&match.igmp_group_ip4,sizeof(fm.match.flow.igmp_group_ip4));
		memcpy(&fm.match.wc.masks.igmp_group_ip4,bytes_mask,sizeof(fm.match.flow.igmp_group_ip4));
	}
	if(memcmp(&match.nw_dst, bytes_zero, sizeof(fm.match.flow.nw_dst))){
		memcpy(&fm.match.flow.nw_dst,&match.nw_dst,sizeof(fm.match.flow.nw_dst));
		memcpy(&fm.match.wc.masks.nw_dst,bytes_mask,sizeof(fm.match.flow.nw_dst));
	}
	if(memcmp(&match.nw_src, bytes_zero, sizeof(fm.match.flow.nw_src))){
		memcpy(&fm.match.flow.nw_src,&match.nw_src,sizeof(fm.match.flow.nw_src));
		memcpy(&fm.match.wc.masks.nw_src,bytes_mask,sizeof(fm.match.flow.nw_src));
	}
	if(memcmp(&match.nw_proto, bytes_zero, sizeof(fm.match.flow.nw_proto))){
		memcpy(&fm.match.flow.nw_proto,&match.nw_proto,sizeof(fm.match.flow.nw_proto));
		memcpy(&fm.match.wc.masks.nw_proto,bytes_mask,sizeof(fm.match.flow.nw_proto));
	}
	if(memcmp(&match.nw_frag, bytes_zero, sizeof(fm.match.flow.nw_frag))){
		memcpy(&fm.match.flow.nw_frag,&match.nw_frag,sizeof(fm.match.flow.nw_frag));
		memcpy(&fm.match.wc.masks.nw_frag,bytes_mask,sizeof(fm.match.flow.nw_frag));
	}
	if(memcmp(&match.nw_tos, bytes_zero, sizeof(fm.match.flow.nw_tos))){
		memcpy(&fm.match.flow.nw_tos,&match.nw_tos,sizeof(fm.match.flow.nw_tos));
		memcpy(&fm.match.wc.masks.nw_tos,bytes_mask,sizeof(fm.match.flow.nw_tos));
	}
	if(memcmp(&match.nw_ttl, bytes_zero, sizeof(fm.match.flow.nw_ttl))){
		memcpy(&fm.match.flow.nw_ttl,&match.nw_ttl,sizeof(fm.match.flow.nw_ttl));
		memcpy(&fm.match.wc.masks.nw_ttl,bytes_mask,sizeof(fm.match.flow.nw_ttl));
	}
	/*L4*/
	if(memcmp(&match.tcp_flags, bytes_zero, sizeof(fm.match.flow.tcp_flags))){
		memcpy(&fm.match.flow.tcp_flags,&match.tcp_flags,sizeof(fm.match.flow.tcp_flags));
		memcpy(&fm.match.wc.masks.tcp_flags,bytes_mask,sizeof(fm.match.flow.tcp_flags));
	}
	if(memcmp(&match.tp_dst, bytes_zero, sizeof(fm.match.flow.tp_dst))){
		memcpy(&fm.match.flow.tp_dst,&match.tp_dst,sizeof(fm.match.flow.tp_dst));
		memcpy(&fm.match.wc.masks.tp_dst,bytes_mask,sizeof(fm.match.flow.tp_dst));
	}
	if(memcmp(&match.tp_src, bytes_zero, sizeof(fm.match.flow.tp_src))){
		memcpy(&fm.match.flow.tp_src,&match.tp_src,sizeof(fm.match.flow.tp_src));
		memcpy(&fm.match.wc.masks.tp_src,bytes_mask,sizeof(fm.match.flow.tp_src));
	}


	fm.command = 0;
	fm.out_port = 0;
	fm.buffer_id = 0xFFFFFFFF;
	fm.table_id = 0xFF;
	fm.cookie = 0;
	fm.cookie_mask = 0;
	fm.new_cookie = 0x70900000000A000;
	fm.ofpacts_len = 8;
	fm.ofpacts = &ofpacts;

	ofproto_flow_mod(ofp, &fm);
}

void controller_del_flow(char* name, struct flow match,  struct ofpact ofpacts){
	struct ofproto * ofp;
	ofp = ofproto_lookup(name);
	struct ofputil_flow_mod fm;
	struct ofpact_output ofp_act;
	struct ofp_header *oh;

	u8 bytes_zero [16];
	u8 bytes_mask [16];

	memset(bytes_zero, 0x00, 16);
	memset(bytes_mask, 0xFF, 16);

	bzero(&fm,sizeof(fm));
	match_init_catchall(&fm.match);
	fm.priority = 1;//have a higher priority than pre-install.
	/*IN_PORT*/
	if(memcmp(&match.in_port, bytes_zero, sizeof(fm.match.flow.in_port))){
		memcpy(&fm.match.flow.in_port,&match.in_port,sizeof(fm.match.flow.in_port));
		memcpy(&fm.match.wc.masks.in_port,bytes_mask,sizeof(fm.match.flow.in_port));
	}
	/*ARP*/
	if(memcmp(&match.arp_sha, bytes_zero, sizeof(fm.match.flow.arp_sha))){
		memcpy(&fm.match.flow.arp_sha,&match.arp_sha,sizeof(fm.match.flow.arp_sha));
		memcpy(&fm.match.wc.masks.arp_sha,bytes_mask,sizeof(fm.match.flow.arp_sha));
	}
	if(memcmp(&match.arp_tha, bytes_zero, sizeof(fm.match.flow.arp_tha))){
		memcpy(&fm.match.flow.arp_tha,&match.arp_tha,sizeof(fm.match.flow.arp_tha));
		memcpy(&fm.match.wc.masks.arp_tha,bytes_mask,sizeof(fm.match.flow.arp_tha));
	}
	/*ETH*/
	if(memcmp(&match.dl_dst, bytes_zero, sizeof(fm.match.flow.dl_dst))){
		memcpy(&fm.match.flow.dl_dst,&match.dl_dst,sizeof(fm.match.flow.dl_dst));
		memcpy(&fm.match.wc.masks.dl_dst,bytes_mask,sizeof(fm.match.flow.dl_dst));
	}
	if(memcmp(&match.dl_src, bytes_zero, sizeof(fm.match.flow.dl_src))){
		memcpy(&fm.match.flow.dl_src,&match.dl_src,sizeof(fm.match.flow.dl_src));
		memcpy(&fm.match.wc.masks.dl_src,bytes_mask,sizeof(fm.match.flow.dl_src));
	}
	if(memcmp(&match.dl_type, bytes_zero, sizeof(fm.match.flow.dl_type))){
		memcpy(&fm.match.flow.dl_type,&match.dl_type,sizeof(fm.match.flow.dl_type));
		memcpy(&fm.match.wc.masks.dl_type,bytes_mask,sizeof(fm.match.flow.dl_type));
	}
	if(memcmp(&match.vlan_tci, bytes_zero, sizeof(fm.match.flow.vlan_tci))){
		memcpy(&fm.match.flow.vlan_tci,&match.vlan_tci,sizeof(fm.match.flow.vlan_tci));
		memcpy(&fm.match.wc.masks.vlan_tci,bytes_mask,sizeof(fm.match.flow.vlan_tci));
	}
	/*IPV6*/
	if(memcmp(&match.ipv6_dst, bytes_zero, sizeof(fm.match.flow.ipv6_dst))){
		memcpy(&fm.match.flow.ipv6_dst,&match.ipv6_dst,sizeof(fm.match.flow.ipv6_dst));
		memcpy(&fm.match.wc.masks.ipv6_dst,bytes_mask,sizeof(fm.match.flow.ipv6_dst));
	}
	if(memcmp(&match.ipv6_src, bytes_zero, sizeof(fm.match.flow.ipv6_src))){
		memcpy(&fm.match.flow.ipv6_src,&match.ipv6_src,sizeof(fm.match.flow.ipv6_src));
		memcpy(&fm.match.wc.masks.ipv6_src,bytes_mask,sizeof(fm.match.flow.ipv6_src));
	}
	if(memcmp(&match.ipv6_label, bytes_zero, sizeof(fm.match.flow.ipv6_label))){
		memcpy(&fm.match.flow.ipv6_label,&match.ipv6_label,sizeof(fm.match.flow.ipv6_label));
		memcpy(&fm.match.wc.masks.ipv6_label,bytes_mask,sizeof(fm.match.flow.ipv6_label));
	}
	if(memcmp(&match.nd_target, bytes_zero, sizeof(fm.match.flow.nd_target))){
		memcpy(&fm.match.flow.nd_target,&match.nd_target,sizeof(fm.match.flow.nd_target));
		memcpy(&fm.match.wc.masks.nd_target,bytes_mask,sizeof(fm.match.flow.nd_target));
	}
	/*IPV4*/
	if(memcmp(&match.igmp_group_ip4, bytes_zero, sizeof(fm.match.flow.igmp_group_ip4))){
		memcpy(&fm.match.flow.igmp_group_ip4,&match.igmp_group_ip4,sizeof(fm.match.flow.igmp_group_ip4));
		memcpy(&fm.match.wc.masks.igmp_group_ip4,bytes_mask,sizeof(fm.match.flow.igmp_group_ip4));
	}
	if(memcmp(&match.nw_dst, bytes_zero, sizeof(fm.match.flow.nw_dst))){
		memcpy(&fm.match.flow.nw_dst,&match.nw_dst,sizeof(fm.match.flow.nw_dst));
		memcpy(&fm.match.wc.masks.nw_dst,bytes_mask,sizeof(fm.match.flow.nw_dst));
	}
	if(memcmp(&match.nw_src, bytes_zero, sizeof(fm.match.flow.nw_src))){
		memcpy(&fm.match.flow.nw_src,&match.nw_src,sizeof(fm.match.flow.nw_src));
		memcpy(&fm.match.wc.masks.nw_src,bytes_mask,sizeof(fm.match.flow.nw_src));
	}
	if(memcmp(&match.nw_proto, bytes_zero, sizeof(fm.match.flow.nw_proto))){
		memcpy(&fm.match.flow.nw_proto,&match.nw_proto,sizeof(fm.match.flow.nw_proto));
		memcpy(&fm.match.wc.masks.nw_proto,bytes_mask,sizeof(fm.match.flow.nw_proto));
	}
	if(memcmp(&match.nw_frag, bytes_zero, sizeof(fm.match.flow.nw_frag))){
		memcpy(&fm.match.flow.nw_frag,&match.nw_frag,sizeof(fm.match.flow.nw_frag));
		memcpy(&fm.match.wc.masks.nw_frag,bytes_mask,sizeof(fm.match.flow.nw_frag));
	}
	if(memcmp(&match.nw_tos, bytes_zero, sizeof(fm.match.flow.nw_tos))){
		memcpy(&fm.match.flow.nw_tos,&match.nw_tos,sizeof(fm.match.flow.nw_tos));
		memcpy(&fm.match.wc.masks.nw_tos,bytes_mask,sizeof(fm.match.flow.nw_tos));
	}
	if(memcmp(&match.nw_ttl, bytes_zero, sizeof(fm.match.flow.nw_ttl))){
		memcpy(&fm.match.flow.nw_ttl,&match.nw_ttl,sizeof(fm.match.flow.nw_ttl));
		memcpy(&fm.match.wc.masks.nw_ttl,bytes_mask,sizeof(fm.match.flow.nw_ttl));
	}
	/*L4*/
	if(memcmp(&match.tcp_flags, bytes_zero, sizeof(fm.match.flow.tcp_flags))){
		memcpy(&fm.match.flow.tcp_flags,&match.tcp_flags,sizeof(fm.match.flow.tcp_flags));
		memcpy(&fm.match.wc.masks.tcp_flags,bytes_mask,sizeof(fm.match.flow.tcp_flags));
	}
	if(memcmp(&match.tp_dst, bytes_zero, sizeof(fm.match.flow.tp_dst))){
		memcpy(&fm.match.flow.tp_dst,&match.tp_dst,sizeof(fm.match.flow.tp_dst));
		memcpy(&fm.match.wc.masks.tp_dst,bytes_mask,sizeof(fm.match.flow.tp_dst));
	}
	if(memcmp(&match.tp_src, bytes_zero, sizeof(fm.match.flow.tp_src))){
		memcpy(&fm.match.flow.tp_src,&match.tp_src,sizeof(fm.match.flow.tp_src));
		memcpy(&fm.match.wc.masks.tp_src,bytes_mask,sizeof(fm.match.flow.tp_src));
	}


	fm.command = 0;
	fm.out_port = 0;
	fm.buffer_id = 0xFFFFFFFF;
	fm.table_id = 0xFF;
	fm.cookie = 0;
	fm.cookie_mask = 0;
	fm.new_cookie = 0x70900000000A000;
	fm.ofpacts_len = 8;
	fm.ofpacts = &ofpacts;

	ofproto_flow_mod(ofp, &fm);
}



void controller_udp_packet_out(char* switchName, uint16_t usOutPort,
		uint8_t srcMAC[6], uint8_t dstMAC[6],
		char* srcIP, char* dstIP,
		uint16_t usSrcPort, uint16_t usDstPort,
		uint8_t* pData, uint16_t usDataLength){
	char str[1000];
	u8 ucOut = usOutPort;
	struct ofproto * ofp;
	ofp = ofproto_lookup(switchName);
	if(ofp==NULL) return;
	char  *pseudogram;
	int i;
	uint16_t usTotalSize = OFP_HEADER_LENGTH + ETH_HEADER_LENGTH +sizeof(struct iphdr) + sizeof(struct udphdr) + usDataLength;
	int8_t buffer [4096];
	bzero(buffer, sizeof(buffer));
	uint8_t ofpheader [] = {0x01,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x08
			,0x00,0x00,0x00,0x08,0x00,ucOut,0xff,0xff};
	struct ethhdr *pEthHeader = (struct ethhdr *)(buffer+OFP_HEADER_LENGTH);
	struct iphdr *pIpHeader = (struct iphdr *)(buffer+OFP_HEADER_LENGTH+ETH_HEADER_LENGTH);
	struct udphdr *pUdpHeaeder = (struct udphdr *) (buffer+OFP_HEADER_LENGTH+ETH_HEADER_LENGTH+sizeof(struct ip));
	struct pseudo_header pPsudoUdpHeader;
	memcpy(ofpheader+2, &usTotalSize, 2);
	for(i=0;i<6;i++){
		buffer[OFP_HEADER_LENGTH+i] = dstMAC[i];
	}
	for(i=0;i<6;i++){
		buffer[OFP_HEADER_LENGTH+6+i] = srcMAC[i];
	}

	buffer[36]=0x08;
	buffer[37]=0x00;

	//IP header
	pIpHeader->ihl = 5;
	pIpHeader->version = 4;
	pIpHeader->tos = 0;
	pIpHeader->tot_len = sizeof (struct iphdr) + sizeof (struct udphdr) + usDataLength;//datasize;
	pIpHeader->id = htonl (54321); //Id of this packet
	pIpHeader->frag_off = 0;
	pIpHeader->ttl = 255;
	pIpHeader->protocol = IPPROTO_UDP;
	pIpHeader->check = 0;      //Set to 0 before calculating checksum
	pIpHeader->saddr = inet_addr ( srcIP );    //Spoof the source ip address
	pIpHeader->daddr = inet_addr ( dstIP );
	pIpHeader->check = csum ( buffer+OFP_HEADER_LENGTH+ETH_HEADER_LENGTH, pIpHeader->tot_len);

	pUdpHeaeder->source = htons (usSrcPort);
	pUdpHeaeder->dest = htons (usDstPort);
	pUdpHeaeder->len = htons(8 + usDataLength); //tcp header size
	pUdpHeaeder->check = 0; //leave checksum 0 now, filled later by pseudo header
	pPsudoUdpHeader.source_address = inet_addr( srcIP );
	pPsudoUdpHeader.dest_address = inet_addr( dstIP );
	pPsudoUdpHeader.placeholder = 0;
	pPsudoUdpHeader.protocol = IPPROTO_UDP;
	pPsudoUdpHeader.udp_length = htons(sizeof(struct udphdr) + usDataLength );
	memcpy(buffer+OFP_HEADER_LENGTH+ETH_HEADER_LENGTH+sizeof(struct ip)+sizeof(struct udphdr),pData,usDataLength);
	int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + usDataLength;
	pseudogram = malloc(psize);
	memcpy(pseudogram , (char*) &pPsudoUdpHeader , sizeof (struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header) , pUdpHeaeder , sizeof(struct udphdr) + usDataLength);
	pUdpHeaeder->check = csum( (unsigned short*) pseudogram , psize);
	memcpy(buffer, ofpheader, sizeof(ofpheader));
	struct ofp_header *oh = (struct ofp_header *)buffer;
	handle_packet_out_docker("s1", oh);
}

void test(){
	printf("");
}


void* kernel_entry(void* arg)
{
	//SRV_TRACE_PRINT(MID_KERNEL,LID_INFO,"Enter kernel_entry\r\n");
	printf("%s-%u:Enter kernel_entryyyyyy\r\n",__FILE__,__LINE__);
	struct timespec tim, tim2;
	tim.tv_nsec = 0;
	tim.tv_sec = 10000;
	uint8_t ucSeqNo = 0;
	while(1)
	{
		if(nanosleep(&tim, &tim2)<0){
			printf("nanosleep failed \n");
		}
	}
	return NULL;
}
