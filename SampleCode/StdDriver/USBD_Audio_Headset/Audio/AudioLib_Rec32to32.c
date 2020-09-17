/**************************************************************************//**
 * @file        AudioLib_Rec32to32.c
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
void _UAC_MicSendTo32to32(S_AUDIO_LIB* psAudioLib)
{
    /* executed in USB IRQ */

    int32_t i, j, i32RecPcmWorkSmplCnt;
    int32_t i32Smpl1, i32Smpl2;

    int32_t *pi32RecPcmWorkBuf;
    int32_t *pi32RecPcmTmpBuf;

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
            psAudioLib->m_u32RecPcmWorkBufIdx2 = RING_BUF_32CNT + (int32_t)(psAudioLib->m_u32RecPcmWorkBufIdx - psAudioLib->m_u16I2sSmplCntRecStart);
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

    pi32RecPcmWorkBuf = (int32_t *)psAudioLib->m_pu8RecPcmWorkBuf;
    pi32RecPcmTmpBuf  = (int32_t *)psAudioLib->m_pu8RecPcmTmpBuf;

    for ( i = 0, j = 0; i < i32RecPcmWorkSmplCnt; i += 2 )
    {
        i32Smpl1 = pi32RecPcmWorkBuf[psAudioLib->m_u32RecPcmWorkBufIdx2++];

        if ( psAudioLib->m_u32RecPcmWorkBufIdx2 >= RING_BUF_32CNT )
            psAudioLib->m_u32RecPcmWorkBufIdx2 = 0;

        i32Smpl2 = pi32RecPcmWorkBuf[psAudioLib->m_u32RecPcmWorkBufIdx2++];

        if ( psAudioLib->m_u32RecPcmWorkBufIdx2 >= RING_BUF_32CNT )
            psAudioLib->m_u32RecPcmWorkBufIdx2 = 0;

        if ( psAudioLib->m_u8RecChannels == 1 )
            pi32RecPcmTmpBuf[j++] = (i32Smpl1 + i32Smpl2) >> 1;
        else
        {
            pi32RecPcmTmpBuf[i  ] = i32Smpl1;

            pi32RecPcmTmpBuf[i+1] = i32Smpl2;
        }
    }
}
#endif  // CONFIG_AUDIO_REC
