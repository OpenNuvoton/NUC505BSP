/**************************************************************************//**
 * @file        ovly_a.c
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
__attribute__((section("overlay_a"))) void aaron(void)
{
    printf("%s() get called.\n", __func__);
}

__attribute__((section("overlay_a"))) void andy(void)
{
    printf("%s() get called.\n", __func__);
}
#elif defined (__ICCARM__)
#pragma default_function_attributes = @ "overlay_a"
void aaron(void)
{
    printf("%s() get called.\n", __func__);
}

void andy(void)
{
    printf("%s() get called.\n", __func__);
}
#pragma default_function_attributes =
#endif
