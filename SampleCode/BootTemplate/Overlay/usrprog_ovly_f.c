/**************************************************************************//**
 * @file        ovly_f.c
 * @version     V1.00
 * $Revision:   1$
 * $Date:       14/07/10 5:00p$
 * @brief       User program which is located in overlay
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>

#if defined ( __ARMCC_VERSION )
#pragma section = "overlay_f"
__attribute__((section("overlay_f"))) int fran(int a, int b, int c, int d)
{
    int retval = a + b + c + d;
    printf("%s(%d, %d, %d, %d)=%d get called.\n", __func__, a, b, c, d, retval);
    return retval;
}
#elif defined (__ICCARM__)
#pragma default_function_attributes = @ "overlay_f"
int fran(int a, int b, int c, int d)
{
    int retval = a + b + c + d;
    printf("%s(%d, %d, %d, %d)=%d get called.\n", __func__, a, b, c, d, retval);
    return retval;
}
#pragma default_function_attributes =
#endif
