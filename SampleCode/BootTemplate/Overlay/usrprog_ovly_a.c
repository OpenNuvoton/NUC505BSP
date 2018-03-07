/**************************************************************************//**
 * @file        ovly_a.c
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
#pragma arm section code="overlay_a"
#elif defined (__ICCARM__)
#pragma default_function_attributes = @ "overlay_a"
#endif

void aaron(void)
{
    printf("%s() get called.\n", __func__);
}

void andy(void)
{
    printf("%s() get called.\n", __func__);
}

#if defined ( __CC_ARM )
#pragma arm section code
#elif defined (__ICCARM__)
#pragma default_function_attributes =
#endif
