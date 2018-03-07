#include "NUC505Series.h"

#include "AudioLib.h"
#include "AudioLib2.h"

#if CONFIG_AUDIO_PLAY
void _UAC_SpkCopyFrom16(S_AUDIO_LIB* psAudioLib)
{
    /* executed in main loop */
    
    int32_t i, i32PlayPcmWorkSmplCnt;
    
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
        
        for ( i = 0; i < i32PlayPcmWorkSmplCnt; i += 2 )
        {
            i16Left  = pi16PlayPcmWorkBuf[psAudioLib->m_u32PlayPcmWorkBufIdx++];
            
            if ( psAudioLib->m_u32PlayPcmWorkBufIdx >= RING_BUF_16CNT )
                psAudioLib->m_u32PlayPcmWorkBufIdx = 0;
            
            i16Right = pi16PlayPcmWorkBuf[psAudioLib->m_u32PlayPcmWorkBufIdx++];
            
            if ( psAudioLib->m_u32PlayPcmWorkBufIdx >= RING_BUF_16CNT )
                psAudioLib->m_u32PlayPcmWorkBufIdx = 0;
            
            pi16I2sPlayPcmBuf[psAudioLib->m_u32I2sPlayPcmBufIdx++] = i16Right;
            
            if ( psAudioLib->m_u32I2sPlayPcmBufIdx >= RING_BUF_16CNT )
                psAudioLib->m_u32I2sPlayPcmBufIdx = 0;
            
            pi16I2sPlayPcmBuf[psAudioLib->m_u32I2sPlayPcmBufIdx++] = i16Left;
            
            if ( psAudioLib->m_u32I2sPlayPcmBufIdx >= RING_BUF_16CNT )
                psAudioLib->m_u32I2sPlayPcmBufIdx = 0;
        }
        
        /* start playback when ring buffer ready */
        if ( psAudioLib->m_u8PlayFlag == 0 )
        {
            //printf("\tp %d\n", psAudioLib->m_u32I2sPlayPcmBufIdx);
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
