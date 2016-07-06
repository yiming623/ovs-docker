/*
 * docker_udp.c
 *
 *  Created on: Jun 27, 2015
 *      Author: flowvisor
 */
#include <stdio.h>
#include <stddef.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include "docker_udp.h"
#include "docker_kernel.h"
#include "docker_config.h"

extern s32  g_EpollFd;

void* udp_entry(void* arg)
{
	printf("%s-%u:Enter udp_entry\r\n",__FILE__,__LINE__);

	udp_recv_msg();

	return NULL;
}

void udp_recv_msg()
{
	s8* pBuf = NULL;
	struct epoll_event events[MAX_UDP_RECV_NUM];
	struct sockaddr_in peerAddr;
	socklen_t peerAddrLen = sizeof(peerAddr);
	s32 num,i,size;

	while(1)
	{
		num = epoll_wait(g_EpollFd, events, MAX_UDP_RECV_NUM, -1);
		for(i = 0;i < num;i ++)
		{
			if (events[i].events & EPOLLIN)
			{
				pBuf = srv_msg_alloc(MAX_UDP_LEN);
				if(NULL == pBuf)
				{
					printf("%s-%u:srv_msg_alloc failed\r\n",__FILE__,__LINE__);
					continue;
				}

				size = recvfrom(events[i].data.fd,
								pBuf ,
								MAX_UDP_LEN,
								0,
								(struct sockaddr*)&peerAddr,
								&peerAddrLen);
				if(size > 0)
				{

				}
				else
				{

				}
			}
		}
	}

}
