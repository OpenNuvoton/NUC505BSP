#include "NUC505Series.h"

#include "AudioLib.h"
#include "AudioLib2.h"

#if CONFIG_AUDIO_REC
void _UAC_MicCopyTo32(S_AUDIO_LIB* psAudioLib)
{
    /* executed in main loop */
    
    int32_t  i, i32Amount;
    uint32_t u32I2sRxDmaCurIdx;
    int32_t i32Left, i32Right;
    
    int32_t *pi32I2sRecPcmBuf;
    int32_t *pi32RecPcmWorkBuf;
    
    u32I2sRxDmaCurIdx = ((uint32_t)I2S_GET_RXDMA_CADDR(I2S) - (uint32_t)psAudioLib->m_pu8I2sRecPcmBuf) >> 2;
    
    if ( psAudioLib->m_u8RecFlag2 == 0 )
    {
        psAudioLib->m_u8RecFlag2 = 1;
        
        i32Amount = psAudioLib->m_u16I2sSmplCntRecStart;
        
        if ( u32I2sRxDmaCurIdx >= i32Amount )
        {
            psAudioLib->m_u32I2sRecPcmBufIdx = u32I2sRxDmaCurIdx - i32Amount;
            //printf("+ %d %d %d\n", u32I2sRxDmaCurIdx, psAudioLib->m_u32I2sRecPcmBufIdx, i32Amount);
        }
        else
        {
            psAudioLib->m_u32I2sRecPcmBufIdx = RING_BUF_32CNT + (int32_t)(u32I2sRxDmaCurIdx - i32Amount);
            //printf("- %d %d %d\n", u32I2sRxDmaCurIdx, psAudioLib->m_u32I2sRecPcmBufIdx, i32Amount);
        }
    }
    else
        i32Amount = (int32_t)(u32I2sRxDmaCurIdx - psAudioLib->m_u32I2sRecPcmBufPreIdx);
    
    if ( i32Amount < 0 )
    {
        i32Amount += RING_BUF_32CNT;
    }
    
    if ( i32Amount % 2 )
        return;
    
    psAudioLib->m_u32I2sRecPcmBufPreIdx = u32I2sRxDmaCurIdx;
    
    pi32I2sRecPcmBuf  = (int32_t *)psAudioLib->m_pu8I2sRecPcmBuf;
    pi32RecPcmWorkBuf = (int32_t *)psAudioLib->m_pu8RecPcmWorkBuf;
    
    for ( i = 0; i < i32Amount; i += 2 )
    {
        i32Left  = pi32I2sRecPcmBuf[psAudioLib->m_u32I2sRecPcmBufIdx++];
        
        if ( psAudioLib->m_u32I2sRecPcmBufIdx >= RING_BUF_32CNT )
            psAudioLib->m_u32I2sRecPcmBufIdx = 0;
        
        i32Right = pi32I2sRecPcmBuf[psAudioLib->m_u32I2sRecPcmBufIdx++];
        
        if ( psAudioLib->m_u32I2sRecPcmBufIdx >= RING_BUF_32CNT )
            psAudioLib->m_u32I2sRecPcmBufIdx = 0;
        
        pi32RecPcmWorkBuf[psAudioLib->m_u32RecPcmWorkBufIdx++] = i32Left;
        
        if ( psAudioLib->m_u32RecPcmWorkBufIdx >= RING_BUF_32CNT )
            psAudioLib->m_u32RecPcmWorkBufIdx = 0;
        
        pi32RecPcmWorkBuf[psAudioLib->m_u32RecPcmWorkBufIdx++] = i32Right;
        
        if ( psAudioLib->m_u32RecPcmWorkBufIdx >= RING_BUF_32CNT )
            psAudioLib->m_u32RecPcmWorkBufIdx = 0;
    }
}
#endif  // CONFIG_AUDIO_REC
