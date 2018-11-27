#include "NUC505Series.h"

#include "AudioLib.h"
#include "AudioLib2.h"

#if CONFIG_AUDIO_PLAY
//extern uint32_t g_u32TimerCnt;

//__align(4) static volatile uint32_t s_u32TimerCnt;

static void _UAC_SpkStop(S_AUDIO_LIB* psAudioLib)
{
    /* executed in USB IRQ */

    /* back to default APLL */
    if ( psAudioLib->m_u32I2sRefApll != psAudioLib->m_u32I2sApll )
    {
        psAudioLib->m_u32I2sRefApll = psAudioLib->m_u32I2sApll;
        CLK_SET_APLL(psAudioLib->m_u32I2sRefApll);
    }

    I2S_DISABLE_TXDMA(I2S);
    I2S_DISABLE_TX(I2S);
    I2S_CLR_TX_FIFO(I2S);

    psAudioLib->m_u8PlayFlag              = 0;
    psAudioLib->m_u32I2sPlayPcmBufIdx     = 0;
    psAudioLib->m_u32PlayPcmWorkBufIdx    = 0;
    psAudioLib->m_u32PlayPcmWorkBufIdx2   = 0;
    psAudioLib->m_i32PlayPcmTmpBufLen     = 0;
    psAudioLib->m_i32PlayPcmWorkSmplCnt   = 0;
}

static void _UAC_SpkSpeed(S_AUDIO_LIB* psAudioLib)
{
    /* executed in main loop */

    int32_t i32Len;

    /* note workaround host send no data to device causes audio DMA playing dirty data */
    if ( psAudioLib->m_u8PlayFlag )
    {
        /* i32Len is byte size */
        i32Len  = (psAudioLib->m_u32I2sPlayPcmBufIdx << psAudioLib->m_u8I2sShiftFlag) + (uint32_t)psAudioLib->m_pu8I2sPlayPcmBuf;

        i32Len -= (int32_t)I2S_GET_TXDMA_CADDR(I2S);

        if ( i32Len < 0 )
        {
            i32Len += RING_BUF_SZ;
        }

        /* i32Len is sample count */
        i32Len >>= psAudioLib->m_u8I2sShiftFlag;
#if 0
        if ( s_u32TimerCnt != g_u32TimerCnt )
        {
            s_u32TimerCnt = g_u32TimerCnt;
            printf("\t%d\n", i32Len);
        }
#endif
        if ( i32Len <= psAudioLib->m_u16I2sSmplCntPlayTooFast )
        {
            if ( psAudioLib->m_u32I2sRefApll != psAudioLib->m_u32I2sApllDn )
            {
                CLK_SET_APLL(psAudioLib->m_u32I2sApllDn);
                psAudioLib->m_u32I2sRefApll = psAudioLib->m_u32I2sApllDn;
                //printf("\t%d<=%d\n", i32Len, psAudioLib->m_u16I2sSmplCntPlayTooFast);
            }
            //else
            //printf("\t%d<=%d\n", i32Len, psAudioLib->m_u16I2sSmplCntPlayTooFast);
        }

        if ( i32Len <= psAudioLib->m_u16I2sSmplCntPlayTooFastStop )
        {
            //NVIC_DisableIRQ(USBD_IRQn);
            _UAC_SpkStop( psAudioLib );
            //printf("\t%d<<=%d\n", i32Len, psAudioLib->m_u16I2sSmplCntPlayTooFastStop);
            //NVIC_EnableIRQ(USBD_IRQn);
        }

        if ( i32Len >= psAudioLib->m_u16I2sSmplCntPlayTooSlow )
        {
            if ( psAudioLib->m_u32I2sRefApll != psAudioLib->m_u32I2sApllUp )
            {
                CLK_SET_APLL(psAudioLib->m_u32I2sApllUp);
                psAudioLib->m_u32I2sRefApll = psAudioLib->m_u32I2sApllUp;
                //printf("\t%d>=%d\n", i32Len, psAudioLib->m_u16I2sSmplCntPlayTooSlow);
            }
            //else
            //printf("\t%d>=%d\n", i32Len, psAudioLib->m_u16I2sSmplCntPlayTooSlow);
        }

        if ( i32Len >= psAudioLib->m_u16I2sSmplCntPlayTooSlowStop )
        {
            //NVIC_DisableIRQ(USBD_IRQn);
            _UAC_SpkStop( psAudioLib );
            //printf("\t%d>>=%d\n", i32Len, psAudioLib->m_u16I2sSmplCntPlayTooSlowStop);
            //NVIC_EnableIRQ(USBD_IRQn);
        }
    }
}

static void _UAC_SpkConfigMaxPayload10(S_AUDIO_LIB* psAudioLib)
{
    /* executed in USB IRQ */

    uint32_t u32I2sBitRate;

    if ( psAudioLib->m_u8PlayBitRate == 16 )
    {
        psAudioLib->m_pfnPlayMode1 = _UAC_SpkRecvFrom16to16;
        psAudioLib->m_pfnPlayMode2 = _UAC_SpkCopyFrom16;
        u32I2sBitRate              = I2S_DATABIT_16;
    }
    else if ( psAudioLib->m_u8PlayBitRate == 24 )
    {
        psAudioLib->m_pfnPlayMode1 = _UAC_SpkRecvFrom24to24;
        psAudioLib->m_pfnPlayMode2 = _UAC_SpkCopyFrom32;
        u32I2sBitRate              = I2S_DATABIT_32;    /* note for 24-bit we use I2S_DATABIT_32 instead */
    }
    else
    {
        psAudioLib->m_pfnPlayMode1 = _UAC_SpkRecvFrom32to32;
        psAudioLib->m_pfnPlayMode2 = _UAC_SpkCopyFrom32;
        u32I2sBitRate              = I2S_DATABIT_32;
    }

    if ( (psAudioLib->m_u32I2sSampleRate != psAudioLib->m_u32PlaySampleRate) ||
            (psAudioLib->m_u32I2sBitRate    != u32I2sBitRate) )
    {
        AudioLib_Init3( psAudioLib, psAudioLib->m_u32PlaySampleRate, u32I2sBitRate );
    }

    psAudioLib->m_u16PlaySmplCnt1 = psAudioLib->m_u32PlaySampleRate / 1000;

    switch ( psAudioLib->m_u32PlaySampleRate )
    {
    case  11025:
        psAudioLib->m_u16PlaySmplCnt2 =  12;
        break;
    case  22050:
        psAudioLib->m_u16PlaySmplCnt2 =  23;
        break;
    case  44100:
        psAudioLib->m_u16PlaySmplCnt2 =  45;
        break;
    case  88200:
        psAudioLib->m_u16PlaySmplCnt2 =  90;
        break;
    case 176400:
        psAudioLib->m_u16PlaySmplCnt2 = 180;
        break;
    default:
        psAudioLib->m_u16PlaySmplCnt2 = psAudioLib->m_u16PlaySmplCnt1;
        break;
    }

    psAudioLib->m_u8PlaySmplSize        = psAudioLib->m_u8PlayBitRate >> 3;
    psAudioLib->m_u16PlayMaxPayload11   = psAudioLib->m_u16PlaySmplCnt1 * psAudioLib->m_u8PlayChannels * psAudioLib->m_u8PlaySmplSize;
    psAudioLib->m_u16PlayMaxPayload12   = psAudioLib->m_u16PlaySmplCnt2 * psAudioLib->m_u8PlayChannels * psAudioLib->m_u8PlaySmplSize;
    psAudioLib->m_u16PlayMaxPayload1_   = psAudioLib->m_u16PlayMaxPayload11;
    psAudioLib->m_u16PlayMaxPayload2_   = psAudioLib->m_u16PlayMaxPayload12;
    psAudioLib->m_u16PlaySmplCnt1     <<= 1;
    psAudioLib->m_u16PlaySmplCnt2     <<= 1;
    psAudioLib->m_u16PlaySmplCnt1_      = psAudioLib->m_u16PlaySmplCnt1;
    psAudioLib->m_u16PlaySmplCnt2_      = psAudioLib->m_u16PlaySmplCnt2;
}

static void _UAC_SpkConfigMaxPayload20(S_AUDIO_LIB* psAudioLib)
{
    /* executed in USB IRQ */

    _UAC_SpkConfigMaxPayload10( psAudioLib );

    psAudioLib->m_u16PlayMaxPayload21 = psAudioLib->m_u16PlayMaxPayload11 >> 3;
    psAudioLib->m_u16PlayMaxPayload22 = psAudioLib->m_u16PlayMaxPayload12 >> 3;
    psAudioLib->m_u16PlayMaxPayload1_ = psAudioLib->m_u16PlayMaxPayload21;
    psAudioLib->m_u16PlayMaxPayload2_ = psAudioLib->m_u16PlayMaxPayload22;
    psAudioLib->m_u16PlaySmplCnt1_    = psAudioLib->m_u16PlaySmplCnt1 >> 3;
    psAudioLib->m_u16PlaySmplCnt2_    = psAudioLib->m_u16PlaySmplCnt2 >> 3;

    switch ( psAudioLib->m_u32PlaySampleRate )
    {
    case  11025:
        psAudioLib->m_u16PlayMaxPayload2_ = ( 24 * psAudioLib->m_u8PlayChannels * psAudioLib->m_u8PlaySmplSize) >> 3;
        break;
    case  22050:
        psAudioLib->m_u16PlayMaxPayload2_ = ( 24 * psAudioLib->m_u8PlayChannels * psAudioLib->m_u8PlaySmplSize) >> 3;
        break;
    case  44100:
        psAudioLib->m_u16PlayMaxPayload2_ = ( 48 * psAudioLib->m_u8PlayChannels * psAudioLib->m_u8PlaySmplSize) >> 3;
        break;
    case  88200:
        psAudioLib->m_u16PlayMaxPayload2_ = ( 96 * psAudioLib->m_u8PlayChannels * psAudioLib->m_u8PlaySmplSize) >> 3;
        break;
    case 176400:
        psAudioLib->m_u16PlayMaxPayload2_ = (192 * psAudioLib->m_u8PlayChannels * psAudioLib->m_u8PlaySmplSize) >> 3;
        break;
    default:
        break;
    }
}

void AudioLib_InitPlayCallback(S_AUDIO_LIB* psAudioLib)
{
    psAudioLib->m_pfnPlaySpeed              = _UAC_SpkSpeed;
    psAudioLib->m_pfnPlayStop               = _UAC_SpkStop;
    psAudioLib->m_pfnPlayConfigMaxPayload10 = _UAC_SpkConfigMaxPayload10;
    psAudioLib->m_pfnPlayConfigMaxPayload20 = _UAC_SpkConfigMaxPayload20;
}
#endif  // CONFIG_AUDIO_PLAY
