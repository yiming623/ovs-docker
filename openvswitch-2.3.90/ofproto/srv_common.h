/*
 * srv_common.h
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

#ifndef SRV_COMMON_H_
#define SRV_COMMON_H_

typedef unsigned int SRV_HANDLE;

#define SRV_INVALID 0xffffffff

typedef unsigned int   u32;
typedef int            s32;
typedef unsigned short u16;
typedef short          s16;
typedef unsigned char  u8;
typedef char           s8;

#define SRV_OK      0
#define SRV_ERR    -1

typedef s32 (*SRV_COMP_FUNC)(u32 key1, u32 key2);

#endif /* SRV_COMMON_H_ */
