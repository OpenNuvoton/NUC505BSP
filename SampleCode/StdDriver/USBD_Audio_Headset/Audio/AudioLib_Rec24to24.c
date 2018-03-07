#include "NUC505Series.h"

#include "AudioLib.h"
#include "AudioLib2.h"

#if CONFIG_AUDIO_REC
void _UAC_MicSendTo24to24(S_AUDIO_LIB* psAudioLib)
{
    /* executed in USB IRQ */
    
    int32_t i, j, i32RecPcmWorkSmplCnt;
    int32_t i32Smpl1, i32Smpl2;
    
    int32_t *pi32RecPcmWorkBuf;
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
    pu8RecPcmTmpBuf   = (uint8_t *)psAudioLib->m_pu8RecPcmTmpBuf;
    
    for ( i = 0, j = 0; i < i32RecPcmWorkSmplCnt; i += 2 )
    {
        i32Smpl1 = pi32RecPcmWorkBuf[psAudioLib->m_u32RecPcmWorkBufIdx2++];
        
        if ( psAudioLib->m_u32RecPcmWorkBufIdx2 >= RING_BUF_32CNT )
            psAudioLib->m_u32RecPcmWorkBufIdx2 = 0;
        
        i32Smpl2 = pi32RecPcmWorkBuf[psAudioLib->m_u32RecPcmWorkBufIdx2++];
        
        if ( psAudioLib->m_u32RecPcmWorkBufIdx2 >= RING_BUF_32CNT )
            psAudioLib->m_u32RecPcmWorkBufIdx2 = 0;
        
        if ( psAudioLib->m_u8RecChannels == 1 )
        {
            pu8RecPcmTmpBuf[j++] = (((i32Smpl1 + i32Smpl2) >> 1) & 0x0000FF00) >>  8;
            pu8RecPcmTmpBuf[j++] = (((i32Smpl1 + i32Smpl2) >> 1) & 0x00FF0000) >> 16;
            pu8RecPcmTmpBuf[j++] = (((i32Smpl1 + i32Smpl2) >> 1) & 0xFF000000) >> 24;
        }
        else
        {
            pu8RecPcmTmpBuf[j++] = (i32Smpl1 & 0x0000FF00) >>  8;
            pu8RecPcmTmpBuf[j++] = (i32Smpl1 & 0x00FF0000) >> 16;
            pu8RecPcmTmpBuf[j++] = (i32Smpl1 & 0xFF000000) >> 24;
            
            pu8RecPcmTmpBuf[j++] = (i32Smpl2 & 0x0000FF00) >>  8;
            pu8RecPcmTmpBuf[j++] = (i32Smpl2 & 0x00FF0000) >> 16;
            pu8RecPcmTmpBuf[j++] = (i32Smpl2 & 0xFF000000) >> 24;
        }
    }
}
#endif  // CONFIG_AUDIO_REC
