#ifndef __AUDIOLIB_H__
#define __AUDIOLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_AUDIO_REC                1
#define CONFIG_AUDIO_PLAY               0

#define CONFIG_CODEC_INTERNAL           1
#define CONFIG_HEADPHONE_OUT_LINE_IN    1
#define CONFIG_HEADPHONE_OUT_MIC0_IN    0

#define CONFIG_CODEC_EXTERNAL           0
#define CONFIG_NAU8822L                 1
#define CONFIG_NAU88L25                 0

/* dB format is 8.8 fixed-point */
#define REC_MUTE        0x00   // 0 is mute disable, 1 is mute enable
#define REC_VOL         0x1000 //16.0 dB, given a default value
#define REC_MAX_VOL     0x1666 //22.4 dB, follow the spec setting
#define REC_MIN_VOL     0x0000 // 0.0 dB, follow the spec setting
#define REC_RES_VOL     0x0199 // 1.6 dB, follow the spec setting

#define PLAY_MUTE       0x00   // 0 is mute disable, 1 is mute enable
#define PLAY_VOL_L      0xEC00 // -20 dB, given a default value
#define PLAY_VOL_R      0xEC00 // -20 dB, given a default value
#define PLAY_MAX_VOL    0x0000 //   0 dB, follow the spec setting
#define PLAY_MIN_VOL    0xC400 // -60 dB, follow the spec setting
#define PLAY_RES_VOL    0x0200 //   2 dB, follow the spec setting

/* note declare about 10ms ring buffer */
#define RING_BUF_SZ     16384

/* note 16-bit sample count in ring buffer */
#define RING_BUF_16CNT  (RING_BUF_SZ / 2)

/* note 32-bit sample count in ring buffer */
#define RING_BUF_32CNT  (RING_BUF_SZ / 4)

typedef struct S_AUDIO_LIB
{
    uint32_t m_u32I2sSampleRate;    //I2S outputs clocks by sampling rate
    uint32_t m_u32I2sBitRate;       //I2S outputs clocks by bits resolution

    uint32_t m_u32I2sApllUp;        //I2S output clcoks speed control, increase APLL
    uint32_t m_u32I2sApll;          //Default I2S engine clock comes from APLL
    uint32_t m_u32I2sApllDn;        //I2S output clocks speed control, decrease APLL
    uint32_t m_u32I2sRefApll;       //Reference APLL for speed control

    uint16_t m_u16I2sMclkFactor;    //I2S MCLK divider
    uint8_t  m_u8I2sBclkFactor;     //I2S BCLK divider
    uint8_t  m_u8I2sShiftFlag;      //1=divide 2; 2=divide 4 for bits resolution to bytes unit

    //#if CONFIG_AUDIO_REC
    uint16_t m_u16I2sSmplCntRecTooFastStop; //Threshold to stop recording
    uint16_t m_u16I2sSmplCntRecTooFast;     //Threshold to decrease APLL
    uint16_t m_u16I2sSmplCntRecStart;       //Threshold to start recording
    uint16_t m_u16I2sSmplCntRecTooSlow;     //Threshold to increase APLL
    uint16_t m_u16I2sSmplCntRecTooSlowStop; //Threshold to stop recording

    uint16_t m_u16RecSmplCnt1;      //Sample count per 1-millisecond
    uint16_t m_u16RecSmplCnt2;      //Sample count per 1-millisecond for 44.1k series
    uint16_t m_u16RecSmplCnt1_;     //Reference sample count per 1-millisecond UAC 1.0 or per 125-microsecond UAC 2.0
    uint16_t m_u16RecSmplCnt2_;     //Reference sample count per 1-millisecond UAC 2.0 or per 125-microsecond UAC 2.0 for 44.1k series
    uint16_t m_u16RecMaxPayload11;  //Max packet size per 1-millisecond UAC 1.0
    uint16_t m_u16RecMaxPayload12;  //Max packet size per 1-millisecond UAC 1.0 for 44.1k series
    uint16_t m_u16RecMaxPayload21;  //Max packet size per 125-microsecond UAC 2.0
    uint16_t m_u16RecMaxPayload22;  //Max packet size per 125-microsecond UAC 2.0 for 44.1k series
    uint16_t m_u16RecMaxPayload1_;  //Reference max packet size per 1-millisecond UAC 1.0 or per 125-microsecond UAC 2.0
    uint16_t m_u16RecMaxPayload2_;  //Reference max packet size per 1-millisecond UAC 2.0 or per 125-microsecond UAC 2.0 for 44.1k series
    uint8_t  m_u8RecSmplSize;       //Bit resolution in byte size
    uint8_t  m_au8RecReserve1[1];

    uint32_t m_u32RecSampleRate;    //Sampling rate for recording
    uint8_t  m_u8RecBitRate;        //Bit resolution for recording
    uint8_t  m_u8RecChannels;       //Channel count for recording
    uint8_t  m_au8RecReserve3[1];
    uint8_t  m_u8RecMute;           //Recording volume control: mute=1 or un-mute=0
    int16_t  m_i16RecVolumeL;       //Recording volume control left channel
    int16_t  m_i16RecVolumeR;       //Recording volume control right channel
    int16_t  m_i16RecMaxVolume;     //Recording volume control max
    int16_t  m_i16RecMinVolume;     //Recording volume control min
    int16_t  m_i16RecResVolume;     //Recording volume control res
    uint8_t  m_u8RecFlag;           //Not recording=0; recording=1
    uint8_t  m_u8RecCnt2;           //Reference packet sequence
    int32_t  m_i32RecFlag;          //Reference ring buffer sample count size
    uint8_t  m_u8RecFlag2;          //Not recording=0; recording=1
    uint8_t  m_au8RecReserve2[3];
    uint8_t* m_pu8I2sRecPcmBuf;         //I2S ring buffer
    uint32_t m_u32I2sRecPcmBufIdx;      //Reference index
    uint32_t m_u32I2sRecPcmBufPreIdx;   //Previous index
    uint8_t* m_pu8RecPcmWorkBuf;        //Working ring buffer
    uint32_t m_u32RecPcmWorkBufIdx;     //Reference index, I2S ring buffer copy to working ring buffer
    uint32_t m_u32RecPcmWorkBufIdx2;    //Reference index, working ring buffer copy to temp buffer
    int32_t  m_i32RecPcmWorkSmplCnt;    //Reference working ring buffer in sample count unit
    uint8_t* m_pu8RecPcmTmpBuf;         //Temp buffer
    int32_t  m_i32RecPcmTmpBufLen;      //Send packet to host byte length
    uint8_t* m_pu8RecPacketSequence;    //algorithm for sending packet
    uint16_t m_u16RecPacketSequenceIdx; //Reference index
    uint16_t m_u16RecPacketSequenceCnt; //Reference packet byte size

    //#if CONFIG_AUDIO_PLAY
    uint32_t m_u32RecLPcmWorkBufIdx;    //Reference index, left channel
    uint32_t m_u32RecRPcmWorkBufIdx;    //Reference index, right channel
    int16_t  m_i16RecLPrevPcm;          //Previous left channel PCM
    int16_t  m_i16RecRPrevPcm;          //Previous right channel PCM
    uint32_t m_u32RecLInterpoResidual;  //Left channel interpolation residual
    uint32_t m_u32RecRInterpoResidual;  //Right channel interpolation residual
    uint32_t m_u32RecInterpoFactor;     //Interpolation factor
    uint32_t m_u32RecInterpoFactor2;    //Interpolation factor
    int32_t  m_i32RecPcmWorkResSmplCnt; //Tuning interpolation factor by hard-coded threshold
    //uint32_t m_u32RecResAdjustSpeedEnable;
    //#endif  // CONFIG_AUDIO_PLAY

    void     (*m_pfnRecSpeed)(struct S_AUDIO_LIB* psAudioLib);  //Speed control
    void     (*m_pfnRecStop)(struct S_AUDIO_LIB* psAudioLib);   //Stop recording
    void     (*m_pfnRecMode1)(struct S_AUDIO_LIB* psAudioLib);  //Send packet to host
    void     (*m_pfnRecMode2)(struct S_AUDIO_LIB* psAudioLib);  //Copy I2S ring buffer to working ring buffer
    void     (*m_pfnRecConfigMaxPayload10)(struct S_AUDIO_LIB* psAudioLib); //Calculate UAC 1.0 max packet size
    void     (*m_pfnRecConfigMaxPayload20)(struct S_AUDIO_LIB* psAudioLib); //Calculate UAC 2.0 max packet size
    //#endif  // CONFIG_AUDIO_REC

    //#if CONFIG_AUDIO_PLAY
    uint16_t m_u16I2sSmplCntPlayTooFastStop;    //Threshold to stop playing
    uint16_t m_u16I2sSmplCntPlayTooFast;        //Threshold to decrease APLL
    uint16_t m_u16I2sSmplCntPlayStart;          //Threshold to start playing
    uint16_t m_u16I2sSmplCntPlayTooSlow;        //Threshold to increase APLL
    uint16_t m_u16I2sSmplCntPlayTooSlowStop;    //Threshold to stop playing

    uint16_t m_u16PlaySmplCnt1;     //Sample count per 1-millisecond
    uint16_t m_u16PlaySmplCnt2;     //Sample count per 1-millisecond for 44.1k series
    uint16_t m_u16PlaySmplCnt1_;    //Reference sample count per 1-millisecond UAC 1.0 or per 125-microsecond UAC 2.0
    uint16_t m_u16PlaySmplCnt2_;    //Reference sample count per 1-millisecond UAC 2.0 or per 125-microsecond UAC 2.0 for 44.1k series
    uint16_t m_u16PlayMaxPayload11; //Max packet size per 1-millisecond UAC 1.0
    uint16_t m_u16PlayMaxPayload12; //Max packet size per 1-millisecond UAC 1.0 for 44.1k series
    uint16_t m_u16PlayMaxPayload21; //Max packet size per 125-microsecond UAC 2.0
    uint16_t m_u16PlayMaxPayload22; //Max packet size per 125-microsecond UAC 2.0 for 44.1k series
    uint16_t m_u16PlayMaxPayload1_; //Reference max packet size per 1-millisecond UAC 1.0 or per 125-microsecond UAC 2.0
    uint16_t m_u16PlayMaxPayload2_; //Reference max packet size per 1-millisecond UAC 2.0 or per 125-microsecond UAC 2.0 for 44.1k series
    uint8_t  m_u8PlaySmplSize;      //Bit resolution in byte size
    uint8_t  m_au8PlayReserve1[1];

    uint32_t m_u32PlaySampleRate;       //Sampling rate for playing
    uint8_t  m_u8PlayBitRate;           //Bit resolution for playing
    uint8_t  m_u8PlayChannels;          //Channel count for playing
    uint8_t  m_au8PlayReserve3[1];
    uint8_t  m_u8PlayMute;              //Playing volume control: mute=1 or un-mute=0
    int16_t  m_i16PlayVolumeL;          //Playing volume control left channel
    int16_t  m_i16PlayVolumeR;          //Playing volume control right channel
    int16_t  m_i16PlayMaxVolume;        //Playing volume control max
    int16_t  m_i16PlayMinVolume;        //Playing volume control min
    int16_t  m_i16PlayResVolume;        //Playing volume control res
    uint8_t  m_u8PlayFlag;              //Not playing=0; playing=1
    uint8_t  m_au8PlayReserve2[1];
    uint8_t* m_pu8I2sPlayPcmBuf;        //I2S ring buffer
    uint32_t m_u32I2sPlayPcmBufIdx;     //Reference index
    uint8_t* m_pu8PlayPcmWorkBuf;       //Working ring buffer
    uint32_t m_u32PlayPcmWorkBufIdx;    //Reference index, I2S ring buffer copy from working ring buffer
    uint32_t m_u32PlayPcmWorkBufIdx2;   //Reference index, working ring buffer receive from host (copy to temp buffer)
    int32_t  m_i32PlayPcmWorkSmplCnt;   //Reference working ring buffer in sample count unit
    uint8_t* m_pu8PlayPcmTmpBuf;        //Temp buffer
    int32_t  m_i32PlayPcmTmpBufLen;     //Packet length received from host

    //#if CONFIG_AUDIO_REC
    uint32_t m_u32PlayLPcmWorkBufIdx;       //Reference index, left channel
    uint32_t m_u32PlayRPcmWorkBufIdx;       //Reference index, right channel
    int16_t  m_i16PlayLPrevPcm;             //Previous left channel PCM
    int16_t  m_i16PlayRPrevPcm;             //Previous right channel PCM
    uint32_t m_u32PlayLInterpoResidual;     //Left channel interpolation residual
    uint32_t m_u32PlayRInterpoResidual;     //Right channel interpolation residual
    uint32_t m_u32PlayInterpoFactor;        //Interpolation factor
    uint32_t m_u32PlayInterpoFactor2;       //Interpolation factor
    uint32_t m_u32PlayResAdjustSpeedEnable; //Tuning interpolation factor by threshold
    //#endif  // CONFIG_AUDIO_REC

    void     (*m_pfnPlaySpeed)(struct S_AUDIO_LIB* psAudioLib); //Speed control
    void     (*m_pfnPlayStop)(struct S_AUDIO_LIB* psAudioLib);  //Stop playing
    void     (*m_pfnPlayMode1)(struct S_AUDIO_LIB* psAudioLib, int32_t i32Len); //Receive packet from host (to temp buffer)
    void     (*m_pfnPlayMode2)(struct S_AUDIO_LIB* psAudioLib); //Copy from temp buffer to working ring buffer
    void     (*m_pfnPlayConfigMaxPayload10)(struct S_AUDIO_LIB* psAudioLib);    //Calculate UAC 1.0 max packet size
    void     (*m_pfnPlayConfigMaxPayload20)(struct S_AUDIO_LIB* psAudioLib);    //Calculate UAC 1.0 max packet size
    //#endif  // CONFIG_AUDIO_PLAY
} S_AUDIO_LIB;

void AudioLib_Start(void);
void AudioLib_Process(void);

void AudioLib_InitRecCallback(S_AUDIO_LIB* psAudioLib);
void AudioLib_InitPlayCallback(S_AUDIO_LIB* psAudioLib);

void AudioLib_Init3(S_AUDIO_LIB* psAudioLib, uint32_t u32I2sSampleRate, uint32_t u32I2sBitRate);

void Codec_Init(void);
void Codec_Vol(S_AUDIO_LIB* psAudioLib);

/* private define */
#define _I2S_SAMPLE_RATE    48000           /* must=48000 if CONFIG_AUDIO_REC=1 and CONFIG_AUDIO_PLAY=1 */
#define _I2S_BIT_RATE       I2S_DATABIT_16  /* must=I2S_DATABIT_16 if CONFIG_AUDIO_REC=1 and CONFIG_AUDIO_PLAY=1 */

#ifdef __cplusplus
}
#endif

#endif  /* __AUDIOLIB_H__ */
