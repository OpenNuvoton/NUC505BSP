#include "NUC505Series.h"

#include "AudioLib.h"
#include "usbd_audio_10.h"

extern S_AUDIO_LIB g_sAudioLib;

/**
 * @brief       Set Interface standard request
 *
 * @param[in]   u32AltInterface Interface
 *
 * @return      None
 *
 * @details     This function is used to set UAC Class relative setting
 */
void UAC_SetInterface_10(uint32_t u32AltInterface)
{
    S_AUDIO_LIB* psAudioLib = &g_sAudioLib;
    
    if ((gUsbCmd.wIndex & 0xff) == 1)        /* Interface 1 for (Speaker & Microphone) / Microphone Only */ 
    { /* Audio Iso IN interface */
        if (u32AltInterface == 6)            /* Interface 1, Alternate 6 */
        {
            psAudioLib->m_u8RecBitRate  = 24;
            psAudioLib->m_u8RecChannels =  2;
            psAudioLib->m_pfnRecConfigMaxPayload10( psAudioLib );
            /* Set the maximum transfer data size per packet for EPA (Must less than EP Maximum Packet in d escriptor) */          
            USBD_SET_MAX_PAYLOAD(EPA, (psAudioLib->m_u16RecMaxPayload1_+24));
            /* Enable EPA IN Token Interrupt for Audio Record (Write data to EPA Buffer) */
            USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_INTKIEN_Msk);
            printf("dR++s\n");
        }
        else if (u32AltInterface == 5)       /* Interface 1, Alternate 5 */
        {
            psAudioLib->m_u8RecBitRate  = 16;
            psAudioLib->m_u8RecChannels =  2;
            psAudioLib->m_pfnRecConfigMaxPayload10( psAudioLib );
            /* Set the maximum transfer data size per packet for EPA (Must less than EP Maximum Packet in descriptor) */          
            USBD_SET_MAX_PAYLOAD(EPA, (psAudioLib->m_u16RecMaxPayload1_+24));
            /* Enable EPA IN Token Interrupt for Audio Record (Write data to EPA Buffer) */
            USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_INTKIEN_Msk);
            printf("dR+s\n");
        }
        else if (u32AltInterface == 4)       /* Interface 1, Alternate 4 */
        {
            psAudioLib->m_u8RecBitRate  = 24;
            psAudioLib->m_u8RecChannels =  2;
            psAudioLib->m_pfnRecConfigMaxPayload10( psAudioLib );
            /* Set the maximum transfer data size per packet for EPA (Must less than EP Maximum Packet in descriptor) */  
            USBD_SET_MAX_PAYLOAD(EPA, (psAudioLib->m_u16RecMaxPayload1_+24));
            /* Enable EPA IN Token Interrupt for Audio Record (Write data to EPA Buffer) */
            USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_INTKIEN_Msk);
            printf("dR++s\n");
        }
        else if (u32AltInterface == 3)       /* Interface 1, Alternate 3 */
        {
            psAudioLib->m_u8RecBitRate  = 16;
            psAudioLib->m_u8RecChannels =  2;
            psAudioLib->m_pfnRecConfigMaxPayload10( psAudioLib );
            /* Set the maximum transfer data size per packet for EPA (Must less than EP Maximum Packet in descriptor) */      
            USBD_SET_MAX_PAYLOAD(EPA, (psAudioLib->m_u16RecMaxPayload1_+24));
            /* Enable EPA IN Token Interrupt for Audio Record (Write data to EPA Buffer) */
            USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_INTKIEN_Msk);
            printf("dR+s\n");
        }
        else if (u32AltInterface == 2)       /* Interface 1, Alternate 2 */
        {
            psAudioLib->m_u8RecBitRate  = 16;
            psAudioLib->m_u8RecChannels =  1;
            psAudioLib->m_pfnRecConfigMaxPayload10( psAudioLib );
            /* Set the maximum transfer data size per packet for EPA (Must less than EP Maximum Packet in descriptor) */  
            USBD_SET_MAX_PAYLOAD(EPA, (psAudioLib->m_u16RecMaxPayload1_+24));
            /* Enable EPA IN Token Interrupt for Audio Record (Write data to EPA Buffer) */
            USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_INTKIEN_Msk);
            printf("dR+m\n");
        }
        else if (u32AltInterface == 1)       /* Interface 1, Alternate 1 */ 
        {
            psAudioLib->m_u8RecBitRate  = 16;
            psAudioLib->m_u8RecChannels =  1;
            psAudioLib->m_pfnRecConfigMaxPayload10( psAudioLib );
            /* Set the maximum transfer data size per packet for EPA (Must less than EP Maximum Packet in descriptor) */          
            USBD_SET_MAX_PAYLOAD(EPA, (psAudioLib->m_u16RecMaxPayload1_+24));
            /* Enable EPA IN Token Interrupt for Audio Record (Write data to EPA Buffer) */
            USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_INTKIEN_Msk);
            printf("dR+m\n");
        }
        else                                 /* Close Interface 1 */ 
        {
            psAudioLib->m_pfnRecStop( psAudioLib );
            USBD->EP[EPA].EPRSPCTL |= USBD_EPRSPCTL_FLUSH_Msk;
            printf("dr-\n");
        }
    }
}
