#include "NUC505Series.h"

#include "Hardware.h"
#include "AudioLib.h"

#if CONFIG_CODEC_EXTERNAL
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
            ;/* TODO */
            ;/* TODO */
        }
        else
        {
            ;/* TODO */
            ;/* TODO */
        }
    }
    
    if ( s_i16PlayVolumeL != psAudioLib->m_i16PlayVolumeL )
    {
        s_i16PlayVolumeL = psAudioLib->m_i16PlayVolumeL;
        if ( s_u8PlayMute )
            ;/* TODO */
        else
            ;/* TODO */
    }
    
    if ( s_i16PlayVolumeR != psAudioLib->m_i16PlayVolumeR )
    {
        s_i16PlayVolumeR = psAudioLib->m_i16PlayVolumeR;
        if ( s_u8PlayMute )
            ;/* TODO */
        else
            ;/* TODO */
    }
    #endif  // CONFIG_AUDIO_PLAY
}

#if CONFIG_NAU8822L
void HeadphoneOutMicIn_Init(void)
{
    _I2C_WriteData( 0x0000, 0x0000 );   /* Reset all registers */
    CLK_SysTickDelay(10000);
    
    //input source is MIC
    _I2C_WriteData(  1, 0x01F );//R1 MICBIASEN ABIASEN IOBUFEN REFIMP
    _I2C_WriteData(  4, 0x070 );//R4 select audio format(I2S format) and word length (32bits)
    _I2C_WriteData(  5, 0x000 );//R5 companding ctrl
    _I2C_WriteData(  6, 0x000 );//R6 clock ctrl at slave mode
    _I2C_WriteData( 35, 0x000 );//R35 disable noise gate
    _I2C_WriteData( 45, 0x13F );//R45 Left input PGA gain
    _I2C_WriteData( 46, 0x13F );//R46 Right input PGA gain
    _I2C_WriteData( 44, 0x033 );//R44 MIC
    _I2C_WriteData( 47, 0x000 );//R47 Left ADC boost
    _I2C_WriteData( 48, 0x000 );//R48 Right ADC boost
    _I2C_WriteData(  2, 0x1BF );//R2 Power Management 2
    _I2C_WriteData(  3, 0x00F );//R3 Power Management 3
    _I2C_WriteData( 10, 0x000 );//R10 DAC control (softmute disable, oversample select 64x (lowest power) )
    _I2C_WriteData( 45, 0x13F );//R45 Left input PGA gain
    _I2C_WriteData( 46, 0x13F );//R46 Right input PGA gain
    _I2C_WriteData( 50, 0x001 );//R50 Left mixer
    _I2C_WriteData( 51, 0x001 );//R51 Right mixer
    _I2C_WriteData( 49, 0x002 );//R49 Output control
}
#endif  // CONFIG_NAU8822L

#if CONFIG_NAU88L25
void HeadphoneOutMicIn_Init(void)
{
    _I2C_WriteData( 0x0000, 0x0000 );   /* Reset all registers */
    CLK_SysTickDelay(10000);
    
    _I2C_WriteData( 0x0066, 0x0060 );
    
    _I2C_WriteData( 0x0003, 0x0010 );
    // FLL Setting
    _I2C_WriteData( 0x0004, 0x0001 );
    _I2C_WriteData( 0x0005, 0x3126 );
    _I2C_WriteData( 0x0006, 0x0008 );
    _I2C_WriteData( 0x0007, 0x0010 );
    _I2C_WriteData( 0x0008, 0xC000 );
    _I2C_WriteData( 0x000C, 0x0048 );
    // Digital Audio Bus Format
    _I2C_WriteData( 0x001C, 0x0002 );
    #define _I2S_MODE_DIR 1
    if (_I2S_MODE_DIR ==  1) { //_I2S_MODE_MASTER
        _I2C_WriteData( 0x001D, 0x3012 ); //301A:Master 3012:Slave
        _I2C_WriteData( 0x001E, 0x2000 );
    }
    else
    {
        _I2C_WriteData( 0x001D, 0x301A ); //301A:Master 3012:Slave
    }
    _I2C_WriteData( 0x002B, 0x0012 );
    _I2C_WriteData( 0x002C, 0x0082 );
    _I2C_WriteData( 0x0030, 0x00CF );
    _I2C_WriteData( 0x0031, 0x1000 );
    _I2C_WriteData( 0x0033, 0x00CF );
    _I2C_WriteData( 0x0034, 0x02CF );
    _I2C_WriteData( 0x0050, 0x2007 );
    _I2C_WriteData( 0x0066, 0x0060 );
    _I2C_WriteData( 0x0068, 0xC300 );
    _I2C_WriteData( 0x006A, 0x0083 );
    _I2C_WriteData( 0x0072, 0x0260 );
    _I2C_WriteData( 0x0073, 0x332C );
    _I2C_WriteData( 0x0074, 0x4502 );
    _I2C_WriteData( 0x0076, 0x3140 );
    _I2C_WriteData( 0x007F, 0x553F );
    _I2C_WriteData( 0x0080, 0x0420 );
    _I2C_WriteData( 0x0001, 0x07D4 );
}
#endif  // CONFIG_NAU88L25

void Codec_Init(void)
{
    int16_t i;
    
    _I2C_SetTxCallback();
    
    /* External CODEC Init */
    HeadphoneOutMicIn_Init();
    
    printf("I2C write External CODEC OK\n");
    
    _I2C_SetRxCallback();
    
    for ( i = 0; i <= 51; i ++ )
    {
        printf("%02d ", i);
        _I2C_ReadData( i );
    }
    
    printf("I2C read External CODEC OK\n");
    
    printf("External CODEC init [OK]\n");
}
#endif  // CONFIG_CODEC_EXTERNAL
