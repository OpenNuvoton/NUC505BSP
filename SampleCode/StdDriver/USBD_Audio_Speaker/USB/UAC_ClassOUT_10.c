/**************************************************************************//**
 * @file        UAC_ClassOUT_10.c
 * @version     V1.00
 * $Revision:   1$
 * $Date:       17/05/09 5:00p$
 * @brief       NUC505 Series USB Audio Class Sample Code source file
 *
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "NUC505Series.h"
#include "AudioLib.h"
#include "usbd_audio_10.h"

void UAC_ClassOUT_10(S_AUDIO_LIB* psAudioLib)
{
    uint32_t u32Temp1;
    uint32_t volatile u32timeout = 0x100000;
    /* To make sure that no DMA is reading the Endpoint Buffer (4-8 & 4-5)*/
    while(1)
    {
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

    /* Enable Control Endpoint to receive data (3)*/
    /* Host to device */
    switch (gUsbCmd.bRequest)
    {
    case UAC_SET_CUR:
    {
        if ((gUsbCmd.wIndex & 0xf) == ISO_OUT_EP_NUM)      /* request to endpoint */
        {
            USBD_CtrlOut((uint8_t *)&psAudioLib->m_u32PlaySampleRate, gUsbCmd.wLength);
            /* Status stage */
            USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
            psAudioLib->m_pfnPlayConfigMaxPayload10( psAudioLib );
            USBD_SET_MAX_PAYLOAD(EPB, psAudioLib->m_u16PlayMaxPayload2_);
            printf("hdP\t%d\n", psAudioLib->m_u32PlaySampleRate);
        }
        else    /* request to interface */
        {
            switch ((gUsbCmd.wValue & 0xff00) >> 8)
            {
            case MUTE_CONTROL:
                if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                {
                    USBD_CtrlOut((uint8_t *)&psAudioLib->m_u8PlayMute, gUsbCmd.wLength);
                    //printf("hdPm\t%d\n", psAudioLib->m_u8PlayMute);
                }
                else
                {
                    USBD_CtrlOut((uint8_t *)&u32Temp1, gUsbCmd.wLength);
                    //printf("g\n");
                }

                /* Status stage */
                USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
                break;

            case VOLUME_CONTROL:
                if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                {
                    if ((gUsbCmd.wValue & 0xff) == 1)
                    {
                        USBD_CtrlOut((uint8_t *)&psAudioLib->m_i16PlayVolumeL, gUsbCmd.wLength);
                        //printf("hdPl\t0x%04X\n", (uint16_t)psAudioLib->m_i16PlayVolumeL);
                    }
                    else
                    {
                        USBD_CtrlOut((uint8_t *)&psAudioLib->m_i16PlayVolumeR, gUsbCmd.wLength);
                        //printf("hdPr\t0x%04X\n", (uint16_t)psAudioLib->m_i16PlayVolumeR);
                    }
                }
                else if (0x0d == ((gUsbCmd.wIndex >> 8) & 0xff))
                {
                    USBD_CtrlOut((uint8_t *)&u32Temp1, gUsbCmd.wLength);
                    //printf("g\n");
                }
                else
                {
                    USBD_CtrlOut((uint8_t *)&u32Temp1, gUsbCmd.wLength);
                    //printf("g\n");
                }
                /* Status stage */
                USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
                break;

            default:
            {
                USBD->CEPCTL = USBD_CEPCTL_FLUSH_Msk;
                /* STALL control pipe */
                USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
                break;
            }
            }
        }
        break;
    }
#ifdef __HID__
    case SET_REPORT:
    {
        if (((gUsbCmd.wValue >> 8) & 0xff) == 2)
        {
            /* Request Type = Feature */
            USBD_CtrlOut((uint8_t *)&u32Temp1, gUsbCmd.wLength);
            /* Status stage */
            USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
            //printf("SET_REPORT %d %d\n",gUsbCmd.wLength,*(uint8_t *)u32Temp1);
        }
        break;
    }
    case SET_IDLE:
    {
        /* Status stage */
        USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
        //printf("Set Idle\n");
        break;
    }
    case SET_PROTOCOL:
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
