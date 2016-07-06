/*
 * srv_trace.h
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

#ifndef SRV_TRACE_H_
#define SRV_TRACE_H_

#include "srv_common.h"

typedef struct SRV_TRACE_MODULE_
{
	s32   mid;
	char* name;
	s32   enable;
}SRV_TRACE_MODULE;

typedef struct SRV_TRACE_LEVEL_
{
	s32   lid;
	char* name;
	s32   enable;
}SRV_TRACE_LEVEL;


s32 srv_trace_init(const SRV_TRACE_MODULE* pstModule, s32 modulenr, const SRV_TRACE_LEVEL* pstLevel, s32 levelnr);

void srv_trace_print(s32 mid, s32 level, char* filename, s32 linenr, char* format, ...);

#define SRV_TRACE_PRINT(mid, level, format...) \
	    srv_trace_print(mid, level, __FILE__, __LINE__, format)

#endif /* SRV_TRACE_H_ */
