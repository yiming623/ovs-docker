/*
 * docker_tcpsender.c
 *
 *  Created on: Oct 29, 2015
 *      Author: yimingxu
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "docker_tcpsender.h"

#include "docker_config.h"
#include "docker_test.h"

void * tcpsender_entry(void* arg){
	printf("%s-%u:Enter tcpsender_entry\r\n",__FILE__,__LINE__);
	while(1)
	{
		sleep(100);
	}
}
