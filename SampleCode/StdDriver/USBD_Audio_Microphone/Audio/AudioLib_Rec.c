#include "NUC505Series.h"

#include "AudioLib.h"
#include "AudioLib2.h"

#if CONFIG_AUDIO_REC
//extern uint32_t g_u32TimerCnt;

//__align(4) static volatile uint32_t s_u32TimerCnt;

static void _UAC_MicStop(S_AUDIO_LIB* psAudioLib)
{
    /* executed in USB IRQ */
    
    /* back to default APLL */
    if ( psAudioLib->m_u32I2sRefApll != psAudioLib->m_u32I2sApll )
    {
        psAudioLib->m_u32I2sRefApll = psAudioLib->m_u32I2sApll;
        CLK_SET_APLL(psAudioLib->m_u32I2sRefApll);
    }
    
    psAudioLib->m_u16RecPacketSequenceIdx = 0;
    psAudioLib->m_u8RecFlag               = 0;
}

void _UAC_MicStop_2(S_AUDIO_LIB* psAudioLib)
{
    /* executed in main loop */
    
    psAudioLib->m_u32RecInterpoFactor           = (0x2000 * _I2S_SAMPLE_RATE) / psAudioLib->m_u32RecSampleRate;
    psAudioLib->m_u32RecInterpoFactor2          = psAudioLib->m_u32RecInterpoFactor;
    psAudioLib->m_i16RecLPrevPcm                = 0;
    psAudioLib->m_i16RecRPrevPcm                = 0;
    psAudioLib->m_u32RecLInterpoResidual        = 0;
    psAudioLib->m_u32RecRInterpoResidual        = 0;
    psAudioLib->m_i32RecPcmWorkSmplCnt          = 0;
    psAudioLib->m_u8RecCnt2                     = 0;
    psAudioLib->m_u32RecPcmWorkBufIdx           = 0;
    psAudioLib->m_u8RecFlag2                    = 0;
    //psAudioLib->m_u32RecResAdjustSpeedEnable    = 0;
    psAudioLib->m_i32RecPcmWorkResSmplCnt       = 0;
    
    //printf("dr--\n");
}

static void _UAC_MicSpeed(S_AUDIO_LIB* psAudioLib)
{
    /* executed in main loop */
    
    int32_t i32Len;
    
    if ( psAudioLib->m_u8RecFlag )
    {
        /* i32Len is sample count */
        i32Len = (int32_t)psAudioLib->m_u32RecPcmWorkBufIdx - (int32_t)psAudioLib->m_u32RecPcmWorkBufIdx2;
        
        if ( i32Len < 0 )
        {
            i32Len += psAudioLib->m_i32RecFlag;
        }
        #if 0
        if ( s_u32TimerCnt != g_u32TimerCnt )
        {
            s_u32TimerCnt = g_u32TimerCnt;
            printf("%d\n", i32Len);
        }
        #endif
        if ( i32Len >= psAudioLib->m_u16I2sSmplCntRecTooFast )
        {
            if ( psAudioLib->m_u32I2sRefApll != psAudioLib->m_u32I2sApllDn )
            {
                CLK_SET_APLL(psAudioLib->m_u32I2sApllDn);
                psAudioLib->m_u32I2sRefApll = psAudioLib->m_u32I2sApllDn;
                //printf("%d>=%d\n", i32Len, psAudioLib->m_u16I2sSmplCntRecTooFast);
            }
            //else
                //printf("r %d>=%d\n", i32Len, psAudioLib->m_u16I2sSmplCntRecTooFast);
        }
        
        if ( i32Len >= psAudioLib->m_u16I2sSmplCntRecTooFastStop )
        {
            //NVIC_DisableIRQ(USBD_IRQn);
            _UAC_MicStop( psAudioLib );
            //printf("%d>>=%d\n", i32Len, psAudioLib->m_u16I2sSmplCntRecTooFastStop);
            //NVIC_EnableIRQ(USBD_IRQn);
        }
        
        if ( i32Len <= psAudioLib->m_u16I2sSmplCntRecTooSlow )
        {
            if ( psAudioLib->m_u32I2sRefApll != psAudioLib->m_u32I2sApllUp )
            {
                CLK_SET_APLL(psAudioLib->m_u32I2sApllUp);
                psAudioLib->m_u32I2sRefApll = psAudioLib->m_u32I2sApllUp;
                //printf("%d<=%d\n", i32Len, psAudioLib->m_u16I2sSmplCntRecTooSlow);
            }
            //else
                //printf("r %d<=%d\n", i32Len, psAudioLib->m_u16I2sSmplCntRecTooSlow);
        }
        
        if ( i32Len <= psAudioLib->m_u16I2sSmplCntRecTooSlowStop )
        {
            //NVIC_DisableIRQ(USBD_IRQn);
            _UAC_MicStop( psAudioLib );
            //printf("%d<<=%d\n", i32Len, psAudioLib->m_u16I2sSmplCntRecTooSlowStop);
            //NVIC_EnableIRQ(USBD_IRQn);
        }
    }
}

#define REC_PKT_SEQ1_00  0, 0, 0, 0, 0, 0, 0, 0, 0, 0

#ifdef __ICCARM__
#pragma data_alignment=4
static uint8_t s_au8RecPacketSequence1_0[] = {
#else   // __CC_ARM
static uint8_t s_au8RecPacketSequence1_0[] __attribute__((aligned(4))) = {
#endif
    REC_PKT_SEQ1_00
};

#define REC_PKT_SEQ1_10  0, 0, 0, 0, 0, 0, 0, 0, 0, 1

#ifdef __ICCARM__
#pragma data_alignment=4
static uint8_t s_au8RecPacketSequence1_1[] = {
#else   // __CC_ARM
static uint8_t s_au8RecPacketSequence1_1[] __attribute__((aligned(4))) = {
#endif
    REC_PKT_SEQ1_10
};

#define REC_PKT_SEQ1_20  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define REC_PKT_SEQ1_21  0, 0, 0, 0, 0, 0, 0, 0, 0, 1

#ifdef __ICCARM__
#pragma data_alignment=4
static uint8_t s_au8RecPacketSequence1_2[] = {
#else   // __CC_ARM
static uint8_t s_au8RecPacketSequence1_2[] __attribute__((aligned(4))) = {
#endif
    REC_PKT_SEQ1_20,
    REC_PKT_SEQ1_21
};

#define REC_PKT_SEQ1_30  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define REC_PKT_SEQ1_31  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define REC_PKT_SEQ1_32  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define REC_PKT_SEQ1_33  0, 0, 0, 0, 0, 0, 0, 0, 0, 1

#ifdef __ICCARM__
#pragma data_alignment=4
static uint8_t s_au8RecPacketSequence1_3[] = {
#else   // __CC_ARM
static uint8_t s_au8RecPacketSequence1_3[] __attribute__((aligned(4))) = {
#endif
    REC_PKT_SEQ1_30,
    REC_PKT_SEQ1_31,
    REC_PKT_SEQ1_32,
    REC_PKT_SEQ1_33
};

static void _UAC_MicConfigMaxPayload10(S_AUDIO_LIB* psAudioLib)
{
    /* executed in USB IRQ */
    
    uint32_t u32I2sBitRate;
    
    if ( psAudioLib->m_u8RecBitRate == 16 )
    {
        psAudioLib->m_pfnRecMode1 = _UAC_MicSendTo16to16;
        psAudioLib->m_pfnRecMode2 = _UAC_MicCopyTo16;
        psAudioLib->m_i32RecFlag  = RING_BUF_16CNT;
        u32I2sBitRate             = I2S_DATABIT_16;
    }
    else if ( psAudioLib->m_u8RecBitRate == 24 )
    {
        psAudioLib->m_pfnRecMode1 = _UAC_MicSendTo24to24;
        psAudioLib->m_pfnRecMode2 = _UAC_MicCopyTo32;
        psAudioLib->m_i32RecFlag  = RING_BUF_32CNT;
        u32I2sBitRate             = I2S_DATABIT_32;     /* note for 24-bit we use I2S_DATABIT_32 instead */
    }
    else
    {
        psAudioLib->m_pfnRecMode1 = _UAC_MicSendTo32to32;
        psAudioLib->m_pfnRecMode2 = _UAC_MicCopyTo32;
        psAudioLib->m_i32RecFlag  = RING_BUF_32CNT;
        u32I2sBitRate             = I2S_DATABIT_32;
    }
    
    if ( (psAudioLib->m_u32I2sSampleRate != psAudioLib->m_u32RecSampleRate) ||
         (psAudioLib->m_u32I2sBitRate    != u32I2sBitRate) )
    {
        AudioLib_Init3( psAudioLib, psAudioLib->m_u32RecSampleRate, u32I2sBitRate );
    }
    
    psAudioLib->m_u16RecSmplCnt1 = psAudioLib->m_u32RecSampleRate / 1000;
    
    switch ( psAudioLib->m_u32RecSampleRate )
    {
        case  11025:
            psAudioLib->m_u16RecSmplCnt2 =  12;
            psAudioLib->m_u16RecPacketSequenceCnt = 40-2;
            psAudioLib->m_pu8RecPacketSequence    = s_au8RecPacketSequence1_3;
            break;
        case  22050:
            psAudioLib->m_u16RecSmplCnt2 =  23;
            psAudioLib->m_u16RecPacketSequenceCnt = 20-2;
            psAudioLib->m_pu8RecPacketSequence    = s_au8RecPacketSequence1_2;
            break;
        case  44100:
            psAudioLib->m_u16RecSmplCnt2 =  45;
            psAudioLib->m_u16RecPacketSequenceCnt = 10-2;
            psAudioLib->m_pu8RecPacketSequence    = s_au8RecPacketSequence1_1;
            break;
        case  88200:
            psAudioLib->m_u16RecSmplCnt2 =  90;
            psAudioLib->m_u16RecPacketSequenceCnt = 10-2;
            psAudioLib->m_pu8RecPacketSequence    = s_au8RecPacketSequence1_1;
            break;
        case 176400:
            psAudioLib->m_u16RecSmplCnt2 = 180;
            psAudioLib->m_u16RecPacketSequenceCnt = 10-2;
            psAudioLib->m_pu8RecPacketSequence    = s_au8RecPacketSequence1_1;
            break;
        default:
            psAudioLib->m_u16RecSmplCnt2 = psAudioLib->m_u16RecSmplCnt1;
            psAudioLib->m_u16RecPacketSequenceCnt = 10-2;
            psAudioLib->m_pu8RecPacketSequence    = s_au8RecPacketSequence1_0;
            break;
    }
    
    psAudioLib->m_u8RecSmplSize        = psAudioLib->m_u8RecBitRate >> 3;
    psAudioLib->m_u16RecMaxPayload11   = psAudioLib->m_u16RecSmplCnt1 * psAudioLib->m_u8RecChannels * psAudioLib->m_u8RecSmplSize;
    psAudioLib->m_u16RecMaxPayload12   = psAudioLib->m_u16RecSmplCnt2 * psAudioLib->m_u8RecChannels * psAudioLib->m_u8RecSmplSize;
    psAudioLib->m_u16RecMaxPayload1_   = psAudioLib->m_u16RecMaxPayload11;
    psAudioLib->m_u16RecMaxPayload2_   = psAudioLib->m_u16RecMaxPayload12;
    psAudioLib->m_u16RecSmplCnt1     <<= 1;
    psAudioLib->m_u16RecSmplCnt2     <<= 1;
    psAudioLib->m_u16RecSmplCnt1_      = psAudioLib->m_u16RecSmplCnt1;
    psAudioLib->m_u16RecSmplCnt2_      = psAudioLib->m_u16RecSmplCnt2;
}

#define REC_PKT_SEQ2_00  0, 0, 0, 0, 0, 0, 0, 0

#ifdef __ICCARM__
#pragma data_alignment=4
static uint8_t s_au8RecPacketSequence2_0[] = {
#else   // __CC_ARM
static uint8_t s_au8RecPacketSequence2_0[] __attribute__((aligned(4))) = {
#endif
    REC_PKT_SEQ2_00
};

#define REC_PKT_SEQ2_10  0, 1, 0, 1, 0, 1, 0, 1
#define REC_PKT_SEQ2_11  1, 0, 1, 0, 1, 0, 1, 0
#define REC_PKT_SEQ2_12  1, 0, 1, 0, 1, 0, 1, 1

#ifdef __ICCARM__
#pragma data_alignment=4
static uint8_t s_au8RecPacketSequence2_1[] = {
#else   // __CC_ARM
static uint8_t s_au8RecPacketSequence2_1[] __attribute__((aligned(4))) = {
#endif
    REC_PKT_SEQ2_10,
    REC_PKT_SEQ2_10,
    REC_PKT_SEQ2_10,
    REC_PKT_SEQ2_10,
    REC_PKT_SEQ2_10,
    REC_PKT_SEQ2_11,
    REC_PKT_SEQ2_11,
    REC_PKT_SEQ2_11,
    REC_PKT_SEQ2_11,
    REC_PKT_SEQ2_12
};

#define REC_PKT_SEQ2_20  0, 1, 1, 1, 0, 1, 1, 1
#define REC_PKT_SEQ2_21  1, 0, 1, 1, 1, 0, 1, 1
#define REC_PKT_SEQ2_22  1, 1, 0, 1, 1, 1, 0, 1
#define REC_PKT_SEQ2_23  1, 1, 1, 0, 1, 1, 1, 0
#define REC_PKT_SEQ2_24  1, 1, 1, 0, 1, 1, 1, 1

#ifdef __ICCARM__
#pragma data_alignment=4
static uint8_t s_au8RecPacketSequence2_2[] = {
#else   // __CC_ARM
static uint8_t s_au8RecPacketSequence2_2[] __attribute__((aligned(4))) = {
#endif
    REC_PKT_SEQ2_20,
    REC_PKT_SEQ2_20,
    REC_PKT_SEQ2_20,
    REC_PKT_SEQ2_20,
    REC_PKT_SEQ2_20,
    REC_PKT_SEQ2_21,
    REC_PKT_SEQ2_21,
    REC_PKT_SEQ2_21,
    REC_PKT_SEQ2_21,
    REC_PKT_SEQ2_21,
    REC_PKT_SEQ2_22,
    REC_PKT_SEQ2_22,
    REC_PKT_SEQ2_22,
    REC_PKT_SEQ2_22,
    REC_PKT_SEQ2_22,
    REC_PKT_SEQ2_23,
    REC_PKT_SEQ2_23,
    REC_PKT_SEQ2_23,
    REC_PKT_SEQ2_23,
    REC_PKT_SEQ2_24
};

#define REC_PKT_SEQ2_30  0, 0, 1, 0, 0, 1, 0, 1
#define REC_PKT_SEQ2_31  0, 0, 1, 0, 1, 0, 0, 1
#define REC_PKT_SEQ2_32  0, 1, 0, 0, 1, 0, 0, 1
#define REC_PKT_SEQ2_33  0, 1, 0, 0, 1, 0, 1, 0
#define REC_PKT_SEQ2_34  0, 1, 0, 1, 0, 0, 1, 0
#define REC_PKT_SEQ2_35  1, 0, 0, 1, 0, 0, 1, 0
#define REC_PKT_SEQ2_36  1, 0, 0, 1, 0, 1, 0, 0
#define REC_PKT_SEQ2_37  1, 0, 1, 0, 0, 1, 0, 0
#define REC_PKT_SEQ2_38  1, 0, 1, 0, 0, 1, 0, 1

#ifdef __ICCARM__
#pragma data_alignment=4
static uint8_t s_au8RecPacketSequence2_3[] = {
#else   // __CC_ARM
static uint8_t s_au8RecPacketSequence2_3[] __attribute__((aligned(4))) = {
#endif
    REC_PKT_SEQ2_30,
    REC_PKT_SEQ2_30,
    REC_PKT_SEQ2_30,
    REC_PKT_SEQ2_30,
    REC_PKT_SEQ2_30,
    REC_PKT_SEQ2_31,
    REC_PKT_SEQ2_31,
    REC_PKT_SEQ2_31,
    REC_PKT_SEQ2_31,
    REC_PKT_SEQ2_31,
    REC_PKT_SEQ2_32,
    REC_PKT_SEQ2_32,
    REC_PKT_SEQ2_32,
    REC_PKT_SEQ2_32,
    REC_PKT_SEQ2_32,
    REC_PKT_SEQ2_33,
    REC_PKT_SEQ2_33,
    REC_PKT_SEQ2_33,
    REC_PKT_SEQ2_33,
    REC_PKT_SEQ2_33,
    REC_PKT_SEQ2_34,
    REC_PKT_SEQ2_34,
    REC_PKT_SEQ2_34,
    REC_PKT_SEQ2_34,
    REC_PKT_SEQ2_34,
    REC_PKT_SEQ2_35,
    REC_PKT_SEQ2_35,
    REC_PKT_SEQ2_35,
    REC_PKT_SEQ2_35,
    REC_PKT_SEQ2_35,
    REC_PKT_SEQ2_36,
    REC_PKT_SEQ2_36,
    REC_PKT_SEQ2_36,
    REC_PKT_SEQ2_36,
    REC_PKT_SEQ2_36,
    REC_PKT_SEQ2_37,
    REC_PKT_SEQ2_37,
    REC_PKT_SEQ2_37,
    REC_PKT_SEQ2_37,
    REC_PKT_SEQ2_38
};

static void _UAC_MicConfigMaxPayload20(S_AUDIO_LIB* psAudioLib)
{
    /* executed in USB IRQ */
    
    _UAC_MicConfigMaxPayload10( psAudioLib );
    
    psAudioLib->m_u16RecMaxPayload21 = psAudioLib->m_u16RecMaxPayload11 >> 3;
    psAudioLib->m_u16RecMaxPayload22 = psAudioLib->m_u16RecMaxPayload12 >> 3;
    psAudioLib->m_u16RecMaxPayload1_ = psAudioLib->m_u16RecMaxPayload21;
    psAudioLib->m_u16RecMaxPayload2_ = psAudioLib->m_u16RecMaxPayload22;
    psAudioLib->m_u16RecSmplCnt1_    = psAudioLib->m_u16RecSmplCnt1 >> 3;
    psAudioLib->m_u16RecSmplCnt2_    = psAudioLib->m_u16RecSmplCnt2 >> 3;
    
    switch ( psAudioLib->m_u32RecSampleRate )
    {
        case  11025:
            psAudioLib->m_u16RecMaxPayload1_ =  1 * psAudioLib->m_u8RecChannels * psAudioLib->m_u8RecSmplSize;
            psAudioLib->m_u16RecMaxPayload2_ =  2 * psAudioLib->m_u8RecChannels * psAudioLib->m_u8RecSmplSize;
            psAudioLib->m_u16RecSmplCnt1_    =  1 * 2;
            psAudioLib->m_u16RecSmplCnt2_    =  2 * 2;
            psAudioLib->m_u16RecPacketSequenceCnt = 320-2;
            psAudioLib->m_pu8RecPacketSequence    = s_au8RecPacketSequence2_3;
            break;
        case  22050:
            psAudioLib->m_u16RecMaxPayload1_ =  2 * psAudioLib->m_u8RecChannels * psAudioLib->m_u8RecSmplSize;
            psAudioLib->m_u16RecMaxPayload2_ =  3 * psAudioLib->m_u8RecChannels * psAudioLib->m_u8RecSmplSize;
            psAudioLib->m_u16RecSmplCnt1_    =  2 * 2;
            psAudioLib->m_u16RecSmplCnt2_    =  3 * 2;
            psAudioLib->m_u16RecPacketSequenceCnt = 160-2;
            psAudioLib->m_pu8RecPacketSequence    = s_au8RecPacketSequence2_2;
            break;
        case  44100:
            psAudioLib->m_u16RecMaxPayload1_ =  5 * psAudioLib->m_u8RecChannels * psAudioLib->m_u8RecSmplSize;
            psAudioLib->m_u16RecMaxPayload2_ =  6 * psAudioLib->m_u8RecChannels * psAudioLib->m_u8RecSmplSize;
            psAudioLib->m_u16RecSmplCnt1_    =  5 * 2;
            psAudioLib->m_u16RecSmplCnt2_    =  6 * 2;
            psAudioLib->m_u16RecPacketSequenceCnt = 80-2;
            psAudioLib->m_pu8RecPacketSequence    = s_au8RecPacketSequence2_1;
            break;
        case  88200:
            psAudioLib->m_u16RecMaxPayload1_ = 10 * psAudioLib->m_u8RecChannels * psAudioLib->m_u8RecSmplSize;
            psAudioLib->m_u16RecMaxPayload2_ = 12 * psAudioLib->m_u8RecChannels * psAudioLib->m_u8RecSmplSize;
            psAudioLib->m_u16RecSmplCnt1_    = 10 * 2;
            psAudioLib->m_u16RecSmplCnt2_    = 12 * 2;
            psAudioLib->m_u16RecPacketSequenceCnt = 80-2;
            psAudioLib->m_pu8RecPacketSequence    = s_au8RecPacketSequence2_1;
            break;
        case 176400:
            psAudioLib->m_u16RecMaxPayload1_ = 20 * psAudioLib->m_u8RecChannels * psAudioLib->m_u8RecSmplSize;
            psAudioLib->m_u16RecMaxPayload2_ = 24 * psAudioLib->m_u8RecChannels * psAudioLib->m_u8RecSmplSize;
            psAudioLib->m_u16RecSmplCnt1_    = 20 * 2;
            psAudioLib->m_u16RecSmplCnt2_    = 24 * 2;
            psAudioLib->m_u16RecPacketSequenceCnt = 80-2;
            psAudioLib->m_pu8RecPacketSequence    = s_au8RecPacketSequence2_1;
            break;
        default:
            psAudioLib->m_u16RecPacketSequenceCnt = 8-2;
            psAudioLib->m_pu8RecPacketSequence    = s_au8RecPacketSequence2_0;
            break;
    }
}

void AudioLib_InitRecCallback(S_AUDIO_LIB* psAudioLib)
{
    psAudioLib->m_pfnRecSpeed              = _UAC_MicSpeed;
    psAudioLib->m_pfnRecStop               = _UAC_MicStop;
    psAudioLib->m_pfnRecConfigMaxPayload10 = _UAC_MicConfigMaxPayload10;
    psAudioLib->m_pfnRecConfigMaxPayload20 = _UAC_MicConfigMaxPayload20;
}
#endif  // CONFIG_AUDIO_REC
