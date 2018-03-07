/**************************************************************************//**
 * @file        ovly_b.c
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
#pragma arm section code="overlay_b"
#elif defined (__ICCARM__)
#pragma default_function_attributes = @ "overlay_b"
#endif

void bill(int a)
{
    printf("%s(%d) get called.\n", __func__, a);
}

void betty(int a)
{
    printf("%s(%d) get called.\n", __func__, a);
}

#if defined ( __CC_ARM )
#pragma arm section code
#elif defined (__ICCARM__)
#pragma default_function_attributes =
#endif
