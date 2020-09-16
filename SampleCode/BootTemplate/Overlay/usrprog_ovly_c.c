/**************************************************************************//**
 * @file        ovly_c.c
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

#if defined ( __CC_ARM )
#pragma arm section code="overlay_c"
#elif defined (__ICCARM__)
#pragma default_function_attributes = @ "overlay_c"
#endif

int chris(int a)
{
    int retval = a;
    printf("%s(%d)=%d get called.\n", __func__, a, retval);
    return retval;
}

#if defined ( __CC_ARM )
#pragma arm section code
#elif defined (__ICCARM__)
#pragma default_function_attributes =
#endif
