/*
 * docker.c
 *
 *  Created on: Jun 2, 2016
 *      Author: root
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
extern FILE * g_fp;

int docker_test(){
	char str [10000];
	bzero(str, sizeof(str));
	sprintf(str+strlen(str),"--=== docker_test ===--\n");
	write(g_fp, str, strlen(str));
	bzero(str, strlen(str));
}
