/**************************************************************************//**
 * @file     MP3Func.c
 * @brief    Fixed-point data to signed integer
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "fixed.h"

//------------------------------------------------------------------------------
// Function Name  : MP3FixedToShort
// Description    : MP3FixedToShort
// Input          : None
// Output         : None
// Return         : None
//------------------------------------------------------------------------------
signed int MP3FixedToShort(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)	
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}
