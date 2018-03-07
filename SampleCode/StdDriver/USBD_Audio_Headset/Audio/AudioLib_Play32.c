#include "NUC505Series.h"

#include "AudioLib.h"
#include "AudioLib2.h"

#if CONFIG_AUDIO_PLAY
void _UAC_SpkCopyFrom32(S_AUDIO_LIB* psAudioLib)
{
    /* executed in main loop */
    
    int32_t i, i32PlayPcmWorkSmplCnt;
    int32_t i32Left, i32Right;
    
    int32_t *pi32I2sPlayPcmBuf;
    int32_t *pi32PlayPcmWorkBuf;
    
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
        
        pi32I2sPlayPcmBuf  = (int32_t *)psAudioLib->m_pu8I2sPlayPcmBuf;
        pi32PlayPcmWorkBuf = (int32_t *)psAudioLib->m_pu8PlayPcmWorkBuf;
        
        for ( i = 0; i < i32PlayPcmWorkSmplCnt; i += 2 )
        {
            i32Left  = pi32PlayPcmWorkBuf[psAudioLib->m_u32PlayPcmWorkBufIdx++];
            
            if ( psAudioLib->m_u32PlayPcmWorkBufIdx >= RING_BUF_32CNT )
                psAudioLib->m_u32PlayPcmWorkBufIdx = 0;
            
            i32Right = pi32PlayPcmWorkBuf[psAudioLib->m_u32PlayPcmWorkBufIdx++];
            
            if ( psAudioLib->m_u32PlayPcmWorkBufIdx >= RING_BUF_32CNT )
                psAudioLib->m_u32PlayPcmWorkBufIdx = 0;
            
            pi32I2sPlayPcmBuf[psAudioLib->m_u32I2sPlayPcmBufIdx++] = i32Left;
            
            if ( psAudioLib->m_u32I2sPlayPcmBufIdx >= RING_BUF_32CNT )
                psAudioLib->m_u32I2sPlayPcmBufIdx = 0;
            
            pi32I2sPlayPcmBuf[psAudioLib->m_u32I2sPlayPcmBufIdx++] = i32Right;
            
            if ( psAudioLib->m_u32I2sPlayPcmBufIdx >= RING_BUF_32CNT )
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
