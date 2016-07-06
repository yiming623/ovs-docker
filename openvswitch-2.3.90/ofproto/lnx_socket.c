/*
 * lnx_socket.c
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

//#ifdef ARCH_LNX
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "srv_socket.h"

s32 srv_skt_init()
{
	return SRV_OK;
}

SRV_HANDLE srv_skt_socket(s32 socktype)
{
	s32 sock = -1;

	switch (socktype)
	{
	case SRV_DGRAM:
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		break;
	case SRV_STREAM:
		sock = socket(AF_INET, SOCK_STREAM, 0);
		break;
	default:
		break;
	}

	if (sock == -1)
		return SRV_INVALID;

	return (SRV_HANDLE)sock;
}

s32 srv_skt_close(SRV_HANDLE hSock)
{
	if(hSock == SRV_INVALID)
	{
		return SRV_ERR;
	}
	if(-1 == close(hSock))
	{
		return SRV_ERR;
	}
	return SRV_OK;
}

s32 srv_skt_opt(SRV_HANDLE hSock,s32 level,s32 option_name,const void *option_value,u32 option_len)
{
	if(hSock == SRV_INVALID)
	{
		return SRV_ERR;
	}
	if(-1 == setsockopt((s32)hSock,level,option_name,option_value,(socklen_t)option_len))
	{
		return SRV_ERR;
	}
	return SRV_OK;
}

s32 srv_skt_bind(SRV_HANDLE hSock, const SRV_SOCK_ADDR* pAddr)
{
	struct sockaddr_in sin;

	if (hSock==SRV_INVALID || pAddr==NULL)
		return SRV_ERR;

	sin.sin_family      = AF_INET;
	sin.sin_addr.s_addr = inet_addr(pAddr->ip);
	sin.sin_port        = htons(pAddr->port);

	if (bind((s32)hSock,(struct sockaddr*)&sin,sizeof(sin)) == -1)
		return SRV_ERR;

	return SRV_OK;
}

s32 srv_skt_listen(SRV_HANDLE hSock, s32 backlog)
{
	if (hSock == SRV_INVALID)
		return SRV_ERR;

	if (listen((s32)hSock,backlog) == -1)
		return SRV_ERR;

	return SRV_OK;
}

SRV_HANDLE srv_skt_accept(SRV_HANDLE hSock, SRV_SOCK_ADDR* pAddr)
{
	s32                clientSock;
	struct sockaddr_in sin;
	s32                sinlen = sizeof(sin);

	if (hSock==SRV_INVALID || pAddr==NULL)
		return SRV_INVALID;

    clientSock = accept((s32)hSock, (struct sockaddr*)&sin, &sinlen);
    if (clientSock == -1)
    	return SRV_INVALID;

    strcpy(pAddr->ip, inet_ntoa(sin.sin_addr));
    pAddr->port = ntohs(sin.sin_port);

    return (SRV_HANDLE)clientSock;
}

s32 srv_skt_connect(SRV_HANDLE hSock, const SRV_SOCK_ADDR* pAddr)
{
	struct sockaddr_in sin;

	if (hSock==SRV_INVALID || pAddr==NULL)
		return SRV_ERR;

	sin.sin_family      = AF_INET;
	sin.sin_addr.s_addr = inet_addr(pAddr->ip);
	sin.sin_port        = htons(pAddr->port);

	if (connect((s32)hSock,(struct sockaddr*)&sin,sizeof(sin)) == -1)
		return SRV_ERR;

	return SRV_OK;
}

s32 srv_skt_send(SRV_HANDLE hSock, const char* buf, s32 len)
{
	if (hSock==SRV_INVALID || buf==NULL)
		return SRV_ERR;

	return send((s32)hSock, buf, len, 0);
}

s32 srv_skt_recv(SRV_HANDLE hSock, char* buf, s32 len)
{
	if (hSock==SRV_INVALID || buf==NULL)
		return SRV_ERR;

	return recv((s32)hSock, buf, len, 0);
}

s32 srv_skt_sendto(SRV_HANDLE hSock, const char* buf, s32 len, const SRV_SOCK_ADDR* pAddr)
{
	struct sockaddr_in sin;

	if (hSock==SRV_INVALID || buf==NULL || pAddr==NULL)
		return SRV_ERR;

	sin.sin_family      = AF_INET;
	sin.sin_addr.s_addr = inet_addr(pAddr->ip);
	sin.sin_port        = htons(pAddr->port);

	return sendto((s32)hSock, buf, len, 0, (struct sockaddr*)&sin, sizeof(sin));
}

s32 srv_skt_recvfrom(SRV_HANDLE hSock, char* buf, s32 len, SRV_SOCK_ADDR* pAddr)
{
	struct sockaddr_in sin;
	s32                sinlen = sizeof(sin);
	s32                rcvsize;

	if (hSock==SRV_INVALID || buf==NULL)
		return SRV_ERR;

	rcvsize = recvfrom((s32)hSock, buf, len, 0, (struct sockaddr*)&sin, &sinlen);
	if (rcvsize < 0)
		return SRV_ERR;

	if (pAddr != NULL)
	{
		strcpy(pAddr->ip, inet_ntoa(sin.sin_addr));
		pAddr->port = ntohs(sin.sin_port);
	}

	return rcvsize;
}
//#endif

