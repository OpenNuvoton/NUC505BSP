/**************************************************************************//**
 * @file        UAC_Init_10.c
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
#include "usbd_audio_10.h"

/*--------------------------------------------------------------------------*/
/**
 * @brief       UAC Class Initial
 *
 * @param[in]   None
 *
 * @return      None
 *
 * @details     This function is used to configure endpoints for UAC class
 */
void UAC_Init_10(S_AUDIO_LIB* psAudioLib)
{
    /* Configure USB controller */
    USBD->OPER = 0; /* Full Speed */
#ifdef __HID__
    /* Enable USB BUS, CEP and EPA , EPB , EPC global interrupt */
    USBD_ENABLE_USB_INT(USBD_GINTEN_USBIEN_Msk|USBD_GINTEN_CEPIEN_Msk|USBD_GINTEN_EPAIEN_Msk|USBD_GINTEN_EPBIEN_Msk|USBD_GINTEN_EPCIEN_Msk);
#else
    /* Enable USB BUS, CEP and EPA , EPB global interrupt */
    USBD_ENABLE_USB_INT(USBD_GINTEN_USBIEN_Msk|USBD_GINTEN_CEPIEN_Msk|USBD_GINTEN_EPAIEN_Msk|USBD_GINTEN_EPBIEN_Msk);
#endif
    /* Enable BUS interrupt */
    USBD_ENABLE_BUS_INT(USBD_BUSINTEN_DMADONEIEN_Msk|USBD_BUSINTEN_RESUMEIEN_Msk|USBD_BUSINTEN_RSTIEN_Msk|USBD_BUSINTEN_VBUSDETIEN_Msk);
    /* Reset Address to 0 */
    USBD_SET_ADDR(0);

    /********************/
    /* Control endpoint */
    USBD_SetEpBufAddr(CEP, CEP_BUF_BASE, CEP_BUF_LEN);
    USBD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk|USBD_CEPINTEN_STSDONEIEN_Msk);

    /*********************************************************/
    /* EPA ==> ISO IN endpoint, address 0x01 (ISO_IN_EP_NUM) */
    USBD_SetEpBufAddr(EPA, EPA_BUF_BASE, EPA_BUF_LEN);
    USBD_SET_MAX_PAYLOAD(EPA, (psAudioLib->m_u16RecMaxPayload1_+24));
    USBD_ConfigEp(EPA, ISO_IN_EP_NUM, USB_EP_CFG_TYPE_ISO, USB_EP_CFG_DIR_IN);
    /* EPA Interrupt for Audio Record (Read data from EPB) when Host sends Set Interface for Audio Record */

    /***********************************************************/
    /* EPB ==> ISO OUT endpoint, address 0x02 (ISO_OUT_EP_NUM) */
    USBD_SetEpBufAddr(EPB, EPB_BUF_BASE, EPB_BUF_LEN);
    USBD_SET_MAX_PAYLOAD(EPB, psAudioLib->m_u16PlayMaxPayload2_);
    USBD_ConfigEp(EPB, ISO_OUT_EP_NUM, USB_EP_CFG_TYPE_ISO, USB_EP_CFG_DIR_OUT);
    /* Enable EPB Data Received Interrupt for Audio Play (Read data from EPB Buffer) */
    USBD_ENABLE_EP_INT(EPB, USBD_EPINTEN_RXPKIEN_Msk);
#ifdef __HID__

    /****************************************************************/
    /* EPC ==> Interrupt IN endpoint, address 0x03 (HID_INT_EP_NUM) */
    USBD_SetEpBufAddr(EPC, EPC_BUF_BASE, EPC_BUF_LEN);
    USBD_SET_MAX_PAYLOAD(EPC, EPC_MAX_PKT_SIZE);
    USBD_ConfigEp(EPC, HID_INT_EP_NUM, USB_EP_CFG_TYPE_INT, USB_EP_CFG_DIR_IN);
    /* Enable EPC IN Token Interrupt for HID (Write HID data to EPC) */
    USBD_ENABLE_EP_INT(EPC, USBD_EPINTEN_INTKIEN_Msk);
#endif
}

