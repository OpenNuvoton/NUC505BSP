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
#include "demo.h"
#include "usbd_audio_10.h"

/**
 * @brief       EPA Handler
 *
 * @param[in]   None
 *
 * @return      None
 *
 * @details     This function is used to process EPA event
 */
/* Record */
void EPA_Handler(S_AUDIO_LIB* psAudioLib)
{
    uint32_t volatile u32timeout = 0x100000;
    psAudioLib->m_pfnRecMode1( psAudioLib );
    
		if(psAudioLib->m_i32RecPcmTmpBufLen == 0)
    {
        printf("psAudioLib->m_i32RecPcmTmpBufLen = %d for DMA\n",psAudioLib->m_i32RecPcmTmpBufLen);
        return;
    }
		
    while(1) {
        if (!(USBD->DMACTL & USBD_DMACTL_DMAEN_Msk))
            break;
        
        if((USBD->CEPINTSTS & USBD->CEPINTEN) & USBD_CEPINTSTS_SETUPPKIF_Msk)
            return;
        if (!USBD_IS_ATTACHED())
            break;
        if(u32timeout == 0)				
        {
            printf("EPA\t%x\n", USBD->EP[EPA].EPDATCNT);
            printf("EPB\t%x\n", USBD->EP[EPB].EPDATCNT);
            printf("EPC\t%x\n", USBD->EP[EPC].EPDATCNT);
            printf("DMACTL\t%X\n", USBD->DMACTL);	
            printf("DMACNT\t%X\n", USBD->DMACNT);	
            u32timeout = 0x100000;
        }					
        else
            u32timeout--;				
    }
    
    USBD_SET_DMA_READ(ISO_IN_EP_NUM);
    USBD_ENABLE_BUS_INT(USBD_BUSINTEN_SUSPENDIEN_Msk|USBD_BUSINTEN_RSTIEN_Msk|USBD_BUSINTEN_VBUSDETIEN_Msk);
    USBD_SET_DMA_ADDR((uint32_t)psAudioLib->m_pu8RecPcmTmpBuf);
    USBD_SET_DMA_LEN(psAudioLib->m_i32RecPcmTmpBufLen);
    //if(psAudioLib->m_i32RecPcmTmpBufLen > (EPA_BUF_LEN - (USBD->EP[EPA].EPDATCNT & 0xffff)))
        //printf("Larger than EPA Buffer %d %d\n", psAudioLib->m_i32RecPcmTmpBufLen, (EPA_BUF_LEN - (USBD->EP[EPA].EPDATCNT & 0xffff)));
    g_usbd_ShortPacket = 1;
    USBD_ENABLE_DMA();
}

#if defined __HID20__ || defined __HID__
extern int32_t g_hid_type;
extern uint8_t g_u8EPCReady;

#ifdef __ICCARM__
#pragma data_alignment=4
           static volatile uint8_t buf[8];
#pragma data_alignment=4
           volatile uint32_t g_hid_count = 0;
#else   // __CC_ARM
__align(4) static volatile uint8_t buf[8];
__align(4) volatile uint32_t g_hid_count = 0;
#endif

void HID_UpdateKbData(void)
{
    /* executed in main loop */
    
    int32_t n;
    int32_t volatile i;
    uint32_t key = 0xF;
    static uint32_t preKey;
    n = 8;
    if(g_u8EPCReady)
    {
        if(USBD->EP[EPC].EPDATCNT & 0xFFFF)
        {
            g_hid_count++;
            g_u8EPCReady = 0;
            if(g_hid_count >2 || (((USBD->EP[EPC].EPDATCNT & 0xFFFF) % 8) != 0))
            {
                printf("HID %d %d\n",g_hid_count,USBD->EP[EPC].EPDATCNT & 0xFFFF );
                USBD->EP[EPC].EPRSPCTL |= USBD_EPCRSPCTL_FLUSH_Msk;
                g_hid_count = 0;
            }
            return;
        }
        
        key = !PC0_PIN | (!PC1_PIN << 1) | (!PC2_PIN << 1) | (!PC3_PIN << 1) | (!PC4_PIN << 1);
        
        if(key == 0)
        {
            for(i = 0; i < n; i++)
                buf[i] = 0;
            
            if(key != preKey)
            {
                preKey = key;
            }
            else
                return;
        }
        else
        {
            if(g_hid_type == HID_TYPE_MEDIAKEY)
            {
                if(preKey == key)
                    return;
                
                preKey = key;
                buf[0] = 0;
                buf[1] = 0;
                if(!PC0_PIN)
                    buf[1] |= HID_CTRL_PAUSE;
                else if(!PC1_PIN)
                    buf[1] |= HID_CTRL_NEXT;
                else if(!PC2_PIN)
                    buf[1] |= HID_CTRL_PREVIOUS;
                else if(!PC3_PIN)
                    buf[0] |= HID_CTRL_VOLUME_INC;
                else if(!PC4_PIN)
                    buf[0] |= HID_CTRL_VOLUME_DEC;
            }
            else
            {
                if(preKey == key)
                    return;
                
                preKey = key;
                if(!PC0_PIN)
                    buf[2] = 0x04;/* Key A */
                else if(!PC1_PIN)
                    buf[2] = 0x05;/* Key B */
                else if(!PC2_PIN)
                    buf[2] = 0x06;/* Key C */
                else if(!PC3_PIN)
                    buf[2] = 0x07;/* Key D */
                else if(!PC4_PIN)
                    buf[2] = 0x08;/* Key E */
                else if(!PC5_PIN)
                    buf[2] = 0x09;/* Key F */
            }
        }
    
        NVIC_DisableIRQ(USBD_IRQn);
        /* Set transfer length and trigger IN transfer */
        while (1)
        {
            if (!(USBD->DMACTL & USBD_DMACTL_DMAEN_Msk))
                break;
            
            if (!USBD_IS_ATTACHED())
                break;
        }
        if(USBD->EP[EPC].EPDATCNT & 0xFFFF)
            USBD->EP[EPC].EPRSPCTL |= USBD_EPCRSPCTL_FLUSH_Msk;
        USBD_SET_DMA_READ(HID_IN_EP_NUM);
        USBD_SET_DMA_ADDR((uint32_t)&buf[0]);
        USBD_SET_DMA_LEN(8);
        USBD_ENABLE_DMA();
        NVIC_EnableIRQ(USBD_IRQn);
        while (1)
        {
            if ((USBD->EP[EPC].EPDATCNT & 0xFFFF)>=8)
                break;
            if (!USBD_IS_ATTACHED())
                break;
        }
        
        if ((USBD->EP[EPC].EPDATCNT & 0xFFFF)>=8)
            USBD->EP[EPC].EPTXCNT = 8;
        g_u8EPCReady = 0;
        g_hid_count = 0;
        return;
    }
}

void EPC_Handler(void)  /* Interrupt IN handler */
{
    g_u8EPCReady = 1;
}
#endif
