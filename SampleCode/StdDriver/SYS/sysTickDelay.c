/**************************************************************************//**
 * @file     sysTickDelay.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 14/07/24 4:08p $
 * @brief    NUC505 Series Global Control and Clock Control Driver Sample Code
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "NUC505Series.h"
void demo_SysTickDelay(void)
{
    CLK_SysTickDelay(10000);
    printf("Waiting for 10 ms\n");
    CLK_SysTickDelay(30000);
    printf("Waiting for 30 ms\n");
}
