/**************************************************************************//**
 * @file        ovly_d.c
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
__attribute__((section("overlay_d"))) int dick(int a, int b)
{
    int retval = a + b;
    printf("%s(%d, %d)=%d get called.\n", __func__, a, b, retval);
    return retval;
}

__attribute__((section("overlay_d"))) int dot(int a, int b)
{
    int retval = a + b;
    printf("%s(%d, %d)=%d get called.\n", __func__, a, b, retval);
    return retval;
}
#elif defined (__ICCARM__)
#pragma default_function_attributes = @ "overlay_d"
int dick(int a, int b)
{
    int retval = a + b;
    printf("%s(%d, %d)=%d get called.\n", __func__, a, b, retval);
    return retval;
}

int dot(int a, int b)
{
    int retval = a + b;
    printf("%s(%d, %d)=%d get called.\n", __func__, a, b, retval);
    return retval;
}
#pragma default_function_attributes =
#endif
