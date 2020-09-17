/**************************************************************************//**
 * @file        Hardware.h
 * @version     V1.00
 * $Revision:   1$
 * $Date:       17/05/09 5:00p$
 * @brief       NUC505 Series USB Audio Class Sample Code header file
 *
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Note default debug port is UART0 defined in retarget.c, change to other UART port if needed. */
#define CONFIG_UART0    1
#define CONFIG_UART1    0

/* note set 0 for BootTemplate->Loader */
#define CONFIG_VECTOR   1

/* note set 1 to CONFIG_I2Cn and CONFIG_CODEC_EXTERNAL (defined in AudioLib.h) to use i2c to config CODEC */
#define CONFIG_I2C      0
#define CONFIG_I2C0     0
#define CONFIG_I2C1     0
/* note check CODEC spec to define properly device address and format */
#define DEVICE_ADDRESS          0x1A
#define CONFIG_I2C_RX_2BYTES    0
#define CONFIG_I2C_TX_4BYTES    0

void Hardware_Init(void);

void _I2C_SetTxCallback(void);
void _I2C_SetRxCallback(void);
void _I2C_WriteData(uint16_t addr, uint16_t data);
void _I2C_ReadData(uint16_t addr);

#ifdef __cplusplus
}
#endif

#endif  /* __HARDWARE_H__ */
