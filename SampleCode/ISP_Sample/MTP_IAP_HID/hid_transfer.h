/******************************************************************************
 * @file     usbd_hid.h
 * @brief    NUC505 USB driver header file
 * @version  2.0.0
 * @date     22, Sep, 2014
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __USBD_HID_H__
#define __USBD_HID_H__

/* Define the vendor id and product id */
#define USBD_VID                0x0416
#define USBD_PID                0x3F00

/*!<Define HID Class Specific Request */
#define GET_REPORT              0x01
#define GET_IDLE                0x02
#define GET_PROTOCOL            0x03
#define SET_REPORT              0x09
#define SET_IDLE                0x0A
#define SET_PROTOCOL            0x0B

/*!<USB HID Interface Class protocol */
#define HID_NONE                0x00
#define HID_KEYBOARD            0x01
#define HID_MOUSE               0x02

/*!<USB HID Class Report Type */
#define HID_RPT_TYPE_INPUT      0x01
#define HID_RPT_TYPE_OUTPUT     0x02
#define HID_RPT_TYPE_FEATURE    0x03

/*-------------------------------------------------------------*/
/* Define EP maximum packet size */
#define CEP_MAX_PKT_SIZE        64
#define CEP_OTHER_MAX_PKT_SIZE  64
#define EPA_MAX_PKT_SIZE        64/*512*/
#define EPA_OTHER_MAX_PKT_SIZE  64
#define EPB_MAX_PKT_SIZE        64/*512*/
#define EPB_OTHER_MAX_PKT_SIZE  64

#define CEP_BUF_BASE    0
#define CEP_BUF_LEN     CEP_MAX_PKT_SIZE
#define EPA_BUF_BASE    (CEP_BUF_BASE + CEP_MAX_PKT_SIZE)
#define EPA_BUF_LEN     EPA_MAX_PKT_SIZE
#define EPB_BUF_BASE    (EPA_BUF_BASE + EPA_BUF_LEN)
#define EPB_BUF_LEN     EPB_MAX_PKT_SIZE

/* Define the interrupt In EP number */
#define INT_IN_EP_NUM   0x01
#define INT_OUT_EP_NUM  0x02

/* Define Descriptor information */
#define HID_DEFAULT_INT_IN_INTERVAL     4
#define USBD_SELF_POWERED               0
#define USBD_REMOTE_WAKEUP              0
#define USBD_MAX_POWER                  50  /* The unit is in 2mA. ex: 50 * 2mA = 100mA */

/*-------------------------------------------------------------*/

extern uint8_t  usb_rcvbuf[];
extern uint8_t  usb_sendbuf[];
extern volatile uint8_t bUsbDataReady;

/*-------------------------------------------------------------*/
void HID_Init(void);
void HID_InitForHighSpeed(void);
void HID_InitForFullSpeed(void);
void HID_ClassRequest(void);
void HID_VendorRequest(void);

void EPA_Handler(void);
void EPB_Handler(void);
void HID_SetInReport(void);
void HID_GetOutReport(uint8_t *pu8EpBuf, uint32_t u32Size);
void HID_ActiveDMA(uint32_t u32In, uint32_t u32Addr, uint32_t u32Len);

#endif  /* __USBD_HID_H_ */

/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/
