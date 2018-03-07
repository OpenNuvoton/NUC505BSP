/**************************************************************************//**
 * @file        ovly_e.c
 * @version     V1.00
 * $Revision:   1$
 * $Date:       14/07/10 5:00p$
 * @brief       User program which is located in overlay
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>

#if defined ( __CC_ARM )
#pragma arm section code="overlay_e"
#elif defined (__ICCARM__)
#pragma default_function_attributes = @ "overlay_e"
#endif

int edgar(int a, int b, int c)
{
    int retval = a + b + c;
    printf("%s(%d, %d, %d)=%d get called.\n", __func__, a, b, c, retval);
    return retval;
}

#if defined ( __CC_ARM )
#pragma arm section code
#elif defined (__ICCARM__)
#pragma default_function_attributes =
#endif
