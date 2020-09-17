/**************************************************************************//**
 * @file        AudioLib_Rec16to24.c
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
#include "AudioLib2.h"

#if CONFIG_AUDIO_REC
void _UAC_MicSendTo16to24(S_AUDIO_LIB* psAudioLib)
{
    /* executed in USB IRQ */

    int32_t i, j, i32RecPcmWorkSmplCnt;

    int16_t i16Smpl1, i16Smpl2;

    int16_t *pi16RecPcmWorkBuf;
    uint8_t *pu8RecPcmTmpBuf;

    if ( psAudioLib->m_u8RecFlag == 0 )
    {
        psAudioLib->m_u8RecFlag = 1;

        if ( psAudioLib->m_u32RecPcmWorkBufIdx >= psAudioLib->m_u16I2sSmplCntRecStart )
        {
            psAudioLib->m_u32RecPcmWorkBufIdx2 = psAudioLib->m_u32RecPcmWorkBufIdx - psAudioLib->m_u16I2sSmplCntRecStart;
            //printf("1 %d %d %d\n", psAudioLib->m_u32RecPcmWorkBufIdx, psAudioLib->m_u32RecPcmWorkBufIdx2, psAudioLib->m_u16I2sSmplCntRecStart);
        }
        else
        {
            psAudioLib->m_u32RecPcmWorkBufIdx2 = RING_BUF_16CNT + (int32_t)(psAudioLib->m_u32RecPcmWorkBufIdx - psAudioLib->m_u16I2sSmplCntRecStart);
            //printf("2 %d %d %d\n", psAudioLib->m_u32RecPcmWorkBufIdx, psAudioLib->m_u32RecPcmWorkBufIdx2, psAudioLib->m_u16I2sSmplCntRecStart);
        }
    }

    if ( psAudioLib->m_pu8RecPacketSequence[psAudioLib->m_u16RecPacketSequenceIdx] == 0 )
    {
        psAudioLib->m_i32RecPcmTmpBufLen = psAudioLib->m_u16RecMaxPayload1_;
        i32RecPcmWorkSmplCnt             = psAudioLib->m_u16RecSmplCnt1_;
    }
    else
    {
        psAudioLib->m_i32RecPcmTmpBufLen = psAudioLib->m_u16RecMaxPayload2_;
        i32RecPcmWorkSmplCnt             = psAudioLib->m_u16RecSmplCnt2_;
    }

    if ( psAudioLib->m_u16RecPacketSequenceIdx++ > psAudioLib->m_u16RecPacketSequenceCnt )
        psAudioLib->m_u16RecPacketSequenceIdx = 0;

    USBD_SET_MAX_PAYLOAD(EPA, psAudioLib->m_i32RecPcmTmpBufLen);

    pi16RecPcmWorkBuf = (int16_t *)psAudioLib->m_pu8RecPcmWorkBuf;
    pu8RecPcmTmpBuf   = (uint8_t *)psAudioLib->m_pu8RecPcmTmpBuf;

    for ( i = 0, j = 0; i < i32RecPcmWorkSmplCnt; i += 2 )
    {
        i16Smpl1 = pi16RecPcmWorkBuf[psAudioLib->m_u32RecPcmWorkBufIdx2++];

        if ( psAudioLib->m_u32RecPcmWorkBufIdx2 >= RING_BUF_16CNT )
            psAudioLib->m_u32RecPcmWorkBufIdx2 = 0;

        i16Smpl2 = pi16RecPcmWorkBuf[psAudioLib->m_u32RecPcmWorkBufIdx2++];

        if ( psAudioLib->m_u32RecPcmWorkBufIdx2 >= RING_BUF_16CNT )
            psAudioLib->m_u32RecPcmWorkBufIdx2 = 0;

        if ( psAudioLib->m_u8RecChannels == 1 )
        {
#ifdef __ICCARM__
            i16Smpl1 = __SSAT( (int32_t)(i16Smpl1 + i16Smpl2), 16 );
#elif defined __GNUC__
            i16Smpl1 = __SSAT( (int32_t)(i16Smpl1 + i16Smpl2), 16 );
#else   // __CC_ARM
            i16Smpl1 = __ssat( (int32_t)(i16Smpl1 + i16Smpl2), 16 );
#endif
            pu8RecPcmTmpBuf[j++] = (i16Smpl1 & 0x0000) >> 0;
            pu8RecPcmTmpBuf[j++] = (i16Smpl1 & 0x00FF) >> 0;
            pu8RecPcmTmpBuf[j++] = (i16Smpl1 & 0xFF00) >> 8;
        }
        else
        {
            pu8RecPcmTmpBuf[j++] = (i16Smpl1 & 0x0000) >> 0;
            pu8RecPcmTmpBuf[j++] = (i16Smpl1 & 0x00FF) >> 0;
            pu8RecPcmTmpBuf[j++] = (i16Smpl1 & 0xFF00) >> 8;

            pu8RecPcmTmpBuf[j++] = (i16Smpl2 & 0x0000) >> 0;
            pu8RecPcmTmpBuf[j++] = (i16Smpl2 & 0x00FF) >> 0;
            pu8RecPcmTmpBuf[j++] = (i16Smpl2 & 0xFF00) >> 8;
        }
    }
}
#endif  // CONFIG_AUDIO_REC
