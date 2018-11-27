/**************************************************************************//**
 * @file        define.h
 * @version     V1.00
 * $Revision:   1$
 * $Date:       17/06/16 5:00p$
 * @brief       Loader (MTP), ISP, Firmware definition
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* Tag Definition */
#define TAG0_INDEX             0x00
#define END_TAG_OFFSET_INDEX   0x01
#define VER_INDEX              0x02
#define TAG1_INDEX             0x03
#define TAG_LEN                0x10
#define TAG0                   0xAA554257
#define TAG1                   0x63594257
#define END_TAG                0x4257AA55

/* Tag Setting */
#define ISP_TAG_OFFSET         0x00100          /*  256B */
#define ISP_ENDTAG_OFFSET      0x0F800          /*  62KB - The End Tag is added when Post-Build*/
#define FIRMWARE_TAG_OFFSET    0x00100          /*  256B */
#define FIRMWARE_ENDTAG_OFFSET 0x80000          /* 512KB - The End Tag is added when Post-Build*/

/* Execute Address */
#define FIRMWARE_CODE_ADDR     0x20000          /* SPI Flash Offset 128KB ; Must be tha sam as the App execute address */
#define ISP_CODE_ADDRESS    0x20000000          /* SRAM */

/* ISP Code address at SPI Flash */
#define ISP_CODE_OFFSET        0x10000          /* SPI Flash Offset 64KB */

/* Version Number */
#define ISP_VERSION         0x20170613
#define FIRMWARE_VERSION01  0x20170614
#define FIRMWARE_VERSION02  0x20170615

/* Update File Name Length (include Filename Extension) */
#define FILE_NAME_LENGTH    39

/* Update File Name (0:ignore) - Windows Only */
static uint8_t u8FileName[FILE_NAME_LENGTH] =
{
    'U', 'p', 'd', 'a', 't', 'e',  0,   0,    0,   0,
    0,   0,   0,   0,   0,   0,  0,   0,    0,   0,
    0,   0,   0,   0,   0,   0,  0,   0,    0,   0,
    0,   0,   0,   0,   0,   0,  0,   0,    0
};

/* SPI Flash Size */
#define SPIFLAHS_SIZE       0x200000        /* 2MB */

#ifdef __GNUC__
#define ISP_CODE_SIZE       0xA000
#else
#define ISP_CODE_SIZE       0x4800
#endif

/* MTP Setting */
/* If you want to encrypt binary firmware, you can enable
the session code and assign signature and offset here */
#define MTP_SIG     (0x5A5AA5A5)
#define MTP_OFFSET  (0x130)                 /* Less than 16KB */

/* Cipher function for ISP mode */
#define DISABLE_CIPHER

#ifdef __cplusplus
}
#endif
