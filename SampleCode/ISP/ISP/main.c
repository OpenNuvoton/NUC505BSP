/**************************************************************************//**
 * @file        main.c
 * @version     V1.00
 * $Revision:   1$
 * $Date:       17/06/16 5:00p$
 * @brief       Demonstrate how to locate a program fully on SRAM.
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "NUC505Series.h"
#include "massstorage.h"
#include "spiflash_drv.h"
#include "define.h"

#ifdef __ICCARM__
const uint32_t gu32TAG[4] @ (ISP_TAG_OFFSET) = {TAG0,ISP_ENDTAG_OFFSET,ISP_VERSION,TAG1};
#elif defined __GNUC__
/* ISP_TAG_OFFSET is defined in script file - section "mtpsig"  */
const uint32_t gu32TAG[4] __attribute__ ((section(".mtpsig"))) = {TAG0,ISP_ENDTAG_OFFSET,ISP_VERSION,TAG1};
#else
const unsigned int gu32TAG[4] __attribute__((at(ISP_TAG_OFFSET))) = {TAG0,ISP_ENDTAG_OFFSET,ISP_VERSION,TAG1};
#endif

#define SPIM_IF         SPIM_CTL1_IFSEL_INTERN
#define SPI_BUS_CLK     60000000

extern uint8_t g_u8UpdateDone;
#ifdef __ICCARM__
#pragma data_alignment=4
extern  uint8_t u8RootDirData[]; 
#else
extern uint8_t u8RootDirData[] __attribute__((aligned(4)));
#endif
 
uint32_t g_au32Buf[8];
uint32_t *g_pu32Buf;
uint32_t EndTagAddr = 0;
uint32_t EndTag = 0;

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

    /* Enable USB IP clock */
    CLK_EnableModuleClock(USBD_MODULE);

    /* Select USB IP clock source */
    CLK_SetModuleClock(USBD_MODULE, CLK_USBD_SRC_EXT, 0);

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

int main(void)
{
    uint32_t i =0;
    uint32_t u32Version = 0;
    int8_t i8tmp = 0, No_Firmware = 1;

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();
    
    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);  
    
    printf("ISP - %08X\n",gu32TAG[VER_INDEX]);
    
    SPIM_Open(SPIM, 0, SPI_BUS_CLK);	

    memset((char *)g_au32Buf, 0, TAG_LEN);

    /* Check Update Firmware Header */
    SPIFlash_ReadData(FIRMWARE_CODE_ADDR + FIRMWARE_TAG_OFFSET, TAG_LEN, (uint8_t *)g_au32Buf);

#ifdef DISABLE_CIPHER	
    /* Disable Cipher */
    SPIM->CTL0 = SPIM->CTL0 | SPIM_CTL0_CIPHOFF_Msk;
#endif
    /* Check App Firmware Header Tag */
    if ((g_au32Buf[TAG0_INDEX] == TAG0) && (g_au32Buf[TAG1_INDEX] == TAG1))
    {
        u32Version =  g_au32Buf[VER_INDEX];	

        EndTagAddr = FIRMWARE_CODE_ADDR + g_au32Buf[END_TAG_OFFSET_INDEX];

        SPIFlash_ReadData(EndTagAddr, 4, (uint8_t *)&EndTag);	/* Read End Tag */
       
        if(EndTag == END_TAG)    
        {
            /* Set MSC Label Name */
            for(i=0;i<8;i++)
            {
                i8tmp = (u32Version >> ((7-i) * 4) & 0x0F);

                if((i8tmp >= 0x0) && (i8tmp <= 0x9))
                    u8RootDirData[96+i] = '0' + i8tmp;
                else if((i8tmp >= 0xA) && (i8tmp <= 0xF))
                    u8RootDirData[96+i] = 'A' + (i8tmp - 10);
                else
                    u8RootDirData[96+i] = i8tmp;
            }
            No_Firmware = 0;
        }
    }
    if(No_Firmware)
    {
        /* Set MSC Label Name */
        u8RootDirData[96] = 'N';
        u8RootDirData[97] = 'o';
        u8RootDirData[98] = ' ';
        u8RootDirData[99] = 'C';
        u8RootDirData[100] = 'o';
        u8RootDirData[101] = 'd';
        u8RootDirData[102] = 'e';
        u8RootDirData[103] = '\n';
    }
    USBD_Open(&gsInfo, MSC_ClassRequest, NULL);

    /* Endpoint configuration */
    MSC_Init();

    /* Enable USBD interrupt */
    NVIC_EnableIRQ(USBD_IRQn);

    /* Start transaction */
    while(1) {
        if (USBD_IS_ATTACHED()) {
            USBD_Start();
            break;
        }
    }

    while(1) {
        if (g_usbd_Configured)
        {
            MSC_ProcessCmd();

            if(g_u8UpdateDone)
            {
                /* Update Compltete */
                SYS->IPRST0 = SYS_IPRST0_CHIPRST_Msk;
                __NOP();
                __NOP();
            }
        }
    }
}

