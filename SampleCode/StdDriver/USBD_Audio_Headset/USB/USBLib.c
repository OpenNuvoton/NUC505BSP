#include "NUC505Series.h"

#include "USBLib.h"

#include "AudioLib.h"
#include "demo.h"

extern S_AUDIO_LIB g_sAudioLib;

extern uint8_t     g_timeout;

/* Global variables for Audio class */
#if defined __UAC20_ONLY__ || defined __UAC10_20__
volatile uint8_t  g_uac_20_mode_flag = 1;
#else
volatile uint8_t  g_uac_20_mode_flag = 0;
#endif

volatile uint32_t g_usbd_rx_flag = 0;
volatile uint32_t g_usbd_tx_flag = 0;
volatile uint8_t  g_uac_10_flag = 0;
volatile uint8_t  g_uac_20_flag = 0;
volatile uint32_t g_u32ClassOUT_10 = 0;
volatile uint32_t g_u32ClassOUT_20 = 0;
volatile uint8_t  g_start_timer_flag = 0;
volatile uint8_t  g_plug_flag = 0;
volatile int32_t  g_hid_type = 0;
volatile uint8_t  g_u8EPCReady = 0;
volatile uint32_t g_u32count = 0;


void USBLib_Init(S_AUDIO_LIB* psAudioLib)
{
    /* Show the UAC device property */
#if 1
#if defined __UAC10_ONLY__ || defined __UAC10_20__
#ifdef __BOTH__
    printf("NUC505 USB UAC 1.0 Headset Earphone");
#elif defined __MIC_ONLY__
    printf("NUC505 USB UAC 1.0 Microphone");
#elif defined __SPEAKER_ONLY__
    printf("NUC505 USB UAC 1.0 Speaker");
#endif
#ifdef __HID__
    printf(" + HID");
#ifdef __KEYBOARD__
    printf(" Keyboard");
#elif defined __MEDIAKEY__
    printf(" Mediakey");
#endif
#endif
    printf("\n");
#endif
#if defined __UAC20_ONLY__ || defined __UAC10_20__
#ifdef __BOTH20__
    printf("NUC505 USB UAC 2.0 Headset Earphone");
#elif defined __MIC_ONLY20__
    printf("NUC505 USB UAC 2.0 Microphone");
#elif defined __SPEAKER_ONLY20__
    printf("NUC505 USB UAC 2.0 Speaker");
#endif
#ifdef __HID20__
    printf(" + HID");
#ifdef __KEYBOARD20__
    printf(" Keyboard");
#elif defined __MEDIAKEY20__
    printf(" Mediakey");
#endif
#endif
    printf("\n");
#endif
#endif

#if defined __HID20__ || defined __HID__
    /* Init GPIO for HID */
    GPIO_SetPullMode(PC, BIT4, GPIO_PULL_UP_EN);
    GPIO_SetPullMode(PC, BIT3, GPIO_PULL_UP_EN);
    GPIO_SetPullMode(PC, BIT2, GPIO_PULL_UP_EN);
    GPIO_SetPullMode(PC, BIT1, GPIO_PULL_UP_EN);
    GPIO_SetPullMode(PC, BIT0, GPIO_PULL_UP_EN);
    GPIO_SetMode(PC, BIT4, GPIO_MODE_INPUT);
    GPIO_SetMode(PC, BIT3, GPIO_MODE_INPUT);
    GPIO_SetMode(PC, BIT2, GPIO_MODE_INPUT);
    GPIO_SetMode(PC, BIT1, GPIO_MODE_INPUT);
    GPIO_SetMode(PC, BIT0, GPIO_MODE_INPUT);
    GPIO_SET_DEBOUNCE_TIME(NULL, GPIO_DBCTL_DBCLKSEL_32768);
    GPIO_ENABLE_DEBOUNCE(PC, BIT4);
    GPIO_ENABLE_DEBOUNCE(PC, BIT3);
    GPIO_ENABLE_DEBOUNCE(PC, BIT2);
    GPIO_ENABLE_DEBOUNCE(PC, BIT1);
    GPIO_ENABLE_DEBOUNCE(PC, BIT0);
#endif

#ifdef __UAC10_20__
    g_u32count= 0;
    if(g_uac_20_mode_flag)         /* Run UAC 2.0 mode */
    {
        printf("Try - UAC2.0\n");
        USBD_Open(&gsInfo_20, UAC_ClassRequest_20, UAC_SetInterface_20);
        /* Endpoint configuration */
        UAC_Init_20( psAudioLib );
#ifdef __HID20__
#ifdef __MEDIAKEY20__
        g_hid_type = HID_TYPE_MEDIAKEY;
#else
        g_hid_type = HID_TYPE_KEYBOARD;
#endif
#endif
    }
    else                           /* Run UAC 1.0 mode */
    {
        printf("Try - UAC1.0\n");
        USBD_Open(&gsInfo_10, UAC_ClassRequest_10, UAC_SetInterface_10);
        /* Endpoint configuration */
        UAC_Init_10( psAudioLib );
#ifdef __HID__
#ifdef __MEDIAKEY__
        g_hid_type = HID_TYPE_MEDIAKEY;
#else
        g_hid_type = HID_TYPE_KEYBOARD;
#endif
#endif
    }
#elif defined __UAC20_ONLY__
    USBD_Open(&gsInfo_20, UAC_ClassRequest_20, UAC_SetInterface_20);
    /* Endpoint configuration */
    UAC_Init_20(psAudioLib);
#ifdef __HID20__
#ifdef __MEDIAKEY20__
    g_hid_type = HID_TYPE_MEDIAKEY;
#else
    g_hid_type = HID_TYPE_KEYBOARD;
#endif
#endif
#else
    USBD_Open(&gsInfo_10, UAC_ClassRequest_10, UAC_SetInterface_10);
    /* Endpoint configuration */
    UAC_Init_10(psAudioLib);
#ifdef __HID__
#ifdef __MEDIAKEY__
    g_hid_type = HID_TYPE_MEDIAKEY;
#else
    g_hid_type = HID_TYPE_KEYBOARD;
#endif
#endif
#endif

    NVIC_EnableIRQ(USBD_IRQn);
    USBD_Start();
}

void USBD_IRQHandler(void)
{
    S_AUDIO_LIB* psAudioLib = &g_sAudioLib;

    if(g_uac_20_mode_flag)
        USBD_IRQHandler_20(psAudioLib);
    else
        USBD_IRQHandler_10(psAudioLib);
}

void USBLib_Start(void)
{
    S_AUDIO_LIB* psAudioLib = &g_sAudioLib;

    USBLib_Init( psAudioLib );
}

void USBLib_Process(void)
{
    /* executed in main loop */

#ifdef __UAC10_20__
    if(g_uac_10_flag == 1)        /* Try UAC 1.0 */
    {
        USBD_SET_SE0();
        g_uac_20_mode_flag = 0;
        g_uac_10_flag = 2;        /* Run UAC 1.0 mode */
        g_uac_20_flag = 0;
        while(g_u32count < g_timeout + 10);  /* Delay 1 second */
        USBLib_Start();                      /* Change mode */
    }
    if(g_uac_20_mode_flag && g_uac_20_flag == 1)   /* UAC 2.0 Bus Enumeration Pass */
    {
        printf("  UAC2.0 USB Bus Enumeration Pass!!\n");
        g_uac_20_flag  = 2;        /* Run UAC 2.0 mode */
    }
#endif
    if (USBD_IS_ATTACHED())
    {
        /* USB Plug In */
        USBD_ENABLE_USB();
        if(g_plug_flag == 0)       /* USB plug */
        {
            printf("USB Plug\n");
            g_plug_flag = 1;
#ifdef __UAC10_20__
            if(g_start_timer_flag == 0 && g_uac_20_mode_flag)   /* Start counting for UAC 2.0 Timeout */
            {
                printf("USB Bus Enumeration!\n");
                g_start_timer_flag = 1;
                //TIMER_Start(TIMER0);
            }
#endif
        }
    }
    else
    {
        if(g_plug_flag == 1)         /* USB Un-plug */
        {
            printf("USB Un-Plug\n");
            g_plug_flag = 0;
#ifdef __UAC10_20__
            g_uac_20_mode_flag = 1;  /* Try UAC 2.0 first */
            g_uac_10_flag = 0;
            g_uac_20_flag = 0;
            g_start_timer_flag = 0;
            g_usbd_tx_flag = 0;
            g_usbd_rx_flag = 0;
            USBLib_Start();          /* Change mode */
#endif
        }
    }

#if defined __HID20__ || defined __HID__
    HID_UpdateKbData();
#endif
}
