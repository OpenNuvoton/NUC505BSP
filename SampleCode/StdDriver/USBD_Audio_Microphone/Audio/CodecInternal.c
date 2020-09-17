/**************************************************************************//**
 * @file        CodecInternal.c
 * @version     V1.00
 * $Revision:   1$
 * $Date:       17/05/09 5:00p$
 * @brief       NUC505 Series USB Audio Class Sample Code source file
 *
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "NUC505Series.h"
#include "AudioLib.h"

#if CONFIG_CODEC_INTERNAL
#if CONFIG_AUDIO_REC
#ifdef __ICCARM__
#pragma data_alignment=4
static volatile int16_t s_i16RecVolumeL;
#pragma data_alignment=4
static volatile uint8_t s_u8RecMute;
#else   // __CC_ARM
static volatile int16_t s_i16RecVolumeL __attribute__((aligned(4)));
static volatile uint8_t s_u8RecMute __attribute__((aligned(4)));
#endif

// from 0 dB, 1.6 dB, 3.2 dB, ... , 22.4 dB step 1.6 dB
#ifdef __ICCARM__
#pragma data_alignment=4
static int8_t s_ai8RecVolTbl[] =
{
#else   // __CC_ARM
static int8_t s_ai8RecVolTbl[] __attribute__((aligned(4))) =
{
#endif
    0, 1, 3, 4, 6, 8, 9, 11, 12, 14, 16, 17, 19, 20, 22, 24
};

static int8_t Codec_RecVol(int8_t i8RecVol)
{
    /* executed in main loop */

    int8_t i;

    for ( i = 0; i <= 14; i ++ )
    {
        if ( i8RecVol <= s_ai8RecVolTbl[i] )
        {
            return i;
        }
    }

    return 14;
}
#endif  // CONFIG_AUDIO_REC

void Codec_Vol(S_AUDIO_LIB* psAudioLib)
{
    /* executed in main loop */

#if CONFIG_AUDIO_REC
    /* note need to ramp up ramp down to avoid pop-noise */

    if ( s_u8RecMute != psAudioLib->m_u8RecMute )
    {
        s_u8RecMute = psAudioLib->m_u8RecMute;
        if ( s_u8RecMute )
        {
            I2S_SET_INTERNAL_CODEC(I2S, 0x10, 0x0F);
            I2S_SET_INTERNAL_CODEC(I2S, 0x11, 0x0F);
        }
        else
        {
#if CONFIG_HEADPHONE_OUT_LINE_IN
            I2S_SET_INTERNAL_CODEC(I2S, 0x10, Codec_RecVol( (int8_t)(s_i16RecVolumeL >> 8) ));
#else
            //MIC0 Pre-Gain 20 dB
            I2S_SET_INTERNAL_CODEC(I2S, 0x10, Codec_RecVol( (int8_t)(s_i16RecVolumeL >> 8) ) | 0x10);
#endif
            I2S_SET_INTERNAL_CODEC(I2S, 0x11, Codec_RecVol( (int8_t)(s_i16RecVolumeL >> 8) ));
        }
    }

    if ( s_i16RecVolumeL != psAudioLib->m_i16RecVolumeL )
    {
        s_i16RecVolumeL = psAudioLib->m_i16RecVolumeL;
        if ( s_u8RecMute )
        {
            I2S_SET_INTERNAL_CODEC(I2S, 0x10, 0x0F);
            I2S_SET_INTERNAL_CODEC(I2S, 0x11, 0x0F);
        }
        else
        {
#if CONFIG_HEADPHONE_OUT_LINE_IN
            I2S_SET_INTERNAL_CODEC(I2S, 0x10, Codec_RecVol( (int8_t)(s_i16RecVolumeL >> 8) ));
#else
            //MIC0 Pre-Gain 20 dB
            I2S_SET_INTERNAL_CODEC(I2S, 0x10, Codec_RecVol( (int8_t)(s_i16RecVolumeL >> 8) ) | 0x10);
#endif
            I2S_SET_INTERNAL_CODEC(I2S, 0x11, Codec_RecVol( (int8_t)(s_i16RecVolumeL >> 8) ));
        }
    }
#endif  // CONFIG_AUDIO_REC

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
