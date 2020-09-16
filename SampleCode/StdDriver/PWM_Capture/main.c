
/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 17 $
 * $Date: 14/05/30 6:01p $
 * @brief    Demonstrate PWM Capture function by using PWM channel 2
 *           to capture the output of PWM channel 0. Please connect
 *           PB.10 and PB.12 to execute this code
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "NUC505Series.h"

#define SAMPLE_CNT 32
extern uint8_t volatile cap_index;
extern uint32_t cap_val[SAMPLE_CNT >> 1][2];

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
    CLK_EnableModuleClock(PWM_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_UART0_SRC_EXT, 0);

    /* Set EXT as PWM channel 0~3 clock source */
    CLK_SetModuleClock(PWM_MODULE,CLK_PWM_SRC_EXT,0);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Configure multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB0MFP_Msk) ) | SYS_GPB_MFPL_PB0MFP_UART0_TXD;
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB1MFP_Msk) ) | SYS_GPB_MFPL_PB1MFP_UART0_RXD;

    /* Set GPB10~GPB13 multi-function pins for PWM Channel 0~3 */
    SYS->GPB_MFPH  = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB10MFP_Msk) ) | SYS_GPB_MFPH_PB10MFP_PWM_CH0;
    SYS->GPB_MFPH  = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB11MFP_Msk) ) | SYS_GPB_MFPH_PB11MFP_PWM_CH1;
    SYS->GPB_MFPH  = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB12MFP_Msk) ) | SYS_GPB_MFPH_PB12MFP_PWM_CH2;
    SYS->GPB_MFPH  = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB13MFP_Msk) ) | SYS_GPB_MFPH_PB13MFP_PWM_CH3;

}

int32_t main (void)
{
    uint8_t i;

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

    printf("\nPWM channel 2 will capture the output of PWM channel 0\n");
    printf("So, please connect GPIO port B10 and B12.\n");
    // PWM frequency is 25000Hz, duty 30%,
    PWM_ConfigOutputChannel(PWM, 0, 25000, 30);
    PWM_EnableDeadZone(PWM, 0, 10);

    // PWM2
    PWM_ConfigCaptureChannel(PWM,2,50,0);

    // Enable output of channel 0
    PWM_EnableOutput(PWM, PWM_CH_0_MASK);

    // Enable capture of channel 2
    PWM_EnableCapture(PWM, PWM_CH_2_MASK);

    // Enable PWM channel 2 rising and falling edge capture interrupt
    PWM_EnableCaptureInt(PWM,2,PWM_RISING_FALLING_LATCH_INT_ENABLE);
    NVIC_EnableIRQ(PWM2_IRQn);

    // Start
    PWM_Start(PWM, PWM_CH_2_MASK);
    PWM_Start(PWM, PWM_CH_0_MASK);

    while(cap_index < SAMPLE_CNT);

    // Stop
    PWM_Stop (PWM, (PWM_CH_0_MASK|PWM_CH_2_MASK));

    printf("Captured data is as below.\n");
    printf("(rising : falling)\n");
    for(i = 1; i < (SAMPLE_CNT  >> 1); i++)    // ignore first sampled data. it's wrong
    {
        printf("%d, %d : %d\n", i, cap_val[i][0], cap_val[i][1]);
    }

    while(1);

}

/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/


