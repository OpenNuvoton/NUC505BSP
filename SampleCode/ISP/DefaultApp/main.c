/****************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/06/15 6:02p $
 * @brief    ISP demo code for test
 *
 * @note
 * Copyright (C) 2013 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "NUC505Series.h"
#include "define.h"

#ifdef __ICCARM__
/* IAR */
static const uint32_t gu32TAG[4] @ (FIRMWARE_CODE_ADDR + FIRMWARE_TAG_OFFSET) = {TAG0,FIRMWARE_ENDTAG_OFFSET,FIRMWARE_VERSION01,TAG1};
#else
/* Keil */ 
__attribute__((at(FIRMWARE_CODE_ADDR + FIRMWARE_TAG_OFFSET))) static const unsigned int gu32TAG[4] = {TAG0,FIRMWARE_ENDTAG_OFFSET,FIRMWARE_VERSION01,TAG1};   
#endif

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable  XTAL */
    CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk;

    CLK_SetCoreClock(96000000);
	
    /* Set PCLK divider */
    CLK_SetModuleClock(PCLK_MODULE, NULL, 1);
	
    /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Enable IP clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_UART0_SRC_EXT, 0);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Configure multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB0MFP_Msk) ) | SYS_GPB_MFPL_PB0MFP_UART0_TXD;
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB1MFP_Msk) ) | SYS_GPB_MFPL_PB1MFP_UART0_RXD;

}

void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART module */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}

void EnterISPmode(void)
{
    SYS->BOOTSET = 0x0E;
    SYS->LVMPADDR = 0x0;
    SYS->LVMPLEN = 0x01;
    SYS->RVMPLEN = 0x01;
	
    SYS->IPRST0 = SYS_IPRST0_CPURST_Msk;
    __NOP();
    __NOP();
}

/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{		
    SYS_Init();
    UART0_Init();

    printf("+----------------------------------------+\n");
    printf("|         NUC505 ISP Sample Code         |\n");
    printf("|              APP Default               |\n"); 
    printf("+----------------------------------------+\n");
    printf("Version %08X\n",gu32TAG[VER_INDEX]);
    printf("Press any key to ISP mode\n");
    getchar();
    EnterISPmode();
}
/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/



