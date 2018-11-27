/******************************************************************************
 * @file     usbd_audio.c
 * @brief    NuMicro series USBD driver Sample file
 * @date     2017/04/26 09:30 a.m.
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "NUC505Series.h"

#include "AudioLib.h"
#include "usbd_audio_20.h"

void EPE_Handler(void)
{
#if 0
    uint32_t PlayRate;

    PlayRate = g_usbd_SampleRate;
    //sample_flag=1;
    //  return;

    PlayRate = (g_usbd_SampleRate/8000);
    PlayRate <<= 16;
    PlayRate&=0xffff0000;
    PlayRate += (((uint32_t)g_usbd_SampleRate% 8000)*0xffff)/8000;
    // PlayRate&=0xfffffff0;

    // USBD->EPE_DATA_BUF_BYTE=  (BufferRemainSize>>8)&0xff;//
    // PlayRate = (PlayRate<<6)&0xfffffffc;
#if 0
    USBD->EP[EPE].EPDAT_BYTE=( PlayRate&0xff);
    USBD->EP[EPE].EPDAT_BYTE=((PlayRate>>8)&0xff);
    USBD->EP[EPE].EPDAT_BYTE=((PlayRate>>16)&0xff);
    USBD->EP[EPE].EPDAT_BYTE=((PlayRate>>24)&0xff);
#else
    while(1)
    {
        if (!(USBD->DMACTL & USBD_DMACTL_DMAEN_Msk))
            break;

        if (!USBD_IS_ATTACHED())
            break;
    }
    USBD_SET_DMA_READ(ISO_FEEDBACK_ENDPOINT);
    USBD_ENABLE_BUS_INT(USBD_BUSINTEN_SUSPENDIEN_Msk|USBD_BUSINTEN_RSTIEN_Msk|USBD_BUSINTEN_VBUSDETIEN_Msk);
    USBD_SET_DMA_ADDR((uint32_t)&PlayRate);
    USBD_SET_DMA_LEN(4);
    g_usbd_ShortPacket = 1;
    USBD_ENABLE_DMA();
#endif

    USBD->EP[EPE].EPRSPCTL = USB_EP_RSPCTL_SHORTTXEN;
    ///FeedBackOk =1;
#endif
}

void EPF_Handler_20(void)
{

}

#ifdef __HID20__
extern uint8_t g_u8EPCReady;

void EPC_Handler_20(void)  /* Interrupt IN handler */
{
    g_u8EPCReady = 1;
}
#endif
