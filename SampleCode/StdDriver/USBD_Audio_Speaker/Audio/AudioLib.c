#include <string.h>

#include "NUC505Series.h"

#include "AudioLib.h"

S_AUDIO_LIB g_sAudioLib;

#if CONFIG_AUDIO_PLAY
#ifdef __ICCARM__
#pragma data_alignment=4
static uint8_t s_au8I2sPlayPcmBuf[RING_BUF_SZ];
#pragma data_alignment=4
static uint8_t s_au8PlayPcmWorkBuf[RING_BUF_SZ];
#pragma data_alignment=4
static uint8_t s_au8PlayPcmTmpBuf[1280];
#else   // __CC_ARM
static uint8_t s_au8I2sPlayPcmBuf[RING_BUF_SZ] __attribute__((aligned(4)));
static uint8_t s_au8PlayPcmWorkBuf[RING_BUF_SZ] __attribute__((aligned(4)));
static uint8_t s_au8PlayPcmTmpBuf[1280] __attribute__((aligned(4)));
#endif
#endif  // CONFIG_AUDIO_PLAY

static void AudioLib_Init2(S_AUDIO_LIB* psAudioLib, uint32_t u32I2sSampleRate, uint32_t u32I2sBitRate)
{
    /* executed in main loop */

    uint16_t u16I2sSmplCnt1;

    psAudioLib->m_u32I2sSampleRate = u32I2sSampleRate;
    psAudioLib->m_u32I2sBitRate    = u32I2sBitRate;

    u16I2sSmplCnt1 = u32I2sSampleRate / 500;    /* stereo samples per millisecond */
    //u16I2sSmplCnt1 = u32I2sSampleRate / 1000 * 2;

#if CONFIG_AUDIO_PLAY
    psAudioLib->m_u16I2sSmplCntPlayTooFastStop = u16I2sSmplCnt1 * 3;
    psAudioLib->m_u16I2sSmplCntPlayTooFast     = u16I2sSmplCnt1 * 4;
    psAudioLib->m_u16I2sSmplCntPlayStart       = u16I2sSmplCnt1 * 6;
    psAudioLib->m_u16I2sSmplCntPlayTooSlow     = u16I2sSmplCnt1 * 8;
    psAudioLib->m_u16I2sSmplCntPlayTooSlowStop = u16I2sSmplCnt1 * 9;
#endif  // CONFIG_AUDIO_PLAY

    /* internal CODEC word width always 32-bit */
    psAudioLib->m_u8I2sBclkFactor = 32;
    if ( u32I2sBitRate == I2S_DATABIT_16 )
    {
        psAudioLib->m_u8I2sShiftFlag = 1;    /* divide 2 */
#if CONFIG_CODEC_EXTERNAL
        /* note may 32 when word width is 32-bit */
        psAudioLib->m_u8I2sBclkFactor = 16;
#endif  // CONFIG_CODEC_EXTERNAL
    }
    else    /* I2S_DATABIT_24 and I2S_DATABIT_32 */
        /* note for 24-bit we use I2S_DATABIT_32 instead */
        psAudioLib->m_u8I2sShiftFlag = 2;    /* divide 4 */

    /* I2S module MCLK factor master mode */
    switch ( u32I2sSampleRate )
    {
    case  64000:
    case 192000:
    case 176400:
        /* note not all codecs support 128 */
        psAudioLib->m_u16I2sMclkFactor = 128;
        break;
    default:
        psAudioLib->m_u16I2sMclkFactor = 256;
        break;
    }

    /* assign a default APLL as a reference */
    if ( u32I2sSampleRate % 11025 )
    {
        /* 8000 Hz and 12000 Hz sampling rate series */
        psAudioLib->m_u32I2sApllUp  = 0x0844D542;
        psAudioLib->m_u32I2sApll    = CLK_APLL_49152031;    //0x0424D542
        psAudioLib->m_u32I2sApllDn  = CLK_APLL_49142857;    //0x    D542
        psAudioLib->m_u32I2sRefApll = psAudioLib->m_u32I2sApll;
    }
    else
    {
        /* 11025 Hz sampling rate series */
        psAudioLib->m_u32I2sApllUp  = 0x0E04D382;
        psAudioLib->m_u32I2sApll    = CLK_APLL_45158425;    //0x0704D382
        psAudioLib->m_u32I2sApllDn  = CLK_APLL_45142857;    //0x    D382
        psAudioLib->m_u32I2sRefApll = psAudioLib->m_u32I2sApll;
    }

    /* I2S module clock from APLL master mode */
    CLK_SET_APLL(psAudioLib->m_u32I2sRefApll);

    /* I2S module clock divider master mode */
    if ( u32I2sSampleRate == 8000 )
        /* APLL / (1 + 1) as I2S engine clock for 8000 Hz lower speed sampling rate */
        CLK_SetModuleClock(I2S_MODULE, CLK_I2S_SRC_APLL, 1);
    else
        /* APLL / (1 + 0) as I2S engine clock for other sampling rate */
        CLK_SetModuleClock(I2S_MODULE, CLK_I2S_SRC_APLL, 0);
}

void AudioLib_Init3(S_AUDIO_LIB* psAudioLib, uint32_t u32I2sSampleRate, uint32_t u32I2sBitRate)
{
    /* executed in main loop */

    uint32_t u32I2sBclkDiv, u32I2sMclkDiv;

#if CONFIG_AUDIO_PLAY
    psAudioLib->m_pfnPlayStop( psAudioLib );
#endif  // CONFIG_AUDIO_PLAY

    AudioLib_Init2( psAudioLib, u32I2sSampleRate, u32I2sBitRate );

    if ( u32I2sSampleRate % 11025 )
    {
        /* 8000 Hz and 12000 Hz sampling rate series stereo */
        u32I2sBclkDiv = 49152000 / (u32I2sSampleRate * 2 * psAudioLib->m_u8I2sBclkFactor)  / 2 - 1;
        u32I2sMclkDiv = 49152000 / (u32I2sSampleRate *     psAudioLib->m_u16I2sMclkFactor) / 2;
    }
    else
    {
        /* 11025 Hz sampling rate series stereo */
        u32I2sBclkDiv = 45158400 / (u32I2sSampleRate * 2 * psAudioLib->m_u8I2sBclkFactor)  / 2 - 1;
        u32I2sMclkDiv = 45158400 / (u32I2sSampleRate *     psAudioLib->m_u16I2sMclkFactor) / 2;
    }

    I2S->CTL    &= ~I2S_CTL_WDWIDTH_Msk;
    I2S->CTL    |=  u32I2sBitRate;
    I2S->CLKDIV &= ~(I2S_CLKDIV_BCLKDIV_Msk | I2S_CLKDIV_MCLKDIV_Msk);
    I2S->CLKDIV  =  (u32I2sBclkDiv << 8)    | u32I2sMclkDiv;
}

static void AudioLib_Init(S_AUDIO_LIB* psAudioLib)
{
    /* executed in main loop */

    memset( psAudioLib, 0, sizeof(S_AUDIO_LIB) );

#if CONFIG_AUDIO_PLAY
    psAudioLib->m_u32PlaySampleRate = 48000;
    psAudioLib->m_u8PlayBitRate     = 16;
    psAudioLib->m_u8PlayChannels    = 2;

    psAudioLib->m_u8PlayMute        = PLAY_MUTE;
    psAudioLib->m_i16PlayVolumeL    = PLAY_VOL_L;
    psAudioLib->m_i16PlayVolumeR    = PLAY_VOL_R;
    psAudioLib->m_i16PlayMaxVolume  = PLAY_MAX_VOL;
    psAudioLib->m_i16PlayMinVolume  = PLAY_MIN_VOL;
    psAudioLib->m_i16PlayResVolume  = PLAY_RES_VOL;

    psAudioLib->m_pu8I2sPlayPcmBuf  = s_au8I2sPlayPcmBuf;
    psAudioLib->m_pu8PlayPcmWorkBuf = s_au8PlayPcmWorkBuf;
    psAudioLib->m_pu8PlayPcmTmpBuf  = s_au8PlayPcmTmpBuf;
#endif  // CONFIG_AUDIO_PLAY

    /* Init I2S, IP clock and multi-function I/O */
    /* Enable I2S Module clock */
    CLK_EnableModuleClock(I2S_MODULE);

    AudioLib_Init2( psAudioLib, _I2S_SAMPLE_RATE, _I2S_BIT_RATE );

    /* Reset IP */
    SYS_ResetModule(I2S_RST);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Configure multi-function pins for I2S */
    SYS->GPC_MFPH = (SYS->GPC_MFPH & (~SYS_GPC_MFPH_PC8MFP_Msk) )  | SYS_GPC_MFPH_PC8MFP_I2S_MCLK;
    SYS->GPC_MFPH = (SYS->GPC_MFPH & (~SYS_GPC_MFPH_PC9MFP_Msk) )  | SYS_GPC_MFPH_PC9MFP_I2S_DIN;
    SYS->GPC_MFPH = (SYS->GPC_MFPH & (~SYS_GPC_MFPH_PC10MFP_Msk) ) | SYS_GPC_MFPH_PC10MFP_I2S_DOUT;
    SYS->GPC_MFPH = (SYS->GPC_MFPH & (~SYS_GPC_MFPH_PC11MFP_Msk) ) | SYS_GPC_MFPH_PC11MFP_I2S_LRCLK;
    SYS->GPC_MFPH = (SYS->GPC_MFPH & (~SYS_GPC_MFPH_PC12MFP_Msk) ) | SYS_GPC_MFPH_PC12MFP_I2S_BCLK;

#if CONFIG_CODEC_INTERNAL
    I2S_Open(I2S, I2S_MODE_MASTER, _I2S_SAMPLE_RATE, _I2S_BIT_RATE, I2S_STEREO, I2S_FORMAT_I2S, I2S_ENABLE_INTERNAL_CODEC);
#endif  // CONFIG_CODEC_INTERNAL

#if CONFIG_CODEC_EXTERNAL
    I2S_Open(I2S, I2S_MODE_MASTER, _I2S_SAMPLE_RATE, _I2S_BIT_RATE, I2S_STEREO, I2S_FORMAT_I2S, I2S_DISABLE_INTERNAL_CODEC);
#endif  // CONFIG_CODEC_EXTERNAL

    I2S_EnableMCLK(I2S, (_I2S_SAMPLE_RATE*psAudioLib->m_u16I2sMclkFactor));

    Codec_Init();

#if CONFIG_AUDIO_PLAY
    I2S_SET_TXDMA_STADDR(I2S, (uint32_t)&s_au8I2sPlayPcmBuf[0]);               // Tx Start Address
    I2S_SET_TXDMA_EADDR( I2S, (uint32_t)&s_au8I2sPlayPcmBuf[RING_BUF_SZ-4]);   // Tx End Address
#endif  // CONFIG_AUDIO_PLAY

#if CONFIG_AUDIO_PLAY
    /* start playing */
    I2S_ENABLE_TXDMA(I2S);
    I2S_ENABLE_TX(I2S);
#endif  // CONFIG_AUDIO_PLAY
}

void AudioLib_Start(void)
{
    S_AUDIO_LIB* psAudioLib = &g_sAudioLib;

    AudioLib_Init( psAudioLib );

#if CONFIG_AUDIO_PLAY
    AudioLib_InitPlayCallback( psAudioLib );

    psAudioLib->m_pfnPlayConfigMaxPayload10( psAudioLib );

    psAudioLib->m_pfnPlayConfigMaxPayload20( psAudioLib );
#endif  // CONFIG_AUDIO_PLAY
}

void AudioLib_Process(void)
{
    /* executed in main loop */

    S_AUDIO_LIB* psAudioLib = &g_sAudioLib;

#if CONFIG_AUDIO_PLAY
    psAudioLib->m_pfnPlaySpeed( psAudioLib );

    psAudioLib->m_pfnPlayMode2( psAudioLib );
#endif  // CONFIG_AUDIO_PLAY

    Codec_Vol( psAudioLib );
}
