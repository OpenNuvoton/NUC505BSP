/****************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 12 $
 * $Date: 14/05/30 6:01p $
 * @brief    Demonstrate the RTC alarm mask function. This sample code sets a minute alarm mask
 *           after execution
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "NUC505Series.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

volatile int32_t   g_bAlarm  = FALSE;


/*---------------------------------------------------------------------------------------------------------*/
/* RTC Alarm Handle                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
void RTC_AlarmHandle(void)
{
    printf("   Alarm!!\n");
    g_bAlarm = TRUE;
}

/**
  * @brief  RTC ISR to handle interrupt event
  * @param  None
  * @retval None
  */
void RTC_IRQHandler(void)
{
    if ( (RTC->INTEN & RTC_INTEN_ALMIEN_Msk) && (RTC->INTSTS & RTC_INTSTS_ALMIF_Msk) )        /* alarm interrupt occurred */
    {
        RTC->INTSTS = 0x1;
        RTC_SyncReg();
        RTC_AlarmHandle();
    }
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable  XTAL */
    CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk;

    CLK_SetCoreClock(96000000);

    /* Set PCLK divider */
    CLK_SetModuleClock(PCLK_MODULE, (uint32_t)NULL, 1);

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

int32_t main(void)
{
    S_RTC_TIME_DATA_T sInitTime;
    S_RTC_TIME_DATA_T sCurTime;

    SYS_Init();
    UART0_Init();

    /* Time Setting */
    sInitTime.u32Year       = 2013;
    sInitTime.u32Month      = 10;
    sInitTime.u32Day        = 15;
    sInitTime.u32Hour       = 12;
    sInitTime.u32Minute     = 30;
    sInitTime.u32Second     = 0;
    sInitTime.u32DayOfWeek  = RTC_TUESDAY;
    sInitTime.u32TimeScale  = RTC_CLOCK_24;

    RTC_Open(&sInitTime);

    printf("\n RTC Alarm Mask Test (Alarm when %d/%02d/%02d %02d:xx:10)\n\n",sInitTime.u32Year,sInitTime.u32Month,
           sInitTime.u32Day,sInitTime.u32Hour);

    g_bAlarm = FALSE;

    /* Get the current time */
    RTC_GetDateAndTime(&sCurTime);

    printf("   Mask Minute alarm setting and Get alarm after 10 seconds\n");

    printf("     Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32Month,
           sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);

    /* The Mask alarm time setting for Minute */
    sCurTime.u32Minute = RTC_ALARM_MASK;

    /* The alarm time setting */
    sCurTime.u32Second = 10;

    /* Set the alarm time */
    RTC_SetAlarmDateAndTime(&sCurTime);

    /* Enable RTC Alarm Interrupt */
    RTC_EnableInt(RTC_INTEN_ALMIEN_Msk);
    NVIC_EnableIRQ(RTC_IRQn);

    while(!g_bAlarm);

    /* Get the current time */
    RTC_GetDateAndTime(&sCurTime);

    printf("     Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32Month,
           sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);

    g_bAlarm = FALSE;

    sCurTime.u32Minute     = 34;
    sCurTime.u32Second     = 55;

    printf("\n\n   Change Current Time and Get alarm after 15 seconds\n");

    printf("     Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32Month,
           sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);
    RTC_SetDateAndTime(&sCurTime);

    while(!g_bAlarm);

    /* Get the current time */
    RTC_GetDateAndTime(&sCurTime);

    printf("     Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32Month,
           sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);

    /* Disable RTC Alarm Interrupt */
    RTC_DisableInt(RTC_INTEN_ALMIEN_Msk);
    NVIC_DisableIRQ(RTC_IRQn);

    printf("\n RTC Alarm Mask Test End !!\n");

    while(1);
}

/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/



