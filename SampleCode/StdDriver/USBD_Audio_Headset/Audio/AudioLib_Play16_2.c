#include "NUC505Series.h"

#include "AudioLib.h"
#include "AudioLib2.h"

#if CONFIG_AUDIO_PLAY
//extern uint32_t g_u32TimerCnt;

//__align(4) static volatile uint32_t s_u32TimerCnt;

/* note re-sample to higher sampling rate causes poor audio quality and more MIPS
   re-sample to lower sampling rate causes poor auido quality
*/
void _UAC_SpkCopyFrom16_2(S_AUDIO_LIB* psAudioLib)
{
    /* executed in main loop */

    int32_t i, i32PlayPcmWorkSmplCnt, i32Amount;

    int16_t i16Left, i16Right;

    int16_t *pi16I2sPlayPcmBuf;
    int16_t *pi16PlayPcmWorkBuf;

    if ( psAudioLib->m_i32PlayPcmWorkSmplCnt >= psAudioLib->m_u16PlaySmplCnt1 )
    {
        i32PlayPcmWorkSmplCnt = psAudioLib->m_u16PlaySmplCnt1;

        if ( psAudioLib->m_i32PlayPcmWorkSmplCnt >= psAudioLib->m_u16PlaySmplCnt2 )
        {
            i32PlayPcmWorkSmplCnt = psAudioLib->m_u16PlaySmplCnt2;
        }

        NVIC_DisableIRQ(USBD_IRQn);
        psAudioLib->m_i32PlayPcmWorkSmplCnt -= i32PlayPcmWorkSmplCnt;
        NVIC_EnableIRQ(USBD_IRQn);

        pi16I2sPlayPcmBuf  = (int16_t *)psAudioLib->m_pu8I2sPlayPcmBuf;
        pi16PlayPcmWorkBuf = (int16_t *)psAudioLib->m_pu8PlayPcmWorkBuf;

        for ( i = 0; i < 96; i += 2 )
        {
            i16Left  = (((int32_t)psAudioLib->m_i16PlayLPrevPcm * (0x2000 - psAudioLib->m_u32PlayLInterpoResidual))+
                        ((int32_t)pi16PlayPcmWorkBuf[psAudioLib->m_u32PlayLPcmWorkBufIdx] * psAudioLib->m_u32PlayLInterpoResidual)) >> 13;
            psAudioLib->m_u32PlayLInterpoResidual += psAudioLib->m_u32PlayInterpoFactor;
            while ( 1 )
            {
                if ( psAudioLib->m_u32PlayLInterpoResidual < 0x2000 )
                    break;
                else
                {
                    psAudioLib->m_i16PlayLPrevPcm = pi16PlayPcmWorkBuf[psAudioLib->m_u32PlayLPcmWorkBufIdx];
                    psAudioLib->m_u32PlayLInterpoResidual -= 0x2000;

                    psAudioLib->m_u32PlayLPcmWorkBufIdx += 2;
                    if ( psAudioLib->m_u32PlayLPcmWorkBufIdx >= RING_BUF_16CNT )
                    {
                        psAudioLib->m_u32PlayLPcmWorkBufIdx = 0;
                    }
                }
            }

            i16Right = (((int32_t)psAudioLib->m_i16PlayRPrevPcm * (0x2000 - psAudioLib->m_u32PlayRInterpoResidual))+
                        ((int32_t)pi16PlayPcmWorkBuf[psAudioLib->m_u32PlayRPcmWorkBufIdx] * psAudioLib->m_u32PlayRInterpoResidual)) >> 13;
            psAudioLib->m_u32PlayRInterpoResidual += psAudioLib->m_u32PlayInterpoFactor;
            while ( 1 )
            {
                if ( psAudioLib->m_u32PlayRInterpoResidual < 0x2000 )
                    break;
                else
                {
                    psAudioLib->m_i16PlayRPrevPcm = pi16PlayPcmWorkBuf[psAudioLib->m_u32PlayRPcmWorkBufIdx];
                    psAudioLib->m_u32PlayRInterpoResidual -= 0x2000;

                    psAudioLib->m_u32PlayRPcmWorkBufIdx += 2;
                    if ( psAudioLib->m_u32PlayRPcmWorkBufIdx >= RING_BUF_16CNT )
                    {
                        psAudioLib->m_u32PlayRPcmWorkBufIdx = 1;
                    }
                }
            }

            pi16I2sPlayPcmBuf[psAudioLib->m_u32I2sPlayPcmBufIdx++] = i16Right;

            if ( psAudioLib->m_u32I2sPlayPcmBufIdx >= RING_BUF_16CNT )
                psAudioLib->m_u32I2sPlayPcmBufIdx = 0;

            pi16I2sPlayPcmBuf[psAudioLib->m_u32I2sPlayPcmBufIdx++] = i16Left;

            if ( psAudioLib->m_u32I2sPlayPcmBufIdx >= RING_BUF_16CNT )
                psAudioLib->m_u32I2sPlayPcmBufIdx = 0;
        }

        i32Amount = (int32_t)(psAudioLib->m_u32PlayPcmWorkBufIdx2 - psAudioLib->m_u32PlayLPcmWorkBufIdx);

        if ( i32Amount < 0 )
        {
            i32Amount += RING_BUF_16CNT;
        }

        if ( i32Amount > psAudioLib->m_u16I2sSmplCntPlayTooSlow )
        {
            psAudioLib->m_u32PlayInterpoFactor        = psAudioLib->m_u32PlayInterpoFactor2 + 1;
            psAudioLib->m_u32PlayResAdjustSpeedEnable = 1;
            //printf("\t%d> %d P\n", i32Amount, psAudioLib->m_u16I2sSmplCntPlayTooSlow);
        }

        if ( psAudioLib->m_u32PlayResAdjustSpeedEnable )
        {
            if ( i32Amount < psAudioLib->m_u16I2sSmplCntPlayTooFast )
            {
                psAudioLib->m_u32PlayInterpoFactor = psAudioLib->m_u32PlayInterpoFactor2 - 1;
                //printf("\t%d< %d P\n", i32Amount, psAudioLib->m_u16I2sSmplCntPlayTooFast);
            }
        }
#if 0
        if ( s_u32TimerCnt != g_u32TimerCnt )
        {
            s_u32TimerCnt = g_u32TimerCnt;
            //if ( psAudioLib->m_i32PlayPcmWorkSmplCnt != 0 )
            //printf("%dP\n", psAudioLib->m_i32PlayPcmWorkSmplCnt);
            if ( i32Amount != 0 )
                printf("\t%dP\n", i32Amount);
        }
#endif
        /* start playback when ring buffer ready */
        if ( psAudioLib->m_u8PlayFlag == 0 )
        {
            //printf("\tP %d\n", psAudioLib->m_u32I2sPlayPcmBufIdx);
            if ( psAudioLib->m_u32I2sPlayPcmBufIdx >= psAudioLib->m_u16I2sSmplCntPlayStart )
            {
                psAudioLib->m_u8PlayFlag = 1;
                /* Enable I2S Tx function */
                //I2S_CLR_TX_FIFO(I2S);
                I2S_ENABLE_TXDMA(I2S);
                I2S_ENABLE_TX(I2S);
            }
        }
    }
}
#endif  // CONFIG_AUDIO_PLAY
