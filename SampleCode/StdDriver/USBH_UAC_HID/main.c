/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 12 $
 * $Date: 14/10/02 6:55p $
 * @brief    This sample shows how to use an UAC+HID device.
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "NUC505Series.h"
#include "usbh_core.h"
#include "usbh_uac.h"
#include "usbh_hid.h"

uint8_t  au_in_buff[4096];
uint8_t hid_rdata[64];

static volatile int  au_in_cnt, au_out_cnt;

static volatile int  g_tick_cnt = 0;


#define HOST_LIKE_PORT1_0                       0x10
#define HOST_LIKE_PORT1_1                       0x20
#define HOST_LIKE_PORT2_0                       0x00
#define HOST_LIKE_PORT1_DISABLE         0xFF
#define HOST_LIKE_PORT2_DISABLE         0xFF

void USB_PortInit(uint32_t u32Port1, uint32_t u32Port2)
{
    SYS->WAKEUP = SYS->WAKEUP | SYS_WAKEUP_USBHWF_Msk;
    switch(u32Port1)
    {
    //port 1
    case HOST_LIKE_PORT1_DISABLE:
        printf("USB host like port 1 Disable\n");
        break;
    case HOST_LIKE_PORT1_0:
        SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk|SYS_GPB_MFPH_PB13MFP_Msk)) | (0x2 << SYS_GPB_MFPH_PB12MFP_Pos) | (0x2 << SYS_GPB_MFPH_PB13MFP_Pos);
        printf("USB host like port 1 from GPB12 & GPB13\n");
        break;
    case HOST_LIKE_PORT1_1:
        SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB14MFP_Msk|SYS_GPB_MFPH_PB15MFP_Msk)) | (0x1 << SYS_GPB_MFPH_PB14MFP_Pos) | (0x1 << SYS_GPB_MFPH_PB15MFP_Pos);
        printf("USB host like port 1 from GPB14 & GPB15\n");
        break;
    }
    switch(u32Port2)
    {
    //port 2
    case HOST_LIKE_PORT2_DISABLE:
        printf("USB host like port 2 Disable\n");
        break;
    case HOST_LIKE_PORT2_0:
        SYS->GPC_MFPH = (SYS->GPC_MFPH & ~(SYS_GPC_MFPH_PC13MFP_Msk|SYS_GPC_MFPH_PC14MFP_Msk)) | (0x1 << SYS_GPC_MFPH_PC13MFP_Pos) | (0x1 << SYS_GPC_MFPH_PC14MFP_Pos);
        printf("USB host like port 2 from GPC13 & GPC14\n");
        break;
    }
    USBH->HcMiscControl = (USBH->HcMiscControl & ~(USBH_HcMiscControl_DPRT1_Msk | USBH_HcMiscControl_DPRT2_Msk)) | ((u32Port1 & 0x01) << USBH_HcMiscControl_DPRT1_Pos) | ((u32Port2 & 0x01) << USBH_HcMiscControl_DPRT2_Pos);

}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable  XTAL */
    CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk;

    /* USB Host Clock Source MUST be multiple of 48MHz */
    CLK_SetCoreClock(96000000);

    /* Set PCLK divider */
    CLK_SetModuleClock(PCLK_MODULE, (uint32_t)NULL, 1);

    /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Enable UART IP clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_UART0_SRC_EXT, 0);

    /* Enable USB Host IP clock */
    CLK_EnableModuleClock(USBH_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(USBH_MODULE, CLK_USBH_SRC_PLL, CLK_GetPLLClockFreq() / 48000000 - 1);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Configure multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB0MFP_Msk) ) | SYS_GPB_MFPL_PB0MFP_UART0_TXD;
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB1MFP_Msk) ) | SYS_GPB_MFPL_PB1MFP_UART0_RXD;

}

void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART module */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}



/**
 *  @brief  Audio-in data callback function.
 *          UAC driver notify user that audio-in data has been moved into user audio-in buffer,
 *          which is provided by user application via UAC_InstallIsoInCbFun().
 *  @param[in] dev    Audio Class device
 *  @param[in] data   Available audio-in data, which is located in user audio-in buffer.
 *  @param[in] len    Length of available audio-in data started from <data>.
 *  @return   UAC driver does not check this return value.
 */
int audio_in_callback(UAC_DEV_T *dev, uint8_t *data, int len)
{
    au_in_cnt += len;
    //printf("I %x,%x\n", (int)data & 0xffff, len);   // UART send too many will cause ISO transfer time overrun

    // Add your code here to get audio-in data ...
    // For example, memcpy(audio_record_buffer, data, len);
    // . . .

    return 0;
}


/**
 *  @brief  Audio-out data callback function.
 *          UAC driver requests user to move audio-out data into the specified address. The audio-out
 *          data will then be send to UAC device via isochronous-out pipe.
 *  @param[in] dev    Audio Class device
 *  @param[in] data   Application should move audio-out data into this buffer.
 *  @param[in] len    Maximum length of audio-out data can be moved.
 *  @return   Actual length of audio data moved.
 */
int audio_out_callback(UAC_DEV_T *dev, uint8_t *data, int len)
{
    au_out_cnt += len;
    //printf("O %x,%x\n", (int)data & 0xffff, len);   // UART send too many will cause ISO transfer time overrun

    // Add your code here to put audio-out data ...
    // For example, memcpy(data, playback_buffer, actual_len);
    //              return actual_len;
    // . . .

    return 192;   // for 48000 stero Hz
}


void  uac_control_example(UAC_DEV_T *uac_dev)
{
    uint8_t    data[8];
    uint32_t   srate[4];
    uint32_t   val32;
    uint16_t   val16;
    uint8_t    val8;
    int        i, ret;

    printf("\nGet channel information ===>\n");

    if(uac_dev->au_out_ifnum != -1)
    {
        /*-------------------------------------------------------------*/
        /*  Get channel number information                             */
        /*-------------------------------------------------------------*/
        ret = UAC_GetChannelNumber(uac_dev, UAC_SPEAKER);
        if (ret < 0)
            printf("    Failed to get speaker's channel number.\n");
        else
            printf("    Speaker: %d\n", ret);
    }
    if(uac_dev->au_in_ifnum != -1)
    {
        ret = UAC_GetChannelNumber(uac_dev, UAC_MICROPHONE);
        if (ret < 0)
            printf("    Failed to get microphone's channel number.\n");
        else
            printf("    Microphone: %d\n", ret);
    }
    printf("\nGet subframe bit resolution ===>\n");

    /*-------------------------------------------------------------*/
    /*  Get audio subframe bit resolution information              */
    /*-------------------------------------------------------------*/
    if(uac_dev->au_out_ifnum != -1)
    {
        ret = UAC_GetBitResolution(uac_dev, UAC_SPEAKER, &val8);
        if (ret < 0)
            printf("    Failed to get speaker's bit resoltion.\n");
        else
        {
            printf("    Speaker audio subframe size: %d bytes\n", val8);
            printf("    Speaker subframe bit resolution: %d\n", ret);
        }
    }
    if(uac_dev->au_in_ifnum != -1)
    {
        ret = UAC_GetBitResolution(uac_dev, UAC_MICROPHONE, &val8);
        if (ret < 0)
            printf("    Failed to get microphone's bit resoltion.\n");
        else
        {
            printf("    Microphone audio subframe size: %d bytes\n", val8);
            printf("    Microphone subframe bit resolution: %d\n", ret);
        }
    }
    printf("\nGet sampling rate list ===>\n");

    /*-------------------------------------------------------------*/
    /*  Get audio subframe bit resolution information              */
    /*-------------------------------------------------------------*/

    if(uac_dev->au_out_ifnum != -1)
    {
        ret = UAC_GetSamplingRate(uac_dev, UAC_SPEAKER, (uint32_t *)&srate[0], 4, &val8);
        if (ret < 0)
            printf("    Failed to get speaker's sampling rate.\n");
        else
        {
            if (val8 == 0)
                printf("    Speaker sampling rate range: %d ~ %d Hz\n", srate[0], srate[1]);
            else
            {
                for (i = 0; i < val8; i++)
                    printf("    Speaker sampling rate: %d\n", srate[i]);
            }
        }
    }
    if(uac_dev->au_in_ifnum != -1)
    {
        ret = UAC_GetSamplingRate(uac_dev, UAC_MICROPHONE, (uint32_t *)&srate[0], 4, &val8);
        if (ret < 0)
            printf("    Failed to get microphone's sampling rate.\n");
        else
        {
            if (val8 == 0)
                printf("    Microphone sampling rate range: %d ~ %d Hz\n", srate[0], srate[1]);
            else
            {
                for (i = 0; i < val8; i++)
                    printf("    Microphone sampling rate: %d\n", srate[i]);
            }
        }
    }


    if(uac_dev->au_out_ifnum != -1)
    {
        printf("\nSpeaker mute control ===>\n");
        /*-------------------------------------------------------------*/
        /*  Get current mute value of UAC device's speaker.            */
        /*-------------------------------------------------------------*/

        if (UAC_MuteControl(uac_dev, UAC_SPEAKER, UAC_GET_CUR, UAC_CH_MASTER, data) == UAC_RET_OK)
        {
            printf("    Speaker mute state is %d.\n", data[0]);
        }
        else
            printf("    Failed to get speaker mute state!\n");

        printf("\nSpeaker L(F) volume control ===>\n");

        /*--------------------------------------------------------------------------*/
        /*  Get current volume value of UAC device's speaker left channel.          */
        /*--------------------------------------------------------------------------*/
        if (UAC_VolumeControl(uac_dev, UAC_SPEAKER, UAC_GET_CUR, UAC_CH_LEFT_FRONT, &val16) == UAC_RET_OK)
        {
            printf("    Speaker L(F) volume is 0x%x.\n", val16);
        }
        else
            printf("    Failed to get seaker L(F) volume!\n");

        /*--------------------------------------------------------------------------*/
        /*  Get minimum volume value of UAC device's speaker left channel.          */
        /*--------------------------------------------------------------------------*/
        if (UAC_VolumeControl(uac_dev, UAC_SPEAKER, UAC_GET_MIN, UAC_CH_LEFT_FRONT, &val16) == UAC_RET_OK)
        {
            printf("    Speaker L(F) minimum volume is 0x%x.\n", val16);
        }
        else
            printf("    Failed to get speaker L(F) minimum volume!\n");

        /*--------------------------------------------------------------------------*/
        /*  Get maximum volume value of UAC device's speaker left channel.          */
        /*--------------------------------------------------------------------------*/
        if (UAC_VolumeControl(uac_dev, UAC_SPEAKER, UAC_GET_MAX, UAC_CH_LEFT_FRONT, &val16) == UAC_RET_OK)
        {
            printf("    Speaker L(F) maximum volume is 0x%x.\n", val16);
        }
        else
            printf("    Failed to get speaker L(F) maximum volume!\n");

        /*--------------------------------------------------------------------------*/
        /*  Get volume resolution of UAC device's speaker left channel.             */
        /*--------------------------------------------------------------------------*/
        if (UAC_VolumeControl(uac_dev, UAC_SPEAKER, UAC_GET_RES, UAC_CH_LEFT_FRONT, &val16) == UAC_RET_OK)
        {
            printf("    Speaker L(F) volume resolution is 0x%x.\n", val16);
        }
        else
            printf("    Failed to get speaker L(F) volume resolution!\n");


        printf("\nSpeaker R(F) volume control ===>\n");

        /*--------------------------------------------------------------------------*/
        /*  Get current volume value of UAC device's speaker right channel.         */
        /*--------------------------------------------------------------------------*/
        if (UAC_VolumeControl(uac_dev, UAC_SPEAKER, UAC_GET_CUR, UAC_CH_RIGHT_FRONT, &val16) == UAC_RET_OK)
        {
            printf("    Speaker R(F) volume is 0x%x.\n", val16);
        }
        else
            printf("    Failed to get speaker R(F) volume!\n");

        /*--------------------------------------------------------------------------*/
        /*  Get minimum volume value of UAC device's speaker right channel.         */
        /*--------------------------------------------------------------------------*/
        if (UAC_VolumeControl(uac_dev, UAC_SPEAKER, UAC_GET_MIN, UAC_CH_RIGHT_FRONT, &val16) == UAC_RET_OK)
        {
            printf("    Speaker R(F) minimum volume is 0x%x.\n", val16);
        }
        else
            printf("    Failed to get speaker R(F) minimum volume!\n");

        /*--------------------------------------------------------------------------*/
        /*  Get maximum volume value of UAC device's speaker right channel.         */
        /*--------------------------------------------------------------------------*/
        if (UAC_VolumeControl(uac_dev, UAC_SPEAKER, UAC_GET_MAX, UAC_CH_RIGHT_FRONT, &val16) == UAC_RET_OK)
        {
            printf("    Speaker R(F) maximum volume is 0x%x.\n", val16);
        }
        else
            printf("    Failed to get speaker R(F) maximum volume!\n");

        /*--------------------------------------------------------------------------*/
        /*  Get volume resolution of UAC device's speaker right channel.            */
        /*--------------------------------------------------------------------------*/
        if (UAC_VolumeControl(uac_dev, UAC_SPEAKER, UAC_GET_RES, UAC_CH_RIGHT_FRONT, &val16) == UAC_RET_OK)
        {
            printf("    Speaker R(F) volume resolution is 0x%x.\n", val16);
        }
        else
            printf("    Failed to get speaker R(F) volume resolution!\n");
    }

    if(uac_dev->au_in_ifnum != -1 && uac_dev->udev->descriptor.idVendor != 0x5AC)
    {
        printf("\nMicrophone mute control ===>\n");

        /*-------------------------------------------------------------*/
        /*  Get current mute value of UAC device's speaker.            */
        /*-------------------------------------------------------------*/
        if (UAC_MuteControl(uac_dev, UAC_MICROPHONE, UAC_GET_CUR, UAC_CH_MASTER, data) == UAC_RET_OK)
        {
            printf("    Microphone mute state is %d.\n", data[0]);
        }
        else
            printf("    Failed to get microphone mute state!\n");

        printf("\nMicrophone volume control ===>\n");

        /*-------------------------------------------------------------*/
        /*  Get current volume value of UAC device's microphone.       */
        /*-------------------------------------------------------------*/
        if (UAC_VolumeControl(uac_dev, UAC_MICROPHONE, UAC_GET_CUR, UAC_CH_MASTER, &val16) == UAC_RET_OK)
        {
            printf("    Microphone current volume is 0x%x.\n", val16);
        }
        else
            printf("    Failed to get microphone current volume!\n");

        /*-------------------------------------------------------------*/
        /*  Get minimum volume value of UAC device's microphone.       */
        /*-------------------------------------------------------------*/
        if (UAC_VolumeControl(uac_dev, UAC_MICROPHONE, UAC_GET_MIN, UAC_CH_MASTER, &val16) == UAC_RET_OK)
        {
            printf("    Microphone minimum volume is 0x%x.\n", val16);
        }
        else
            printf("    Failed to get microphone minimum volume!\n");

        /*-------------------------------------------------------------*/
        /*  Get maximum volume value of UAC device's microphone.       */
        /*-------------------------------------------------------------*/
        if (UAC_VolumeControl(uac_dev, UAC_MICROPHONE, UAC_GET_MAX, UAC_CH_MASTER, &val16) == UAC_RET_OK)
        {
            printf("    Microphone maximum volume is 0x%x.\n", val16);
        }
        else
            printf("    Failed to get microphone maximum volume!\n");
    }
    printf("\nSampling rate control ===>\n");

    if(uac_dev->au_out_ifnum != -1)
    {
        /*-------------------------------------------------------------*/
        /*  Set new sampling rate value of UAC device's speaker.       */
        /*-------------------------------------------------------------*/
        val32 = 48000;
        if (UAC_SamplingRateControl(uac_dev, UAC_SPEAKER, UAC_SET_CUR, &val32) == UAC_RET_OK)
        {
            printf("    Speaker's current sampling rate is %d.\n", val32);
        }
        else
            printf("    Failed to set speaker's current sampling rate!\n");

        /*-------------------------------------------------------------*/
        /*  Get current sampling rate value of UAC device's speaker.   */
        /*-------------------------------------------------------------*/
        if (UAC_SamplingRateControl(uac_dev, UAC_SPEAKER, UAC_GET_CUR, &val32) == UAC_RET_OK)
        {
            printf("    Speaker's current sampling rate is %d.\n", val32);
        }
        else
            printf("    Failed to get speaker's current sampling rate!\n");
    }
    if(uac_dev->au_in_ifnum != -1)
    {
        /*-------------------------------------------------------------*/
        /*  Set new sampling rate value of UAC device's microphone.    */
        /*-------------------------------------------------------------*/
        val32 = 48000;
        if (UAC_SamplingRateControl(uac_dev, UAC_MICROPHONE, UAC_SET_CUR, &val32) == UAC_RET_OK)
        {
            printf("    Microphone's current sampling rate is %d.\n", val32);
        }
        else
            printf("    Failed to set microphone's current sampling rate!\n");

        if(uac_dev->udev->descriptor.idVendor != 0x5AC)
        {
            /*-------------------------------------------------------------*/
            /*  Get current sampling rate value of UAC device's microphone.*/
            /*-------------------------------------------------------------*/
            if (UAC_SamplingRateControl(uac_dev, UAC_MICROPHONE, UAC_GET_CUR, &val32) == UAC_RET_OK)
            {
                printf("    Microphone's current sampling rate is %d.\n", val32);
            }
            else
                printf("    Failed to get microphone's current sampling rate!\n");
        }
    }
}


void  int_read_callback(HID_DEV_T *hdev, uint8_t *rdata, int data_len)
{
    int  i;
    if(memcmp(hid_rdata, rdata ,data_len)!= 0)
    {
        printf("INT-in pipe data %d bytes received =>\n", data_len);
        for (i = 0; i < data_len; i++)
            printf("0x%02x ", rdata[i]);
        printf("\n");
    }
    memcpy(hid_rdata, rdata ,data_len);
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int32_t main(void)
{
    uint32_t u32UsbhPort1 = HOST_LIKE_PORT1_DISABLE;
    char Item ;
    UAC_DEV_T    *uac_dev;
    HID_DEV_T    *hdev;

    /* Init System, IP clock and multi-function I/O */
    SYS_Init(); //In the end of SYS_Init() will issue SYS_LockReg() to lock protected register. If user want to write protected register, please issue SYS_UnlockReg() to unlock protected register.

    /* Init UART0 for printf */
    UART0_Init();

    printf("+--------------------------------------+\n");
    printf("|                                      |\n");
    printf("|     USB Host HID sample program      |\n");
    printf("|                                      |\n");
    printf("+--------------------------------------+\n");

    do
    {
        printf("============================================================================================\n");
        printf("Please select the USB host port 1 through GPIO\n");
        printf("[A] GPB12 and GPB13\n");
        printf("[B] GPB14 and GPB15\n");
        printf("[C] Disable\n");
        printf("============================================================================================\n");

        scanf("%c",&Item);
        switch(Item)
        {
        case 'A':
        case 'a':
            u32UsbhPort1 = HOST_LIKE_PORT1_0;
            goto next;
        case 'B':
        case 'b':
            u32UsbhPort1 = HOST_LIKE_PORT1_1;
            goto next;
        case 'C':
        case 'c':
            goto next;

        }
    }
    while(1);
next:
    do
    {
        printf("============================================================================================\n");
        printf("Please select the USB host port 2 through GPIO\n");
        printf("[A] GPC13 and GPC14\n");
        printf("[B] Disable\n");
        printf("============================================================================================\n");
        scanf("%c",&Item);
        switch(Item)
        {
        case 'A':
        case 'a':
            USB_PortInit(u32UsbhPort1, HOST_LIKE_PORT2_0);
            goto start;
        case 'B':
        case 'b':
            USB_PortInit(u32UsbhPort1, HOST_LIKE_PORT2_DISABLE);
            goto start;
        }
    }
    while(1);

start:

    USBH_Open();

    USBH_HidInit();

    UAC_Init();

    printf("Wait until any Audio Class devices connected...\n");
    while (1)
    {
        USBH_ProcessHubEvents();             /* USB Host port detect polling and management */

        uac_dev = UAC_GetDeviceList();
        if (uac_dev != NULL)
            break;
    }

    hdev = USBH_HidGetDeviceList();
    if (hdev == NULL)
    {
        printf("HID device not found!\n");
    }
    else
    {
        printf("\nUSBH_HidStartIntReadPipe...\n");
        if (USBH_HidStartIntReadPipe(hdev, int_read_callback) == HID_RET_OK)
        {
            printf("Interrupt in transfer started...\n");
        }
    }

    uac_control_example(uac_dev);

    if(uac_dev->au_in_ifnum != -1)
    {
        if (UAC_InstallIsoInCbFun(uac_dev, au_in_buff, 2048, audio_in_callback) != UAC_RET_OK)
        {
            printf("Failed to install audio-in callback function!\n");
            goto err_out;
        }
    }
    if(uac_dev->au_out_ifnum != -1)
    {
        if (UAC_InstallIsoOutCbFun(uac_dev, audio_out_callback) != UAC_RET_OK)
        {
            printf("Failed to install audio-out callback function!\n");
            goto err_out;
        }
    }
    while (1)
    {
        au_in_cnt = 0;
        au_out_cnt = 0;

        if(uac_dev->au_out_ifnum != -1)
        {
            printf("\nStart audio output stream...\n");
            UAC_StartIsoOutPipe(uac_dev);
        }
        if(uac_dev->au_in_ifnum != -1)
        {
            printf("\nStart audio input stream...\n");
            UAC_StartIsoInPipe(uac_dev);

            while (au_in_cnt < 64*1024);

            UAC_StopIsoInPipe(uac_dev);
            printf("64 KB bytes audio data received.\n");
            printf("Audio input stream stopped.\n");
        }
        if(uac_dev->au_out_ifnum != -1)
        {
            while (au_out_cnt < 64*1024) ;

            UAC_StopIsoOutPipe(uac_dev);
            printf("64 KB bytes audio data send.\n");
            printf("Audio output stream stopped.\n");
        }
        getchar();
    }

err_out:
    printf("\nFailed!\n");
    while (1);
}


/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/
