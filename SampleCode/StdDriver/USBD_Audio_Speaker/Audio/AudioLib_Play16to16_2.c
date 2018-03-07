#include "NUC505Series.h"

#include "AudioLib.h"
#include "AudioLib2.h"

#if CONFIG_AUDIO_PLAY
void _UAC_SpkRecvFrom16to16_2(S_AUDIO_LIB* psAudioLib, int32_t i32Len)
{
    /* executed in USB IRQ */
    
    int32_t i, i32PlayPcmTmpSmplCnt;
    
    int16_t *pi16PlayPcmWorkBuf;
    int16_t *pi16PlayPcmTmpBuf;
    
    pi16PlayPcmWorkBuf = (int16_t *)psAudioLib->m_pu8PlayPcmWorkBuf;
    pi16PlayPcmTmpBuf  = (int16_t *)psAudioLib->m_pu8PlayPcmTmpBuf;
    
    i32PlayPcmTmpSmplCnt = psAudioLib->m_i32PlayPcmTmpBufLen >> 1;
    
    for ( i = 0; i < i32PlayPcmTmpSmplCnt; i ++ )
    {
        pi16PlayPcmWorkBuf[psAudioLib->m_u32PlayPcmWorkBufIdx2++] = pi16PlayPcmTmpBuf[i];
        
        if ( psAudioLib->m_u32PlayPcmWorkBufIdx2 >= RING_BUF_16CNT )
            psAudioLib->m_u32PlayPcmWorkBufIdx2 = 0;
    }
    
    psAudioLib->m_i32PlayPcmWorkSmplCnt += i32PlayPcmTmpSmplCnt;
    psAudioLib->m_i32PlayPcmTmpBufLen    = i32Len;
}
#endif  // CONFIG_AUDIO_PLAY
