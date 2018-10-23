/**************************************************************************//**
 * @file        dmm_tst.c
 * @version     V1.00
 * $Revision:   1$
 * $Date:       14/07/10 5:00p$
 * @brief       Do SPIM DMM mode test.
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NUC505Series.h"
#include "spiflash_drv.h"

#define TEST_IP         "SPIM"
#define TEST_ITEM       "DMM Test"

/* Through SPIM, user can choose to access embedded or externally attached SPI Flash.
   To access embedded SPI Flash, specify SPIM_CTL1_IFSEL_INTERN.
   To access externally attached SPI Flash, specify SPIM_CTL1_IFSEL_EXTERN and configure related multi-function pins. */
#define SPIM_IF         SPIM_CTL1_IFSEL_INTERN
//#define SPIM_IF         SPIM_CTL1_IFSEL_EXTERN

#define SPI_BUS_CLK     60000000

#define TSTBUF_SIZE     (8 * 1024)

static int32_t g_i32HasErr = 0;
#if defined ( __CC_ARM )
static __align(4096) uint8_t g_au8TstBuf[TSTBUF_SIZE];
static __align(4096) uint8_t g_au8TstBuf2[TSTBUF_SIZE];
#elif defined (__ICCARM__)
#pragma data_alignment=4096
static __no_init uint8_t g_au8TstBuf[TSTBUF_SIZE];
#pragma data_alignment=4096
static __no_init uint8_t g_au8TstBuf2[TSTBUF_SIZE];
#elif defined (__GNUC__)
static uint8_t g_au8TstBuf[TSTBUF_SIZE] __attribute__ ((aligned(4096)));
static uint8_t g_au8TstBuf2[TSTBUF_SIZE]__attribute__ ((aligned(4096)));;
#endif


static const uint32_t s_au32Patterns[] = {
    0x00000000, 0xFFFFFFFF, 0x55aa55aa, 0xaa55aa55, 0x33cc33cc, 0xcc33cc33
    //0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666
    //0x11111111, 0x22222222, 0x33333333, 0x44444444
    //0x00112233, 0x44556677, 0x8899AABB, 0xCCDDEEFF
};
    
static void PopData(uint8_t *pu8Buf, uint32_t u32BufSize)
{
    uint8_t *bufInd = pu8Buf;
    uint32_t u32BufRmn = u32BufSize;

    while (u32BufRmn) {
        uint32_t u32NTx = sizeof (s_au32Patterns);
        if (u32NTx > u32BufRmn) {
            u32NTx = u32BufRmn;
        }
        memcpy(bufInd, (uint8_t *) s_au32Patterns, u32NTx);
        bufInd += u32NTx;
        u32BufRmn -= u32NTx;
    }
}

#define _UNVMP(x)       (((uint32_t) (x)) < 0x20000000 ? ((uint8_t *) (SYS->LVMPADDR + ((uint32_t) (x)))) : (x))
#define MAPADDR         (128 * 1024)

#undef TEST_ITEM
#define TEST_ITEM       "DMM 1-bit Test"
void DoDMMSnglTest()
{
    do {
        // Erase and check if erased data are all 0xFF.
        SPIFlash_EraseAddrRange(MAPADDR, TSTBUF_SIZE);
        memset(g_au8TstBuf, 0xFF, TSTBUF_SIZE);
        SPIFlash_ReadData(MAPADDR, TSTBUF_SIZE, g_au8TstBuf2);
        if (memcmp(g_au8TstBuf, g_au8TstBuf2, TSTBUF_SIZE)) {
            printf("[%s][%s]Erase\t\tFAILED\n", TEST_IP, TEST_ITEM);
            g_i32HasErr = 1;
            break;
        }
        
        PopData(g_au8TstBuf, TSTBUF_SIZE);                                      // Populate data.
        // Write via DMA Write mode.
        SPIFlash_DMAWrite(SPIM, MAPADDR, 0, TSTBUF_SIZE, _UNVMP(g_au8TstBuf), SPIM_CTL0_CMDCODE_PAGE_PROG);
       
        SPIM_ENABLE_DMM_MODE(SPIM, SPIM_CTL0_CMDCODE_FAST_READ, 0);              // Switch to DMM mode.
                
        // Compare.
        if (memcmp(g_au8TstBuf, (void *) MAPADDR, TSTBUF_SIZE)) {
            printf("[%s][%s]Compare data\t\tFAILED\n", TEST_IP, TEST_ITEM);
            g_i32HasErr = 1;
            break;
        }
    }
    while (0);
    
    /* SPIM H/W may be still in operation due to in DMM mode, wait according to spec and then switch back. */
    CLK_SysTickDelay(250 * 1000000 / SPIM_GetBusClock(SPIM));       // Wait for at least 250 peripheral cycles (SPI bus cycles).
    SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_STAN, 1);           // Switch back to I/O, 1-bit output mode.
}

#undef TEST_ITEM
#define TEST_ITEM       "DMM 4-bit Test"
void DoDMMQuadTest()
{
    uint32_t u32JedecID = SPIFlash_ReadJedecID();
    uint8_t u8MID = u32JedecID & 0x000000FF;
    
    do {
        uint32_t u32QuadWriteCmdCode;
        
        // Erase and check if erased data are all 0xFF.
        SPIFlash_EraseAddrRange(MAPADDR, TSTBUF_SIZE);
        memset(g_au8TstBuf, 0xFF, TSTBUF_SIZE);
        SPIFlash_ReadData(MAPADDR, TSTBUF_SIZE, g_au8TstBuf2);
        if (memcmp(g_au8TstBuf, g_au8TstBuf2, TSTBUF_SIZE)) {
            printf("[%s][%s]Erase\t\tFAILED\n", TEST_IP, TEST_ITEM);
            g_i32HasErr = 1;
            break;
        }
        
        /* Enable Quad bit or QPI mode */
        if (u8MID == MFGID_WINBOND) {
            SPIFlash_W25Q_SetQuadEnable(1);
            u32QuadWriteCmdCode = SPIM_CTL0_CMDCODE_QUAD_PAGE_PROG_TYPE1;
        }
        else if (u8MID == MFGID_MXIC) {
            SPIFlash_MX25_SetQuadEnable(1);
            u32QuadWriteCmdCode = SPIM_CTL0_CMDCODE_QUAD_PAGE_PROG_TYPE2;
        }
        else if (u8MID == MFGID_EON) {
            SPIFlash_EN25Q_SetQPIMode(1);
            u32QuadWriteCmdCode = SPIM_CTL0_CMDCODE_QUAD_PAGE_PROG_TYPE3;
        }
        else if (u8MID == MFGID_ISSI) {
            SPIFlash_ISSI_SetQuadEnable(1);
            u32QuadWriteCmdCode = SPIM_CTL0_CMDCODE_QUAD_PAGE_PROG_TYPE1;
        }
        else {
            printf("[%s][%s]MID: 0x%02X not support\n", TEST_IP, TEST_ITEM, u8MID);
            g_i32HasErr = 1;
            break;
        }
        
        PopData(g_au8TstBuf, TSTBUF_SIZE);                                      // Populate data.
        // Write via DMA Write mode.
        SPIFlash_DMAWrite(SPIM, MAPADDR, 0, TSTBUF_SIZE, _UNVMP(g_au8TstBuf), u32QuadWriteCmdCode);
        
        /* Disable Quad bit or QPI mode */
        if (u8MID == MFGID_EON) {
            SPIFlash_EN25Q_SetQPIMode(0);
        }
        
        SPIM_ENABLE_DMM_MODE(SPIM, SPIM_CTL0_CMDCODE_FAST_READ_QUAD_IO, 0);     // Switch to DMM mode.
                
        // Compare.
        if (memcmp(g_au8TstBuf, (void *) MAPADDR, TSTBUF_SIZE)) {
            printf("[%s][%s]Compare data\tFAILED\n", TEST_IP, TEST_ITEM);
            g_i32HasErr = 1;
            break;
        }
    }
    while (0);
    
    /* SPIM H/W may be still in operation due to in DMM mode, wait according to spec and then switch back. */
    CLK_SysTickDelay(250 * 1000000 / SPIM_GetBusClock(SPIM));       // Wait for at least 250 peripheral cycles (SPI bus cycles).
    SPIM_ENABLE_IO_MODE(SPIM, SPIM_CTL0_BITMODE_STAN, 1);           // Switch back to I/O, 1-bit output mode.
    
    /* Disable Quad bit or QPI mode */
    if (u8MID == MFGID_WINBOND) {
        SPIFlash_W25Q_SetQuadEnable(0);
    }
    else if (u8MID == MFGID_MXIC) {
        SPIFlash_MX25_SetQuadEnable(0);
    }
    else if (u8MID == MFGID_ISSI) {
        SPIFlash_ISSI_SetQuadEnable(0);
    }
}

#undef TEST_ITEM
#define TEST_ITEM       "DMM Test"
void TestDMMMode(void)
{   
    printf("[%s][%s]\t\t\tSTART\n", TEST_IP, TEST_ITEM);
    
    SPIM_Open(SPIM, 0, SPI_BUS_CLK);
    
#if (SPIM_IF == SPIM_CTL1_IFSEL_EXTERN)
    SPIM_SetIF(SPIM, SPIM_IF);
    
    /* Configure multi-function pins for SPIM, GPIO slave I/F. */
    SYS->GPA_MFPH = SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA8MFP_Msk | (1 << SYS_GPA_MFPH_PA8MFP_Pos);    // SPIM_SS
    SYS->GPA_MFPH = SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA9MFP_Msk | (1 << SYS_GPA_MFPH_PA9MFP_Pos);    // SPIM_SCLK
    SYS->GPA_MFPH = SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA10MFP_Msk | (1 << SYS_GPA_MFPH_PA10MFP_Pos);  // SPIM_D0
    SYS->GPA_MFPH = SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA11MFP_Msk | (1 << SYS_GPA_MFPH_PA11MFP_Pos);  // SPIM_D1
    SYS->GPA_MFPH = SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA12MFP_Msk | (1 << SYS_GPA_MFPH_PA12MFP_Pos);  // SPIM_D2
    SYS->GPA_MFPH = SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA13MFP_Msk | (1 << SYS_GPA_MFPH_PA13MFP_Pos);  // SPIM_D3
#endif  // #if (SPIM_IF == SPIM_CTL1_IFSEL_EXTERN)

    /* Actual SPI bus clock */
    printf("[%s][%s] SPI bus clock\t\t%dHz\n", TEST_IP, TEST_ITEM, SPIM_GetBusClock(SPIM));
    
    /* Vendor specific setting */
    {
        uint32_t u32JedecID = SPIFlash_ReadJedecID();
        uint8_t u8MID = u32JedecID & 0x000000FF;
        
        if (u8MID == MFGID_ISSI) {
            // Configure dummy cycles for ISSI read commands. Set P[6:3] to 0 to stand for default.
            uint8_t params = SPIFlash_ISSI_ReadReadParams();        
            params &= ~(BIT3 | BIT4 | BIT5 | BIT6);
            SPIFlash_ISSI_SetReadParamsV(params);
        }
    }
    
    DoDMMSnglTest();        // 1-bit Direct Memory Mapping test.
    DoDMMQuadTest();        // 4-bit Direct Memory Mapping test.

#if (SPIM_IF == SPIM_CTL1_IFSEL_EXTERN)
    /* Configure multi-function pins back for GPIO. */
    SYS->GPA_MFPH = SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA8MFP_Msk | (0 << SYS_GPA_MFPH_PA8MFP_Pos);    // GPIOA[8]
    SYS->GPA_MFPH = SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA9MFP_Msk | (0 << SYS_GPA_MFPH_PA9MFP_Pos);    // GPIOA[9]
    SYS->GPA_MFPH = SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA10MFP_Msk | (0 << SYS_GPA_MFPH_PA10MFP_Pos);  // GPIOA[10]
    SYS->GPA_MFPH = SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA11MFP_Msk | (0 << SYS_GPA_MFPH_PA11MFP_Pos);  // GPIOA[11]
    SYS->GPA_MFPH = SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA12MFP_Msk | (0 << SYS_GPA_MFPH_PA12MFP_Pos);  // GPIOA[12]
    SYS->GPA_MFPH = SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA13MFP_Msk | (0 << SYS_GPA_MFPH_PA13MFP_Pos);  // GPIOA[13]
#endif  // #if (SPIM_IF == SPIM_CTL1_IFSEL_EXTERN)

    SPIM_Close(SPIM);
    
    if (g_i32HasErr) {
        printf("[%s][%s]\t\t\tFAILED\n", TEST_IP, TEST_ITEM);
        //exit(1);
    }
    else {
        printf("[%s][%s]\t\t\tPASSED\n", TEST_IP, TEST_ITEM);
    }
}   






