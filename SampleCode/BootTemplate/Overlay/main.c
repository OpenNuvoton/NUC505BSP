/**************************************************************************//**
 * @file        main.c
 * @version     V1.00
 * $Revision:   1$
 * $Date:       14/07/10 5:00p$
 * @brief       Demonstrate how to use overlay to run a large program in small size SRAM, through which a large program is divided into smaller ones
 *              which are located in the same SRAM address for execution.
 *
 * @note        Define your overlay table both in usrprog_ovly_tab.c and the linker script file.
 *              A overlay table consists of overlays (overlaid programs) and overlay regions. In this sample, overlay table will be as follows:
 *              usrporg_ovly_a.c/usrporg_ovly_b.c in the same overlay region
 *              usrporg_ovly_c.c/usrporg_ovly_d.c/usrporg_ovly_e.c in the same overlay region
 *              usrporg_ovly_f.c in single overlay region
 * @note        Load/exec addresses of overlays are determined through the linker script file. These addresses can be acquired through
 *              linker-generated symbols. See DEFINE_OVERLAY in ovlymgr.h for how to access them.
 * @note        load_overlay() is responsible for loading overlay to overlay region through SPIM DMA Read mode or memcpy().
 *              All overlay manager code (ovlymgr.c) must be located in SRAM for running SPIM DMA Read mode.
 * @note        User must be responsible for calling load_overlay() to load overlay before its execution.
 * @note        Overlaid program cannot be source-level debugged.
 * @note        This sample just demonstrates how to overlay code. Data is not overlaid.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "NUC505Series.h"
#include "usrprog_ovly_tab.h"

void SYS_Init(void)
{

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    //SYS_UnlockReg();

    /* Enable  XTAL */
    CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk;

    /* Enable IP clock */
    CLK_EnableModuleClock(UART0_MODULE);
    //CLK->APBCLK = CLK_APBCLK_UART0CKEN_Msk;     // Enable UART0 IP clock.

    /* Select IP clock source */
    /* PCLK divider = 1 (/2) */
    CLK_SetModuleClock(PCLK_MODULE, 0, 1);
    /* UART0 clock source = XIN */
    CLK_SetModuleClock(UART0_MODULE, CLK_UART0_SRC_EXT, 0);
    //CLK->CLKDIV3 &= ~(CLK_CLKDIV3_UART0DIV_Msk | CLK_CLKDIV3_UART0SEL_Msk);

    /* Update System Core Clock */
    /* Note too high system clock will cause over-spec of SPI Flash read command on running code on SPI Flash. */
    CLK_SetCoreClock(100000000);
    SystemCoreClockUpdate();

    /* Init I/O multi-function pins */
    /* Configure multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB0MFP_Msk) ) | SYS_GPB_MFPL_PB0MFP_UART0_TXD;
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB1MFP_Msk) ) | SYS_GPB_MFPL_PB1MFP_UART0_RXD;

    /* Lock protected registers */
    //SYS_LockReg();

}

void aaron(void);
void andy(void);
void bill(int a);
void betty(int a);
int chris(int a);
int dick(int a, int b);
int dot(int a, int b);
int edgar(int a, int b, int c);
int fran(int a, int b, int c, int d);

int main(void)
{

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

    printf("+------------------------------------------------+\n");
    printf("|           NUC505 Series Demo Sample            |\n");
    printf("+------------------------------------------------+\n");

    /* Relocate vector table in SRAM for fast interrupt handling. */
    {
#if defined ( __CC_ARM )
        extern uint32_t __Vectors[];
        extern uint32_t __Vectors_Size[];
        extern uint32_t Image$$ER_VECTOR2$$ZI$$Base[];

        printf("Relocate vector table in SRAM (0x%08X) for fast interrupt handling.\n", Image$$ER_VECTOR2$$ZI$$Base);
        memcpy((void *) Image$$ER_VECTOR2$$ZI$$Base, (void *) __Vectors, (unsigned int) __Vectors_Size);
        SCB->VTOR = (uint32_t) Image$$ER_VECTOR2$$ZI$$Base;

#elif defined (__ICCARM__)
#pragma section = "VECTOR2"
        extern uint32_t __Vectors[];
        extern uint32_t __Vectors_Size[];

        printf("Relocate vector table in SRAM (0x%08X) for fast interrupt handling.\n", __section_begin("VECTOR2"));
        memcpy((void *) __section_begin("VECTOR2"), (void *) __Vectors, (unsigned int) __Vectors_Size);
        SCB->VTOR = (uint32_t) __section_begin("VECTOR2");

#endif
    }

    load_overlay(ovly_A);
    aaron();
    andy();

    load_overlay(ovly_B);
    bill(1);
    betty(2);

    load_overlay(ovly_C);
    chris(3);

    load_overlay(ovly_D);
    dick(4, 5);
    dot(6, 7);

    load_overlay(ovly_E);
    edgar(8, 9, 10);

    load_overlay(ovly_F);
    fran(11, 12, 13, 14);

    while (1);
    //return 0;
}


