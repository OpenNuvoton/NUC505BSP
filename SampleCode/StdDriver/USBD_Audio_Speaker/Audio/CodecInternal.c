#include "NUC505Series.h"

#include "AudioLib.h"

#if CONFIG_CODEC_INTERNAL

#if CONFIG_AUDIO_PLAY
#ifdef __ICCARM__
#pragma data_alignment=4
static volatile int16_t s_i16PlayVolumeL;
#pragma data_alignment=4
static volatile int16_t s_i16PlayVolumeR;
#pragma data_alignment=4
static volatile uint8_t s_u8PlayMute;
#else   // __CC_ARM
static volatile int16_t s_i16PlayVolumeL __attribute__((aligned(4)));
static volatile int16_t s_i16PlayVolumeR __attribute__((aligned(4)));
static volatile uint8_t s_u8PlayMute __attribute__((aligned(4)));
#endif

// from 0 dB, -2 dB, -4 dB, ... , -60 dB, step -2 dB
#ifdef __ICCARM__
#pragma data_alignment=4
static int8_t s_ai8PlayVolTbl[] = {
#else   // __CC_ARM
static int8_t s_ai8PlayVolTbl[] __attribute__((aligned(4))) = {
#endif
    0,    -2,  -4,  -6,  -8, -10, -12, -14, -16, -18, -20, -22, -24, -26, -28, -30,
    -32, -34, -36, -38, -40, -42, -44, -46, -48, -50, -52, -54, -56, -58, -60
};

static int8_t Codec_PlayVol(int8_t i8PlayVol)
{
    /* executed in main loop */
    
    int8_t i;
    
    for ( i = 0; i <= 30; i ++ )
    {
        if ( i8PlayVol >= s_ai8PlayVolTbl[i] )
        {
            return i;
        }
    }
    
    return 30;
}
#endif  // CONFIG_AUDIO_PLAY

void Codec_Vol(S_AUDIO_LIB* psAudioLib)
{
    /* executed in main loop */
    
    #if CONFIG_AUDIO_PLAY
    /* note need to ramp up ramp down to avoid pop-noise */
    
    if ( s_u8PlayMute != psAudioLib->m_u8PlayMute )
    {
        s_u8PlayMute = psAudioLib->m_u8PlayMute;
        if ( s_u8PlayMute )
        {
            I2S_SET_INTERNAL_CODEC(I2S, 0x08, 0x1F);
            I2S_SET_INTERNAL_CODEC(I2S, 0x09, 0x1F);
        }
        else
        {
            I2S_SET_INTERNAL_CODEC(I2S, 0x08, Codec_PlayVol( (int8_t)(s_i16PlayVolumeL >> 8) ));
            I2S_SET_INTERNAL_CODEC(I2S, 0x09, Codec_PlayVol( (int8_t)(s_i16PlayVolumeR >> 8) ));
        }
    }
    
    if ( s_i16PlayVolumeL != psAudioLib->m_i16PlayVolumeL )
    {
        s_i16PlayVolumeL = psAudioLib->m_i16PlayVolumeL;
        if ( s_u8PlayMute )
        {
            I2S_SET_INTERNAL_CODEC(I2S, 0x08, 0x1F);
        }
        else
        {
            I2S_SET_INTERNAL_CODEC(I2S, 0x08, Codec_PlayVol( (int8_t)(s_i16PlayVolumeL >> 8) ));
        }
    }
    
    if ( s_i16PlayVolumeR != psAudioLib->m_i16PlayVolumeR )
    {
        s_i16PlayVolumeR = psAudioLib->m_i16PlayVolumeR;
        if ( s_u8PlayMute )
        {
            I2S_SET_INTERNAL_CODEC(I2S, 0x09, 0x1F);
        }
        else
        {
            I2S_SET_INTERNAL_CODEC(I2S, 0x09, Codec_PlayVol( (int8_t)(s_i16PlayVolumeR >> 8) ));
        }
    }
    #endif  // CONFIG_AUDIO_PLAY
}

#if CONFIG_HEADPHONE_OUT_LINE_IN
void HeadphoneOutLineIn_Init(void)
{
    int16_t i;
    
    /* IIC Configure Step without PLL: */
    /* Add MCLK(256*Fs) in. */
    
    I2S_SET_INTERNAL_CODEC(I2S, 0x08, 0x1F);    //Mute headphone of Left channel
    I2S_SET_INTERNAL_CODEC(I2S, 0x09, 0x1F);    //Mute headphone of Right channel
    I2S_SET_INTERNAL_CODEC(I2S, 0x10, 0x0F);    //Mute the ADC Left channel volume
    I2S_SET_INTERNAL_CODEC(I2S, 0x11, 0x0F);    //Mute the ADC Right channel volume
    I2S_SET_INTERNAL_CODEC(I2S, 0x12, 0x0F);    //Mute the ADC Side tone volume
    
    I2S_SET_INTERNAL_CODEC(I2S, 0x02, 0xC0);    //Set CODEC slave
    
    I2S_SET_INTERNAL_CODEC(I2S, 0x01, 0x80);    //Digital Part Enable
    I2S_SET_INTERNAL_CODEC(I2S, 0x0F, 0xF0);    //Enable Analog Part
    I2S_SET_INTERNAL_CODEC(I2S, 0x0E, 0x00);    //ADC input select Line in
    
    I2S_SET_INTERNAL_CODEC(I2S, 0x0B, 0xF3);    //Analog Part Enable
    I2S_SET_INTERNAL_CODEC(I2S, 0x0D, 0x31);    //Biasing enable
    I2S_SET_INTERNAL_CODEC(I2S, 0x0B, 0xE3);
    for ( i = 0; i < 15; i ++ )  //Delay 1.5s~2.5s
        CLK_SysTickDelay(100000);
    I2S_SET_INTERNAL_CODEC(I2S, 0x0A, 0x09);
    I2S_SET_INTERNAL_CODEC(I2S, 0x0B, 0xF0);
    I2S_SET_INTERNAL_CODEC(I2S, 0x00, 0xD0);    //ADC digital enabled
    //CLK_SysTickDelay(100000);    //Delay 100mS
    //I2S_SET_INTERNAL_CODEC(I2S, 0x08, 15);      //Un-mute Headphone and set volume
    //I2S_SET_INTERNAL_CODEC(I2S, 0x09, 15);      //Un-mute Headphone and set volume
    //I2S_SET_INTERNAL_CODEC(I2S, 0x10, 14);      //Un-Mute the ADC Left channel volume
    //I2S_SET_INTERNAL_CODEC(I2S, 0x11, 14);      //Un-Mute the ADC Right channel volume
    //I2S_SET_INTERNAL_CODEC(I2S, 0x12, 0x00);    //Un-Mute the ADC Side tone volume
    
    /* If Fs is changed, please Mute Headphone First and soft reset digital part after MCLK is stable. */
}
#endif  // CONFIG_LINE_IN_HEADPHONE_OUT

#if CONFIG_HEADPHONE_OUT_MIC0_IN
void HeadphoneOutMicIn_Init(void)
{
    int16_t i;
    
    /* IIC Configure Step without PLL: */
    /* Add MCLK(256*Fs) in. */
    
    I2S_SET_INTERNAL_CODEC(I2S, 0x08, 0x1F);    //Mute headphone of Left channel
    I2S_SET_INTERNAL_CODEC(I2S, 0x09, 0x1F);    //Mute headphone of Right channel
    I2S_SET_INTERNAL_CODEC(I2S, 0x10, 0x0F);    //Mute the ADC Left channel volume
    I2S_SET_INTERNAL_CODEC(I2S, 0x11, 0x0F);    //Mute the ADC Right channel volume
    I2S_SET_INTERNAL_CODEC(I2S, 0x12, 0x0F);    //Mute the ADC Side tone volume
    
    I2S_SET_INTERNAL_CODEC(I2S, 0x02, 0xC0);    //Set CODEC slave
    
    I2S_SET_INTERNAL_CODEC(I2S, 0x01, 0x80);    //Digital Part Enable
    I2S_SET_INTERNAL_CODEC(I2S, 0x0F, 0xC0);    //Enable Analog Part
    I2S_SET_INTERNAL_CODEC(I2S, 0x0E, 0x02);    //ADC input select MIC0
    
    I2S_SET_INTERNAL_CODEC(I2S, 0x0B, 0xF3);    //Analog Part Enable
    I2S_SET_INTERNAL_CODEC(I2S, 0x0D, 0x31);    //Biasing enable
    I2S_SET_INTERNAL_CODEC(I2S, 0x0B, 0xE3);
    for ( i = 0; i < 15; i ++ )  //Delay 1.5s~2.5s
        CLK_SysTickDelay(100000);
    I2S_SET_INTERNAL_CODEC(I2S, 0x0A, 0x09);
    I2S_SET_INTERNAL_CODEC(I2S, 0x0B, 0xF0);
    I2S_SET_INTERNAL_CODEC(I2S, 0x00, 0xD0);    //ADC digital enabled
    //CLK_SysTickDelay(100000);    //Delay 100mS
    //I2S_SET_INTERNAL_CODEC(I2S, 0x08, 15);      //Un-mute Headphone and set volume
    //I2S_SET_INTERNAL_CODEC(I2S, 0x09, 15);      //Un-mute Headphone and set volume
    //I2S_SET_INTERNAL_CODEC(I2S, 0x10, 14|0x10); //Un-Mute the ADC Left channel volume and set Pre-Gain 20 dB
    //I2S_SET_INTERNAL_CODEC(I2S, 0x11, 14);      //Un-Mute the ADC Right channel volume
    //I2S_SET_INTERNAL_CODEC(I2S, 0x12, 0x00);    //Un-Mute the ADC Side tone volume
    
   /* If Fs is changed, please Mute Headphone First and soft reset digital part after MCLK is stable. */
}
#endif  // CONFIG_HEADPHONE_OUT_MIC0_IN

void Codec_Init(void)
{
    #if CONFIG_HEADPHONE_OUT_LINE_IN
    // Setting Right Line In Channel
    SYS->GPD_MFPL  = (SYS->GPD_MFPL & (~SYS_GPD_MFPL_PD4MFP_Msk) ) | SYS_GPD_MFPL_PD4MFP_RLINEIN; /* note for BSP .003 */
    /* SYS->GPD_MFPL  = (SYS->GPD_MFPL & (~SYS_GPD_MFPL_PD4MFP_Msk) ) | SYS_GPD_MFPL_PD4MFP_RIGHT_LINE_IN; note for BSP .002 */
    SYS_SetSharedPinType(SYS_PORT_D, 4, 0, 0);
    
    HeadphoneOutLineIn_Init();
    #endif  // CONFIG_HEADPHONE_OUT_LINE_IN
    
    #if CONFIG_HEADPHONE_OUT_MIC0_IN
    HeadphoneOutMicIn_Init();
    #endif  // CONFIG_HEADPHONE_OUT_MIC0_IN
    
    printf("Internal CODEC init [OK]\n");
}
#endif  // CONFIG_CODEC_INTERNAL
