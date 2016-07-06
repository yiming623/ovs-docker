/*
 * srv_bitmap.h
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

#ifndef SRV_BITMAP_H_
#define SRV_BITMAP_H_

#include "srv_common.h"

SRV_HANDLE srv_bmp_create(s32 bitnr);
void srv_bmp_destroy(SRV_HANDLE hBmp);

u32 srv_bmp_get_bit(SRV_HANDLE hBmp, s32 bit);
s32 srv_bmp_set_bit(SRV_HANDLE hBmp, s32 bit, u32 val);

#endif /* SRV_BITMAP_H_ */
