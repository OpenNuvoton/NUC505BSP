/**************************************************************************//**
 * @file        AudioLib_Rec16_2.c
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
//extern uint32_t g_u32TimerCnt;

//__align(4) static volatile uint32_t s_u32TimerCnt;

/* note re-sample to higher sampling rate causes poor audio quality and more MIPS
   re-sample to lower sampling rate causes poor auido quality
*/
void _UAC_MicCopyTo16_2(S_AUDIO_LIB* psAudioLib)
{
    /* executed in main loop */

    int32_t  i, i32Amount;
    uint32_t u32I2sRxDmaCurIdx;

    int16_t  i16Left, i16Right;

    int16_t *pi16I2sRecPcmBuf;
    int16_t *pi16RecPcmWorkBuf;

    u32I2sRxDmaCurIdx = ((uint32_t)I2S_GET_RXDMA_CADDR(I2S) - (uint32_t)psAudioLib->m_pu8I2sRecPcmBuf) >> 1;

    if ( psAudioLib->m_u8RecFlag2 == 0 )
    {
        psAudioLib->m_u8RecFlag2 = 1;

        i32Amount = 576;

        if ( u32I2sRxDmaCurIdx >= i32Amount )
        {
            psAudioLib->m_u32RecLPcmWorkBufIdx = u32I2sRxDmaCurIdx - i32Amount;
            psAudioLib->m_u32RecRPcmWorkBufIdx = psAudioLib->m_u32RecLPcmWorkBufIdx + 1;
            //printf("+ %d %d %d\n", u32I2sRxDmaCurIdx, psAudioLib->m_u32RecLPcmWorkBufIdx, i32Amount);
        }
        else
        {
            psAudioLib->m_u32RecLPcmWorkBufIdx = RING_BUF_16CNT + (int32_t)(u32I2sRxDmaCurIdx - i32Amount);
            psAudioLib->m_u32RecRPcmWorkBufIdx = psAudioLib->m_u32RecLPcmWorkBufIdx + 1;
            //printf("- %d %d %d\n", u32I2sRxDmaCurIdx, psAudioLib->m_u32RecLPcmWorkBufIdx, i32Amount);
        }
    }
    else
        i32Amount = (int32_t)(u32I2sRxDmaCurIdx - psAudioLib->m_u32I2sRecPcmBufPreIdx);

    if ( i32Amount < 0 )
    {
        i32Amount += RING_BUF_16CNT;
    }

    psAudioLib->m_u32I2sRecPcmBufPreIdx = u32I2sRxDmaCurIdx;

    psAudioLib->m_i32RecPcmWorkSmplCnt += i32Amount;

    if ( psAudioLib->m_i32RecPcmWorkSmplCnt >= 96 )
    {
        psAudioLib->m_i32RecPcmWorkSmplCnt -= 96;

        pi16I2sRecPcmBuf  = (int16_t *)psAudioLib->m_pu8I2sRecPcmBuf;
        pi16RecPcmWorkBuf = (int16_t *)psAudioLib->m_pu8RecPcmWorkBuf;

        /* note based on the sampling rate,
           "descriptor_10.c, usbd_audio_10.h, descriptor_20.c and UAC_ClassRequest_20"

            9 for 44100 Hz or others, psAudioLib->m_u8RecCnt2++ <  9
           19 for 22050 Hz,           psAudioLib->m_u8RecCnt2++ < 19
           39 for 11025 Hz            psAudioLib->m_u8RecCnt2++ < 39
        */
        if ( psAudioLib->m_u8RecCnt2++ < 9 )
        {
            i32Amount = psAudioLib->m_u16RecSmplCnt1;
        }
        else
        {
            psAudioLib->m_u8RecCnt2 = 0;
            i32Amount = psAudioLib->m_u16RecSmplCnt2;
        }

        i32Amount += psAudioLib->m_i32RecPcmWorkResSmplCnt;

        psAudioLib->m_i32RecPcmWorkResSmplCnt = 0;

        for ( i = 0; i < i32Amount; i += 2 )
        {
            i16Left  = (((int32_t)psAudioLib->m_i16RecLPrevPcm * (0x2000 - psAudioLib->m_u32RecLInterpoResidual))+
                        ((int32_t)pi16I2sRecPcmBuf[psAudioLib->m_u32RecLPcmWorkBufIdx] * psAudioLib->m_u32RecLInterpoResidual)) >> 13;
            psAudioLib->m_u32RecLInterpoResidual += psAudioLib->m_u32RecInterpoFactor;
            while ( 1 )
            {
                if ( psAudioLib->m_u32RecLInterpoResidual < 0x2000 )
                    break;
                else
                {
                    psAudioLib->m_i16RecLPrevPcm = pi16I2sRecPcmBuf[psAudioLib->m_u32RecLPcmWorkBufIdx];
                    psAudioLib->m_u32RecLInterpoResidual -= 0x2000;

                    psAudioLib->m_u32RecLPcmWorkBufIdx += 2;
                    if ( psAudioLib->m_u32RecLPcmWorkBufIdx >= RING_BUF_16CNT )
                    {
                        psAudioLib->m_u32RecLPcmWorkBufIdx = 0;
                    }
                }
            }

            i16Right = (((int32_t)psAudioLib->m_i16RecRPrevPcm * (0x2000 - psAudioLib->m_u32RecRInterpoResidual))+
                        ((int32_t)pi16I2sRecPcmBuf[psAudioLib->m_u32RecRPcmWorkBufIdx] * psAudioLib->m_u32RecRInterpoResidual)) >> 13;
            psAudioLib->m_u32RecRInterpoResidual += psAudioLib->m_u32RecInterpoFactor;
            while ( 1 )
            {
                if ( psAudioLib->m_u32RecRInterpoResidual < 0x2000 )
                    break;
                else
                {
                    psAudioLib->m_i16RecRPrevPcm = pi16I2sRecPcmBuf[psAudioLib->m_u32RecRPcmWorkBufIdx];
                    psAudioLib->m_u32RecRInterpoResidual -= 0x2000;

                    psAudioLib->m_u32RecRPcmWorkBufIdx += 2;
                    if ( psAudioLib->m_u32RecRPcmWorkBufIdx >= RING_BUF_16CNT )
                    {
                        psAudioLib->m_u32RecRPcmWorkBufIdx = 1;
                    }
                }
            }

            pi16RecPcmWorkBuf[psAudioLib->m_u32RecPcmWorkBufIdx++] = i16Right;

            if ( psAudioLib->m_u32RecPcmWorkBufIdx >= RING_BUF_16CNT )
                psAudioLib->m_u32RecPcmWorkBufIdx = 0;

            pi16RecPcmWorkBuf[psAudioLib->m_u32RecPcmWorkBufIdx++] = i16Left;

            if ( psAudioLib->m_u32RecPcmWorkBufIdx >= RING_BUF_16CNT )
                psAudioLib->m_u32RecPcmWorkBufIdx = 0;
        }

        i32Amount = (int32_t)(u32I2sRxDmaCurIdx - psAudioLib->m_u32RecLPcmWorkBufIdx);

        if ( i32Amount < 0 )
        {
            i32Amount += RING_BUF_16CNT;
        }

        if ( i32Amount > 864 )
        {
            psAudioLib->m_u32RecInterpoFactor        = psAudioLib->m_u32RecInterpoFactor2 + 1;
            //psAudioLib->m_u32RecResAdjustSpeedEnable = 1;
            //printf("%d> 864R\n", i32Amount);
        }

        //if ( psAudioLib->m_u32RecResAdjustSpeedEnable )
        //{
        if ( i32Amount < 288 )
        {
            psAudioLib->m_u32RecInterpoFactor = psAudioLib->m_u32RecInterpoFactor2 - 1;
            //printf("%d< 288R\n", i32Amount);
        }
        //}
#if 0
        if ( s_u32TimerCnt != g_u32TimerCnt )
        {
            s_u32TimerCnt = g_u32TimerCnt;
            if ( i32Amount != 0 )
                printf("%d\n", i32Amount);
            //if ( i32Amount > 864 )
            //printf("%d\n", i32Amount);
        }
#endif
    }
}
#endif  // CONFIG_AUDIO_REC
