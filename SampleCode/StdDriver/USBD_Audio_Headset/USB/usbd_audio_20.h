/******************************************************************************
 * @file     usbd_audio.h
 * @brief    NuMicro series USB driver header file
 * @date     2017/04/26 09:30 a.m.
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __USBD_UAC_H__
#define __USBD_UAC_H__

#include "demo.h"

/*!<Define Audio information */
#define PLAY_CHANNELS   2

#define REC_CHANNELS    2
#define REC_FEATURE_UNITID      0x05
#define PLAY_FEATURE_UNITID     0x06
#define CLOCK_SOURCE_ID       0x10

/* Define Descriptor information */
#if(PLAY_CHANNELS == 1)
#define PLAY_CH_CFG     1
#endif
#if(PLAY_CHANNELS == 2)
#define PLAY_CH_CFG     3
#endif

#if(REC_CHANNELS == 1)
#define REC_CH_CFG     1
#endif
#if(REC_CHANNELS == 2)
#define REC_CH_CFG     3
#endif

//#define PLAY_RATE_LO    (PLAY_RATE & 0xFF)
//#define PLAY_RATE_MD    ((PLAY_RATE >> 8) & 0xFF)
//#define PLAY_RATE_HI    ((PLAY_RATE >> 16) & 0xFF)

//#define REC_RATE_LO     (REC_RATE & 0xFF)
//#define REC_RATE_MD     ((REC_RATE >> 8) & 0xFF)
//#define REC_RATE_HI     ((REC_RATE >> 16) & 0xFF)

/***************************************************/
/*      Audio Class-Specific Request Codes         */
/***************************************************/
/*!<Define Audio Class Specific Request */
#define UAC_REQUEST_CODE_UNDEFINED  0x00
#define UAC_SET_CUR                 0x01
#define UAC_GET_CUR                 0x01
#define UAC_SET_RANGE               0x02
#define UAC_GET_RANGE               0x02
#define UAC_SET_MEM                 0x03
#define UAC_GET_MEM                 0x03

#define MUTE_CONTROL                0x01
#define VOLUME_CONTROL              0x02

#define FREQ_CONTROL                0x01
#define FREQ_VALID                  0x02
/*-------------------------------------------------------------*/
/* Define EP maximum packet size */
#define CEP_MAX_PKT_SIZE        64
//#define CEP_OTHER_MAX_PKT_SIZE  64

//#define EPA_MAX_PKT_SIZE        REC_RATE*REC_CHANNELS*2/1000
//#define EPA_OTHER_MAX_PKT_SIZE  REC_RATE*REC_CHANNELS*2/1000
//#define EPA_MAX_PKT_SIZE        192
//#define EPA_OTHER_MAX_PKT_SIZE  200

//#define EPB_MAX_PKT_SIZE       (PLAY_RATE*PLAY_CHANNELS*3/1000+128)
//#define EPB_OTHER_MAX_PKT_SIZE  (PLAY_RATE*PLAY_CHANNELS*3/1000+128)
//#define EPB_MAX_PKT_SIZE       200
//#define EPB_OTHER_MAX_PKT_SIZE  (PLAY_RATE*PLAY_CHANNELS*4/8000)

#define CEP_BUF_BASE    0
#define CEP_BUF_LEN     CEP_MAX_PKT_SIZE

#define EPA_BUF_BASE   (CEP_BUF_BASE + CEP_BUF_LEN)

/* FEEDBACK */
#define EPE_BUF_BASE    (EPA_BUF_BASE + EPA_BUF_LEN)
#define EPE_BUF_LEN     64
#define EPE_MAX_PKT_SIZE  64


/* HID_INPUT */
#define EPC_BUF_BASE    (EPE_BUF_BASE + EPE_BUF_LEN)
#define EPC_BUF_LEN     8
#define EPC_MAX_PKT_SIZE  8

#define EPB_BUF_BASE   (EPC_BUF_BASE + EPC_BUF_LEN)
#define EPB_BUF_LEN     0x800 - (CEP_BUF_LEN +EPA_BUF_LEN+EPC_BUF_LEN+EPE_BUF_LEN)

/*-------------------------------------------------------------*/
//void UAC_Init_20(S_AUDIO_LIB* psAudioLib);
//void UAC_ClassRequest_20(void);
//void UAC_SetInterface_20(uint32_t u32AltInterface);
//void UAC_ClassOUT_20(S_AUDIO_LIB* psAudioLib);

//void EPC_Handler_20(void);
//void EPD_Handler_20(void);
//void EPF_Handler_20(void);
//void EPE_Handler_20(void);

#endif  /* __USBD_UAC_H__ */

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
