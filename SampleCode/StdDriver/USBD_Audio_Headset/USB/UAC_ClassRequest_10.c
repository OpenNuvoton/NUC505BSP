#include "NUC505Series.h"

#include "AudioLib.h"
#include "usbd_audio_10.h"

extern S_AUDIO_LIB g_sAudioLib;

extern uint32_t g_u32ClassOUT_10;

void UAC_ClassRequest_10(void)
{
    S_AUDIO_LIB* psAudioLib = &g_sAudioLib;
    uint32_t volatile u32timeout = 0x100000;
    
    if (gUsbCmd.bmRequestType & 0x80)   /* request data transfer direction */
    {
        /* To make sure that no DMA is writing the Endpoint Buffer (2-6) */
        while(1) {
            if (!(USBD->DMACTL & USBD_DMACTL_DMAEN_Msk))
                break;
            
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
        
        USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STSDONEIF_Msk);
        // Device to host
        switch (gUsbCmd.bRequest)
        {
            case UAC_GET_CUR:
            {
                if ((gUsbCmd.wIndex & 0xf) == ISO_OUT_EP_NUM) {    /* request to endpoint */
                    USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_u32PlaySampleRate, gUsbCmd.wLength);
                    USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                    USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                    //printf("dhP\t%d\n", psAudioLib->m_u32PlaySampleRate);
                }
                else if ((gUsbCmd.wIndex & 0xf) == ISO_IN_EP_NUM) {    /* request to endpoint */
                    USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_u32RecSampleRate, gUsbCmd.wLength);
                    USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                    USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                    //printf("dhR\t%d\n", psAudioLib->m_u32RecSampleRate);
                }
                else {
                    switch ((gUsbCmd.wValue & 0xff00) >> 8)
                    {
                        case MUTE_CONTROL:
                        {
                            if (REC_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                            {
                                USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_u8RecMute, 1);
                                //printf("dhRm\t%d\n",psAudioLib->m_u8RecMute);
                            }
                            else if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                            {
                                USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_u8PlayMute, 1);
                                //printf("dhPm\t%d\n",psAudioLib->m_u8PlayMute);
                            }
                            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                            break;
                        }
                        case VOLUME_CONTROL:
                        {
                            if (REC_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                            {
                                USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_i16RecVolumeL, 2);
                                //printf("dhRv\t0x%04X\n",(uint16_t)psAudioLib->m_i16RecVolumeL);
                            }
                            else if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                            {
                                if((gUsbCmd.wValue & 0xff) == 1)
                                {
                                    USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_i16PlayVolumeL, 2);
                                    //printf("dhPl\t0x%04X\n",(uint16_t)psAudioLib->m_i16PlayVolumeL);
                                }
                                else
                                {
                                    USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_i16PlayVolumeR, 2);
                                    //printf("dhPr\t0x%04X\n",(uint16_t)psAudioLib->m_i16PlayVolumeR);
                                }
                            }
                            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                            break;
                        }
                        #if 0
                        case AUTOMATIC_GAIN_CONTROL:
                        {
                            if (REC_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                                USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_u8RecMute, 1);
                            else if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                                USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_u8PlayMute, 1);
                            //printf("UAC_GET_CUR - AUTOMATIC_GAIN_CONTROL\n");
                            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                            break;
                        }
                        #endif
                        default:
                        {
                            USBD->CEPCTL = USBD_CEPCTL_FLUSH_Msk;
                            /* Setup error, stall the device */
                            USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
                            break;
                        }
                    }
                }
                break;
            }
            
            case UAC_GET_MIN:
            {
                switch ((gUsbCmd.wValue & 0xff00) >> 8)
                {
                    case VOLUME_CONTROL:
                    {
                        if (REC_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                        {
                            USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_i16RecMinVolume, 2);
                            //printf("dhRmv\t0x%04X\n",(uint16_t)psAudioLib->m_i16RecMinVolume);
                        }
                        else if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                        {
                            USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_i16PlayMinVolume, 2);
                            //printf("dhPmv\t0x%04X\n",(uint16_t)psAudioLib->m_i16PlayMinVolume);
                        }
                        USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                        USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                        break;
                    }
                    default:
                    {
                        USBD->CEPCTL = USBD_CEPCTL_FLUSH_Msk;
                        /* STALL control pipe */
                        USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
                        break;
                    }
                }
                break;
            }
            
            case UAC_GET_MAX:
            {
                switch ((gUsbCmd.wValue & 0xff00) >> 8)
                {
                    case VOLUME_CONTROL:
                    {
                        if (REC_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                        {
                            USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_i16RecMaxVolume, 2);
                            //printf("dhRMv\t0x%04X\n",(uint16_t)psAudioLib->m_i16RecMaxVolume);
                        }
                        else if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                        {
                            USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_i16PlayMaxVolume, 2);
                            //printf("dhPMv\t0x%04X\n",(uint16_t)psAudioLib->m_i16PlayMaxVolume);
                        }
                        USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                        USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                        break;
                    }
                    default:
                    {
                        USBD->CEPCTL = USBD_CEPCTL_FLUSH_Msk;
                        /* STALL control pipe */
                        USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
                        break;
                    }
                }
                break;
            }
            
            case UAC_GET_RES:
            {
                switch ((gUsbCmd.wValue & 0xff00) >> 8)
                {
                    case VOLUME_CONTROL:
                    {
                        if (REC_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                        {
                            USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_i16RecResVolume, 2);
                            //printf("dhRrv\t0x%04X\n",(uint16_t)psAudioLib->m_i16RecResVolume);
                        }
                        else if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                        {
                            USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_i16PlayResVolume, 2);
                            //printf("dhPrv\t0x%04X\n",(uint16_t)psAudioLib->m_i16PlayResVolume);
                        }
                        USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                        USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                        break;
                    }
                    default:
                    {
                        USBD->CEPCTL = USBD_CEPCTL_FLUSH_Msk;
                        /* STALL control pipe */
                        USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
                        break;
                    }
                }
                break;
            }
            
            default:
            {
                /* Setup error, stall the device */
                USBD->CEPCTL = USBD_CEPCTL_FLUSH_Msk;
                USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
                break;
            }
        }
    }
    else
    {
        g_u32ClassOUT_10 = 1;
        USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
        USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
    }
}
