/********************************************************************************************
 UAC mode Setting
********************************************************************************************/
#define __UAC10_ONLY__      /* Sample code is for UAC 1.0 Only */
// #define __UAC20_ONLY__      /* Sample code is for UAC 2.0 Only */
// #define __UAC10_20__        /* Sample code is for UAC 2.0 & UAC 1.0 (Try UAC 2.0 First) */

/********************************************************************************************
 UAC 1.0 Setting
********************************************************************************************/
//#define __MIC_ONLY__         /* Enable MIC only for UAC 1.0 */
//#define __SPEAKER_ONLY__     /* Enable SPK only for UAC 1.0*/
#define __BOTH__             /* Enable MIC and SPK for UAC 1.0 */
#define __HID__              /* Enable HID for UAC 1.0 */

#ifdef __HID__
#define __MEDIAKEY__        /* Select Mediakey for HID */
//#define __KEYBOARD__        /* Select Keyboard for HID */
#endif
/********************************************************************************************
 UAC 2.0 Setting
********************************************************************************************/
// #define __MIC_ONLY20__       /* Enable MIC only for UAC 2.0 */
// #define __SPEAKER_ONLY20__   /* Enable SPK only for UAC 2.0*/
#define __BOTH20__           /* Enable MIC and SPK for UAC 2.0 */
#define __HID20__            /* Enable HID for UAC 2.0 */

#ifdef __HID20__
#define __MEDIAKEY20__      /* Select Mediakey for HID */
//     #define __KEYBOARD20__      /* Select Keyboard for HID */
#endif

extern S_USBD_INFO_T gsInfo_10,gsInfo_20;

void USBD_IRQHandler_10(S_AUDIO_LIB* psAudioLib);
void USBD_IRQHandler_20(S_AUDIO_LIB* psAudioLib);
void UAC_ClassRequest_10(void);
void UAC_ClassRequest_20(void);
void UAC_ClassOUT_10(S_AUDIO_LIB* psAudioLib);
void UAC_ClassOUT_20(S_AUDIO_LIB* psAudioLib);
void UAC_SetInterface_10(uint32_t u32AltInterface);
void UAC_SetInterface_20(uint32_t u32AltInterface);
void UAC_Init_10(S_AUDIO_LIB* psAudioLib);
void UAC_Init_20(S_AUDIO_LIB* psAudioLib);
void EPA_Handler(S_AUDIO_LIB* psAudioLib);
void EPB_Handler(S_AUDIO_LIB* psAudioLib);
void EPC_Handler(void);
void EPE_Handler(void);
void EPF_Handler(void);
void HID_UpdateKbData(void);

#define HID_CTRL_MUTE        0x01
#define HID_CTRL_VOLUME_INC  0x02
#define HID_CTRL_VOLUME_DEC  0x04

#define HID_CTRL_EJECT       0x08
#define HID_CTRL_PLAY        0x01
#define HID_CTRL_STOP        0x02
#define HID_CTRL_PAUSE       0x04
#define HID_CTRL_NEXT        0x08
#define HID_CTRL_PREVIOUS    0x10
#define HID_CTRL_RECORD      0x20
#define HID_CTRL_REWIND      0x40
#define HID_CTRL_FF          0x80

#define HID_TYPE_MEDIAKEY   0
#define HID_TYPE_KEYBOARD   1

#define UAC_MICROPHONE  0
#define UAC_SPEAKER     1

/*-------------------------------------------------------------*/
/* Define the interrupt In EP number */
#define ISO_IN_EP_NUM          0x01
#define HID_IN_EP_NUM          0x03
#define ISO_OUT_EP_NUM         0x02
#define ISO_FEEDBACK_ENDPOINT  0x05
#define HID_INT_EP_NUM         HID_IN_EP_NUM

#define EPA_BUF_LEN    768

/********************************************************************************************
 UAC 1.0 define
********************************************************************************************/

/* Define the vendor id and product id */
#define USBD_VID                    0x0420
#ifdef __BOTH__
#ifdef __HID__
#ifdef __KEYBOARD__
#define USBD_PID        0x1421
#elif defined __MEDIAKEY__
#define USBD_PID        0x1422
#endif
#else
#define USBD_PID            0x1423
#endif
#elif defined __MIC_ONLY__
#ifdef __HID__
#ifdef __KEYBOARD__
#define USBD_PID        0x1424
#elif defined __MEDIAKEY__
#define USBD_PID        0x1425
#endif
#else
#define USBD_PID             0x1426
#endif
#elif defined __SPEAKER_ONLY__
#ifdef __HID__
#ifdef __KEYBOARD__
#define USBD_PID        0x1427
#elif defined __MEDIAKEY__
#define USBD_PID        0x1428
#endif
#else
#define USBD_PID            0x1429
#endif
#endif

/********************************************************************************************
 UAC 2.0 define
********************************************************************************************/
/* Define the vendor id and product id */
#define USBD_VID20                    0x042A
#ifdef __BOTH20__
#ifdef __HID20__
#ifdef __KEYBOARD20__
#define USBD_PID20        0x142B
#elif defined __MEDIAKEY20__
#define USBD_PID20        0x142C
#endif
#else
#define USBD_PID20            0x142D
#endif
#elif defined __MIC_ONLY20__
#ifdef __HID20__
#ifdef __KEYBOARD20__
#define USBD_PID20        0x142E
#elif defined __MEDIAKEY20__
#define USBD_PID20        0x142F
#endif
#else
#define USBD_PID20            0x1430
#endif
#elif defined __SPEAKER_ONLY20__
#ifdef __HID20__
#ifdef __KEYBOARD20__
#define USBD_PID20        0x1431
#elif defined __MEDIAKEY20__
#define USBD_PID20        0x1432
#endif
#else
#define USBD_PID20            0x1433
#endif
#endif
