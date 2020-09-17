/******************************************************************************
 * @file     USBLib.h
 * @brief    NuMicro series USB driver header file
 * @date     2017/04/26 09:30 a.m.
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __USBLIB_H__
#define __USBLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

void USBLib_Start(void);
void USBLib_Process(void);

#ifdef __cplusplus
}
#endif

#endif  /* __USBLIB_H__ */
