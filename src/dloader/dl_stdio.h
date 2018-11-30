/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _FDL_STDIO_H_
#define _FDL_STDIO_H_

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void*)0)
#endif /* __cplusplus */
#endif /* NULL */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************
 * FDL_memcpy
 ******************************************************************************/
void *FDL_memcpy (void *dst, const void *src, unsigned int count);

/******************************************************************************
 * FDL_memset
 ******************************************************************************/
void *FDL_memset (void *dst, int c, unsigned int count);
unsigned short EndianConv_16 (unsigned short value);
unsigned int EndianConv_32 (unsigned int value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* FDL_STDIO_H */

