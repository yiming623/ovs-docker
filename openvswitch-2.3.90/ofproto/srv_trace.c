/*
 * srv_trace.c
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "srv_trace.h"
#include "srv_buffer.h"

#define MAX_TRACE_BUF 1024

typedef struct TRACE_CFG_
{
	SRV_TRACE_MODULE* pstMdlCfg;
	SRV_TRACE_LEVEL*  pstLvlCfg;
}TRACE_CFG;

static TRACE_CFG* g_pstTraceCfg = NULL;
static char g_sTraceBuf[MAX_TRACE_BUF] = {0};

s32 srv_trace_init(const SRV_TRACE_MODULE* pstModule, s32 modulenr, const SRV_TRACE_LEVEL* pstLevel, s32 levelnr)
{
	s32        nr,i;

	if (pstModule==NULL || pstLevel==NULL)
		return SRV_ERR;

	g_pstTraceCfg = (TRACE_CFG*)srv_buf_alloc(sizeof(TRACE_CFG));
	if (g_pstTraceCfg == NULL)
		return SRV_ERR;

	// register module information.

	nr = 0;
	for (i=0; i<modulenr; i++)
	{
		if (pstModule[i].mid > nr)
			nr = pstModule[i].mid;
	}
	nr++;

	g_pstTraceCfg->pstMdlCfg = (SRV_TRACE_MODULE*)srv_buf_alloc(sizeof(SRV_TRACE_MODULE)*nr);
	if (g_pstTraceCfg->pstMdlCfg == NULL)
		return SRV_ERR;

	for (i=0; i<modulenr; i++)
	{
		g_pstTraceCfg->pstMdlCfg[pstModule[i].mid].mid    = pstModule[i].mid;
		g_pstTraceCfg->pstMdlCfg[pstModule[i].mid].name   = pstModule[i].name;
		g_pstTraceCfg->pstMdlCfg[pstModule[i].mid].enable = pstModule[i].enable;
	}

	// register level information.

	nr = 0;
	for (i=0; i<levelnr; i++)
	{
		if (pstLevel[i].lid > nr)
			nr = pstLevel[i].lid;
	}
	nr++;

	g_pstTraceCfg->pstLvlCfg = (SRV_TRACE_LEVEL*)srv_buf_alloc(sizeof(SRV_TRACE_LEVEL)*nr);
	if (g_pstTraceCfg->pstLvlCfg == NULL)
		return SRV_ERR;

	for (i=0; i<modulenr; i++)
	{
		g_pstTraceCfg->pstLvlCfg[pstLevel[i].lid].lid    = pstLevel[i].lid;
		g_pstTraceCfg->pstLvlCfg[pstLevel[i].lid].name   = pstLevel[i].name;
		g_pstTraceCfg->pstLvlCfg[pstLevel[i].lid].enable = pstLevel[i].enable;
	}

	return SRV_OK;
}

void srv_trace_print(s32 mid, s32 level, char* filename, s32 linenr, char* format, ...)
{
    va_list    marker;
    s32        headsize;
    time_t     cursec;
    struct tm* pstCurTime;

    if (!g_pstTraceCfg->pstMdlCfg[mid].enable || !g_pstTraceCfg->pstLvlCfg[level].enable)
    	return;

    // get current time
    cursec = time(NULL);
    pstCurTime = localtime(&cursec);
    if (pstCurTime == NULL)
    	return;

    // print trace head
    headsize = sprintf(g_sTraceBuf, "<MID: %s> -- <LEVEL: %s> -- <FILE: %s> -- <LINE: %d> -- <%d-%d-%d %d:%d:%d> --",
    		           g_pstTraceCfg->pstMdlCfg[mid].name, g_pstTraceCfg->pstLvlCfg[level].name, filename, linenr,
    		           pstCurTime->tm_year+1900, pstCurTime->tm_mon+1, pstCurTime->tm_mday,
    		           pstCurTime->tm_hour, pstCurTime->tm_min, pstCurTime->tm_sec);

    // print trace body
    va_start(marker, format);
    vsprintf(&g_sTraceBuf[headsize], format, marker);
    va_end(marker);

    printf(g_sTraceBuf);
}


