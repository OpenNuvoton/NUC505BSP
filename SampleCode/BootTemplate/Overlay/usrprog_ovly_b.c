/**************************************************************************//**
 * @file        ovly_b.c
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
__attribute__((section("overlay_b"))) void bill(int a)
{
    printf("%s(%d) get called.\n", __func__, a);
}

__attribute__((section("overlay_b"))) void betty(int a)
{
    printf("%s(%d) get called.\n", __func__, a);
}
#elif defined (__ICCARM__)
#pragma default_function_attributes = @ "overlay_b"

void bill(int a)
{
    printf("%s(%d) get called.\n", __func__, a);
}

void betty(int a)
{
    printf("%s(%d) get called.\n", __func__, a);
}

#pragma default_function_attributes =
#endif
