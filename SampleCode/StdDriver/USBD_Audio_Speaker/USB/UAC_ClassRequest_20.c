#include "NUC505Series.h"

#include "AudioLib.h"
#include "usbd_audio_20.h"

extern S_AUDIO_LIB g_sAudioLib;

extern uint32_t    g_u32ClassOUT_20;
extern uint8_t     g_uac_20_flag;
extern uint8_t     g_uac_20_mode_flag;

#ifdef __ICCARM__
#pragma data_alignment=4
           static int16_t s_ai16SpkVolRange[4] = {
#else   // __CC_ARM
static int16_t s_ai16SpkVolRange[4] __attribute__((aligned(4))) = {
#endif
    1,
    PLAY_MIN_VOL,
    PLAY_MAX_VOL,
    PLAY_RES_VOL
};

#ifdef __ICCARM__
#pragma data_alignment=4
           static uint8_t Speedx[] = {
#else   // __CC_ARM
static uint8_t Speedx[] __attribute__((aligned(4))) = {
#endif
    0x06, 0x00,             //number of sample rate triplets
    
    0x44, 0xAC, 0x00, 0x00, //44.1k Min
    0x44, 0xAC, 0x00, 0x00, //44.1k Max
    0x00, 0x00, 0x00, 0x00, //0 Res
    
    0x88, 0x58, 0x01, 0x00, //88.2k Min
    0x88, 0x58, 0x01, 0x00, //88.2k Max
    0x00, 0x00, 0x00, 0x00, //0 Res
    
    0x10, 0xB1, 0x02, 0x00, //176.4k Min
    0x10, 0xB1, 0x02, 0x00, //176.4k Max
    0x00, 0x00, 0x00, 0x00, //0 Res
    
    0x80, 0xBB, 0x00, 0x00, //48k Min
    0x80, 0xBB, 0x00, 0x00, //48k Max
    0x00, 0x00, 0x00, 0x00, //0 Res
    
    0x00, 0x77, 0x01, 0x00, //96k Min
    0x00, 0x77, 0x01, 0x00, //96k Max
    0x00, 0x00, 0x00, 0x00, //0 Res
    
    0x00, 0xEE, 0x02, 0x00, //192k Max
    0x00, 0xEE, 0x02, 0x00, //192k Max
    0x00, 0x00, 0x00, 0x00  //0 Res
};

void UAC_ClassRequest_20(void)
{
    S_AUDIO_LIB* psAudioLib = &g_sAudioLib;
    uint32_t volatile u32timeout = 0x100000;
    
    uint8_t tempbuf[10];
    
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
        
        if(gUsbCmd.bmRequestType == 0x82)
        {
            if (ISO_OUT_EP_NUM == (gUsbCmd.wIndex)) /* g_usbd_PlaySamplingFrequency */
            {
                USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_u32PlaySampleRate, 3);
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                //printf("dhP\t%d\n", psAudioLib->m_u32PlaySampleRate);
            }
            return;
        }
        
        switch (gUsbCmd.bRequest&0x7f)
        {
            case UAC_GET_CUR:
            {
                if (CLOCK_SOURCE_ID == ((gUsbCmd.wIndex >> 8) & 0xff))
                {
                    switch((gUsbCmd.wValue & 0xff00) >> 8)
                    {
                        case FREQ_CONTROL:
                        {
                            USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_u32PlaySampleRate, 4);
                            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk  );

                            if(g_uac_20_mode_flag)
                                if(g_uac_20_flag == 0)
                                    g_uac_20_flag  = 1;
                            //printf("GET FREQ_CONTROL\n");
                            break;
                        }
                        case FREQ_VALID:
                        {
                            tempbuf[0]=0x1;
                            USBD_PrepareCtrlIn(tempbuf, 1);
                            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk  );
                            //printf("GET FREQ_VALID\n");
                            break;
                        }
                    }
                }
                else
                    switch ((gUsbCmd.wValue & 0xff00) >> 8)
                    {
                        case MUTE_CONTROL:
                        {
                            if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                            {
                                USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_u8PlayMute, 1);
                                //printf("dhPm\t%d\n",psAudioLib->m_u8PlayMute);
                            }
                            
                            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk  );
                            //printf("GET MUTE_CONTROL\n");
                            break;
                        }
                        case VOLUME_CONTROL:
                        {
                            if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                            {
                                if((gUsbCmd.wValue & 0xff) == 1)
                                {
                                    USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_i16PlayVolumeL, 2);
                                    USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                                    USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk  );
                                    //printf("dhPl\t0x%04X\n",(uint16_t)psAudioLib->m_i16PlayVolumeL);
                                }
                                else
                                {
                                    USBD_PrepareCtrlIn((uint8_t *)&psAudioLib->m_i16PlayVolumeR, 2);
                                    USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                                    USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk  );
                                    //printf("dhPr\t0x%04X\n",(uint16_t)psAudioLib->m_i16PlayVolumeR);
                                }
                            }
                            break;
                        }
                        
                        default:
                        {
                            /* Setup error, stall the device */
                            USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);  /* USBD_CEPCTL_STALLEN_Msk */
                        }
                    }
                break;
            }
            case UAC_GET_RANGE:
            {
                if (CLOCK_SOURCE_ID == ((gUsbCmd.wIndex >> 8) & 0xff))
                    switch ((gUsbCmd.wValue & 0xff00) >> 8)
                    {
                        case FREQ_CONTROL:
                        {
                            if (CLOCK_SOURCE_ID == ((gUsbCmd.wIndex >> 8) & 0xff))
                            {
                                USBD_PrepareCtrlIn((uint8_t *)Speedx, gUsbCmd.wLength);
                            }
                            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk  );
                            break;
                        }
                        
                        default:
                            /* STALL control pipe */
                            USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
                    }
                else
                    switch ((gUsbCmd.wValue & 0xff00) >> 8)
                    {
                        case VOLUME_CONTROL:
                        {
                            if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                            {
                                USBD_PrepareCtrlIn((uint8_t *)s_ai16SpkVolRange, gUsbCmd.wLength);
                            }
                            
                            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk  );
                            break;
                        }
                        default:
                            /* STALL control pipe */
                            USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
                    }
                break;
            }
            #if 0
            case UAC_AUDIO_GET_CUR:
            {
                if (CLOCK_SOURCE_ID == ((gUsbCmd.wIndex >> 8) & 0xff))
                {
                    switch((gUsbCmd.wValue & 0xff00) >> 8)
                    {
                        case FREQ_CONTROL:
                        {
                            USBD_PrepareCtrlIn((uint8_t *)&g_usbd_PlaySamplingFrequency, 4);
                            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                            USBD_ENABLE_CEP_INT(USBD_CEP_IRQ_ENB_IN_TK_Msk);
                            break;
                        }
                        case FREQ_VALID:
                        {
                            tempbuf[0]=0x1;
                            USBD_PrepareCtrlIn(tempbuf, 1);
                            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                            USBD_ENABLE_CEP_INT(USBD_CEP_IRQ_ENB_IN_TK_Msk);
                            break;
                        }
                    }
                }
                break;
            }
            
            case UAC_AUDIO_GET_RANGE:
            {
                switch ((gUsbCmd.wValue & 0xff00) >> 8)
                {
                    case FREQ_CONTROL:
                    {
                        if (CLOCK_SOURCE_ID == ((gUsbCmd.wIndex >> 8) & 0xff))
                        {
                            USBD_PrepareCtrlIn((uint8_t *)Speedx, gUsbCmd.wLength);
                        }
                        USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                        USBD_ENABLE_CEP_INT(USBD_CEP_IRQ_ENB_IN_TK_Msk);
                        //printf("UAC_AUDIO_GET_RANGE FREQ_CONTROL*\n");
                        break;
                    }
                    default:
                        /* STALL control pipe */
                        USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
                }
                break;
            }
            #endif
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
        g_u32ClassOUT_20 = 1;
        USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
        USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
    }
}
