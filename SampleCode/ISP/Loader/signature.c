/**************************************************************************//**
 * @file        main.c
 * @version     V1.00
 * $Revision:   1$
 * $Date:       17/06/16 5:00p$
 * @brief       Loader to load ISP code and execute firmware (include MTP)
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "NUC505Series.h"
#include "spiflash_drv.h"
#include "define.h"

#ifdef __ICCARM__
/* IAR */
const uint32_t gu32MtpAddr @ "mtpsig";
const uint32_t gu32MtpAddr = MTP_SIG;
#elif defined __GNUC__
/* MTP_OFFSET is defined in script file - section "mtpsig"  */
const uint32_t gu32MtpAddr[1] __attribute__((section(".mtpsig"))) = {MTP_SIG};
#else
const uint32_t gu32MtpAddr[1] __attribute__((at(MTP_OFFSET)))  = {MTP_SIG};
#endif


