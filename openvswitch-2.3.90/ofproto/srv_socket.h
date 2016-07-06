/*
 * srv_socket.h
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

#ifndef SRV_SOCKET_H_
#define SRV_SOCKET_H_

#include "srv_common.h"

#define SRV_DGRAM 0
#define SRV_STREAM 1

#define MAX_IP_ADDR 16

typedef struct SRV_SOCK_ADDR_
{
	char ip[MAX_IP_ADDR];
	s32  port;
}SRV_SOCK_ADDR;


s32 srv_skt_init();

SRV_HANDLE srv_skt_socket(s32 socktype);

s32 srv_skt_close(SRV_HANDLE hSock);

s32 srv_skt_opt(SRV_HANDLE hSock,s32 level,s32 option_name,const void *option_value,u32 option_len);

s32 srv_skt_bind(SRV_HANDLE hSock, const SRV_SOCK_ADDR* pstAddr);

s32 srv_skt_listen(SRV_HANDLE hSock, s32 backlog);

SRV_HANDLE srv_skt_accept(SRV_HANDLE hSock, SRV_SOCK_ADDR* pstAddr);

s32 srv_skt_connect(SRV_HANDLE hSock, const SRV_SOCK_ADDR* pstAddr);

s32 srv_skt_send(SRV_HANDLE hSock, const char* buf, s32 len);

s32 srv_skt_recv(SRV_HANDLE hSock, char* buf, s32 len);

s32 srv_skt_sendto(SRV_HANDLE hSock, const char* buf, s32 len, const SRV_SOCK_ADDR* pstAddr);

s32 srv_skt_recvfrom(SRV_HANDLE hSock, char* buf, s32 len, SRV_SOCK_ADDR* pstAddr);

#endif /* SRV_SOCKET_H_ */
