/******************************************************************************
 * @file     SDGlue.c
 * @version  V1.01
 * $Revision: 5 $
 * $Date: 14/11/27 3:18p $
 * @brief    SD glue functions for FATFS
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NUC505Series.h"
#include "diskio.h"     /* FatFs lower layer API */

extern SD_INFO_T SD0;

extern int sd0_ok;
extern int sd1_ok;

extern uint8_t pSD0_offset;
extern uint8_t pSD1_offset;

extern DISK_DATA_T SD_DiskInfo0;
extern DISK_DATA_T SD_DiskInfo1;

int SD_Open_(uint32_t cardSel);
void SD_Close_(uint32_t cardSel);

int SD_Open_(uint32_t cardSel)
{
    SD_Open(cardSel);
    SD_Probe(cardSel & 0x00ff);

    return SD_GET_CARD_CAPACITY(SD_PORT0);
}

void SD_Close_(uint32_t cardSel)
{
    if (cardSel == 0)
    {
        sd0_ok = 0;
        memset(&SD0, 0, sizeof(SD_INFO_T));
    }
}

