#include "NUC505Series.h"

#include "AudioLib.h"
#include "usbd_audio_20.h"

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
void UAC_SetInterface_20(uint32_t u32AltInterface)
{
    S_AUDIO_LIB* psAudioLib = &g_sAudioLib;

    if ((gUsbCmd.wIndex & 0xff) == 1)        /* Interface 1 */
    {
        /* Audio Iso OUT interface */
        if (u32AltInterface == 2)            /* Interface 1, Alternate 2 */
        {
            psAudioLib->m_u8PlayBitRate  = 24;
            psAudioLib->m_u8PlayChannels =  2;
            psAudioLib->m_pfnPlayConfigMaxPayload20( psAudioLib );
            USBD_SET_MAX_PAYLOAD(EPB, psAudioLib->m_u16PlayMaxPayload2_);
            printf("dP++s\n");
        }
        else if (u32AltInterface == 1)       /* Interface 1, Alternate 1 */
        {
            psAudioLib->m_u8PlayBitRate  = 16;
            psAudioLib->m_u8PlayChannels =  2;
            psAudioLib->m_pfnPlayConfigMaxPayload20( psAudioLib );
            USBD_SET_MAX_PAYLOAD(EPB, psAudioLib->m_u16PlayMaxPayload2_);
            printf("dP+s\n");
        }
        else                                 /* Close Interface 1 */
        {
            psAudioLib->m_pfnPlayStop( psAudioLib );
            USBD->EP[EPB].EPRSPCTL |= USBD_EPRSPCTL_FLUSH_Msk;
            printf("dp-\n");
        }

    }
}
