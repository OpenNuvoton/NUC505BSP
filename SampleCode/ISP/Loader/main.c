/**************************************************************************//**
 * @file        main.c
 * @version     V1.00
 * $Revision:   1$
 * $Date:       17/06/16 5:00p$
 * @brief       Loader to load ISP code and execute firmware (include MTP)
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "NUC505Series.h"
#include "spiflash_drv.h"
#include "define.h"

#define SPIM_IF         SPIM_CTL1_IFSEL_INTERN
#define SPI_BUS_CLK     60000000

uint32_t g_au32Buf[8];
uint32_t *g_pu32Buf;
uint8_t g_u8ISP = 1; 

#ifdef __ICCARM__
/* IAR */
extern const uint32_t gu32MtpAddr;
#elif defined __GNUC__
/* MTP_OFFSET is defined in script file - section "mtpsig"  */
extern const uint32_t gu32MtpAddr[1] __attribute__ ((section(".mtpsig")));
#else
extern const uint32_t gu32MtpAddr[1] __attribute__((at(MTP_OFFSET)));
#endif

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

    /* Select IP clock source */
    /* PCLK divider = 1 (/2) */
    CLK_SetModuleClock(PCLK_MODULE, 0, 1);

    /* UART0 clock source = XIN */
    CLK_SetModuleClock(UART0_MODULE, CLK_UART0_SRC_EXT, 0);
    
    /* Update System Core Clock */
    CLK_SetCoreClock(100000000);
    
    SystemCoreClockUpdate();
    
    /* Init I/O multi-function pins */
    /* Configure multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB0MFP_Msk) ) | SYS_GPB_MFPL_PB0MFP_UART0_TXD;
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB1MFP_Msk) ) | SYS_GPB_MFPL_PB1MFP_UART0_RXD;
    /* Configure multi-function pins for SPIM, Slave I/F=GPIO. */       
}

#if defined (__CC_ARM)

__asm static void start_new_application(void *sp, void *pc)
{
    MOV R2, #0
    MSR CONTROL, R2         // Switch to main stack
    MOV SP, R0
    MSR PRIMASK, R2         // Enable interrupts
    BX R1
}

#elif defined (__GNUC__) || defined (__ICCARM__)

void start_new_application(void *sp, void *pc)
{
    __asm volatile (
        "mov    r2, #0      \n"
        "msr    control, r2 \n" // Switch to main stack
        "mov    sp, %0      \n"
        "msr    primask, r2 \n" // Enable interrupts
        "bx     %1          \n"
        :
        : "l" (sp), "l" (pc)
        : "r2", "cc", "memory"
    );
}

#else

#error "Unsupported toolchain"

#endif

void EnterISP(void)
{
    uint32_t u32Version = 0;

    CLK_EnableModuleClock(SPIM_MODULE);

    SPIM_Open(SPIM, 0, SPI_BUS_CLK);

    memset((char *)g_au32Buf, 0, 32);
    
    /* Check Update Firmware Header */
    SPIFlash_ReadData(ISP_CODE_OFFSET + ISP_TAG_OFFSET, TAG_LEN, (uint8_t *)g_au32Buf);

    if ((g_au32Buf[TAG0_INDEX] == TAG0) && (g_au32Buf[TAG1_INDEX] == TAG1))  /* Check ISP Firmware Header Tag 0 & 1 */
    {
        SPIFlash_ReadData(ISP_CODE_OFFSET + g_au32Buf[END_TAG_OFFSET_INDEX], 4, (uint8_t *)g_au32Buf);	/* Read End Tag */

        if(g_au32Buf[0] != END_TAG)
            printf("Update failed last time!\n");
        else
        {
            u32Version =  g_au32Buf[VER_INDEX];
            printf("ISP\n");
            printf("  ISP FW Version : 0x%08X\n", u32Version);

            /* Loader ISP Firmware */
            SPIFlash_ReadData(ISP_CODE_OFFSET, ISP_CODE_SIZE, (uint8_t *)ISP_CODE_ADDRESS);
            SYS->LVMPADDR = ISP_CODE_ADDRESS;
            SYS->LVMPLEN = 0x80;
            SYS->RVMPLEN = 0x01;
            SYS->IPRST0 = SYS_IPRST0_CPURST_Msk;
            __NOP();
            __NOP();
        }
    }
    else
    {
        printf("No ISP Firmware!!\n");	
        g_u8ISP = 0;
    }
}

int main(void) 
{
#ifdef __ICCARM__
    uint32_t volatile u32Data = gu32MtpAddr;
#else
    uint32_t volatile u32Data = gu32MtpAddr[0];
#endif    

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();
    
    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

    printf("NUC505 Loader\n");

    /* Check Update Operation Condition - Firwmare set BOOTSET to 0xE and reset CPU to Enter ISP mode */
    if((SYS->BOOTSET & 0xF) == 0xE)    /* SYS->BOOTSET can't be modified when ICE mode */
        EnterISP();

    g_pu32Buf = (uint32_t *)(FIRMWARE_CODE_ADDR + FIRMWARE_TAG_OFFSET);

    /* Check Firmware Tag */
    if ((g_pu32Buf[TAG0_INDEX] == TAG0) && (g_pu32Buf[TAG1_INDEX] == TAG1))
    {
        g_pu32Buf = (uint32_t *)(FIRMWARE_CODE_ADDR + g_pu32Buf[END_TAG_OFFSET_INDEX]);    /* Read End Tag */

        if(*g_pu32Buf != END_TAG)
            printf("Update failed last time!\n");
        else
        {
            void *sp;
            void *pc;
            printf("Jump to Firmware!!\n");
            SYS->LVMPADDR = FIRMWARE_CODE_ADDR;
            SYS->LVMPLEN = 0x01;
            SYS->RVMPLEN = 0x01;
            sp = *((void**)FIRMWARE_CODE_ADDR + 0);
            pc = *((void**)FIRMWARE_CODE_ADDR + 1);
            start_new_application(sp, pc);
        }
    }
    printf("No Firmware!!\n");

    if(g_u8ISP)
        EnterISP();    /* Execute ISP */

    while (1);
}




