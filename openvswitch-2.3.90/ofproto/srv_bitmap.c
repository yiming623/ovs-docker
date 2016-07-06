/*
 * srv_bitmap.c
 *
 *  Created on: 2015-6-7
 *      Author: flowvisor
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "srv_bitmap.h"
//#include "srv_buffer.h"

#define SRV_BMP_BITS 32

SRV_HANDLE srv_bmp_create(s32 bitnr)
{
	u32* pBmp;
	s32  size;

	((bitnr%SRV_BMP_BITS) == 0) ? (size=bitnr/SRV_BMP_BITS) : (size=bitnr/SRV_BMP_BITS+1);

	//pBmp = (u32*)srv_buf_alloc(size*sizeof(u32));
	pBmp = (u32*)malloc(size*sizeof(u32));//change by yxf
	if (pBmp == NULL)
		return SRV_INVALID;

	memset(pBmp, 0, size*sizeof(u32));

	return (SRV_HANDLE)pBmp;
}

void srv_bmp_destroy(SRV_HANDLE hBmp)
{
	u32* pBmp = (u32*)hBmp;

    if (hBmp == SRV_INVALID)
    	return;

    //srv_buf_free(pBmp);
    free(pBmp);//Change by yxf
}

u32 srv_bmp_get_bit(SRV_HANDLE hBmp, s32 bit)
{
	u32* pBmp = (u32*)hBmp;
	u32  val;
	s32  ind, subind;

    if (hBmp == SRV_INVALID)
    	return SRV_ERR;

    ind    = bit / SRV_BMP_BITS;
    subind = bit % SRV_BMP_BITS;

    val = (pBmp[ind]>>subind) & 0x1;

    return val;
}

s32 srv_bmp_set_bit(SRV_HANDLE hBmp, s32 bit, u32 val)
{
	u32* pBmp = (u32*)hBmp;
	s32  ind, subind;

    if (hBmp == SRV_INVALID)
    	return SRV_ERR;

    ind    = bit / SRV_BMP_BITS;
    subind = bit % SRV_BMP_BITS;

    if (val)
        pBmp[ind] |= 1 << subind;
    else
    	pBmp[ind] &= ~(1<<subind);

    return SRV_OK;
}
