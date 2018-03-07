/******************************************************************************
 * @file     descriptors.c
 * @brief    NuMicro series USBD driver source file
 * @date     2017/04/26 09:30 a.m.
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
/*!<Includes */
#include "NUC505Series.h"
#include "AudioLib.h"
#include "usbd_audio_10.h"

/*
  *Microphone - Interface alternate 1~6
  +-----------+------------+----------------+------------------+
  | Alternate | Channel(s) | Bit Resolution | Sampling Rate(s) |
  +-----------+------------+----------------+------------------+
  |     1     |      1     |     16 bits    |  44.1kHz, 48kHz  |
  +-----------+------------+----------------+------------------+
  |     2     |      1     |     16 bits    |      96kHz       |
  +-----------+------------+----------------+------------------+
  |     3     |      2     |     16 bits    |  44.1kHz, 48kHz  |
  +-----------+------------+----------------+------------------+
  |     4     |      2     |     24 bits    |  44.1kHz, 48kHz  |
  +-----------+------------+----------------+------------------+
  |     5     |      2     |     16 bits    |      96kHz       |
  +-----------+------------+----------------+------------------+
  |     6     |      2     |     24 bits    |      96kHz       |
  +-----------+------------+----------------+------------------+
	
  Note:
  1.If you want to remove certain alternate for Microphone interface, please remove the group of 
    the alternate descriptors (Standard AS interface, Audio Streaming Class Specific Interface Descriptor, 
    Audio Streaming Format Type Descriptor, Endpoint Descriptor, Audio Streaming Class Specific Audio Data 
    Endpoint Descriptor) and modify the Total Length field of Configuration Descriptor.
    For example,
      Remove  Microphone Interface alternate 6 (24 bit resolution) from Microphone descriptor with HID  
		
      1.Remove the following descriptors for Microphone - Interface alternate 6
         Microphone - Interface alternate 6
           Standard AS interface                                         (0x09)													 
           Audio Streaming Class Specific Interface Descriptor           (0x07)	
           Audio Streaming Format Type Descriptor                        (0x0B) 
           Endpoint Descriptor                                           (0x07)	
           Audio Streaming Class Specific Audio Data Endpoint Descriptor (0x07)	
           *Interface alternate Summary                                  (0x29)                                                        
                         
      2.Modify the Total Length field of Configuration Descriptor to 0x130
         
         0x159(Original Total Length) - 0x29(Total Length of descriptors for Microphone - Interface alternate 6) = 0x130			

      3.Modify the change for interface alternate in UAC_SetInterface_10.c
				
  2.If you want to add / remove sampling rate to certain alternate for Microphone interface,
    please modify Audio Streaming Format Type Descriptor (bLength, bSamFreqType, tSamFreq fields)
    and the Total Length field of Configuration Descriptor.
    For example,
      Remove 48kHz from Microphone Interface alternate 3 (16 bit resolution) from Microphone descriptor with HID 	
	
      1.Modify Audio Streaming Format Type Descriptor for Microphone Interface alternate 3

         Audio Streaming Format Type Descriptor
         +--------------------+------------------+-------------------+	
         | *bLength           |  Original Value  |   Modified Value  |			
         +--------------------+------------------+-------------------+	
         | bLength            |        0x0E      |   0x0B(-3 Bytes)  |			
         +--------------------+------------------+-------------------+					 
         | bDescriptorType    |        0x24      |        0x24       |   				
         +--------------------+------------------+-------------------+	
         | bDescriptorSubType |        0x02      |        0x02       |	
         +--------------------+------------------+-------------------+	
         | bFormatType        |        0x01      |        0x01       |		
         +--------------------+------------------+-------------------+	
         | bNrChannels        |        0x02      |        0x02       | 	
         +--------------------+------------------+-------------------+	
         | bSubFrameSize      |        0x02      |        0x02       |  			
         +--------------------+------------------+-------------------+		
         | bBitResolution     |        0x10      |        0x10       | 		
         +--------------------+------------------+-------------------+	
         | *bSamFreqType      |        0x02      | 0x01(-1 Frequency)| 			
         +--------------------+------------------+-------------------+	
         | *tSamFreq          | REC_RATE_441K_LO |  REC_RATE_441K_LO |
         |                    | REC_RATE_441K_MD |  REC_RATE_441K_MD |
         |                    | REC_RATE_441K_HI |  REC_RATE_441K_MD |
         |                    |  REC_RATE_48K_LO |                   |
         |                    |  REC_RATE_48K_MD |                   |									
         |                    |  REC_RATE_48K_HI |                   |
         +--------------------+------------------+-------------------+	
                         
      2.Modify the Total Length field of Configuration Descriptor to 0x156.

         0x159(Original Total Length) - 0x03(The decrease Length of Audio Streaming Format Type Descriptor) = 0x156	
		
  4.If you want to change the support function of audio control, please modify the bmaControls field of
    Audio Control Feature Unit Descriptor for Microphone
      A bit set to 1 indicates that the mentioned Control is supported
         0:
         D0: Mute
         D1: Volume
         D2: Bass
         D3: Mid
         D4: Treble
         D5: Graphic Equalizer
         D6: Automatic Gain
         D7: Delay
         D8: Bass Boost
         D9: Loudness
         D10..(n*8-1): Reserved		
	5.If you want to change the polling interal of HID Endpoint, please modify the bInterval field of Endpoint Descriptor for HID.
*/

/*----------------------------------------------------------------------------*/
/*!<USB Device Descriptor */
#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t gu8DeviceDescriptor_10[] = {
#else
__align(4) uint8_t gu8DeviceDescriptor_10[] = {
#endif
    LEN_DEVICE,         /* bLength */
    DESC_DEVICE,        /* bDescriptorType */
    0x10, 0x01,         /* bcdUSB */
    0x00,               /* bDeviceClass */
    0x00,               /* bDeviceSubClass */
    0x00,               /* bDeviceProtocol */
    CEP_MAX_PKT_SIZE,   /* bMaxPacketSize0 */
    /* idVendor */
    USBD_VID & 0x00FF,
    (USBD_VID & 0xFF00) >> 8,
    /* idProduct */
    USBD_PID & 0x00FF,
    (USBD_PID & 0xFF00) >> 8,
    0x00, 0x01,         /* bcdDevice */
    0x01,               /* iManufacture */
    0x02,               /* iProduct */
    0x03,               /* iSerialNumber - no serial */
    0x01                /* bNumConfigurations */
};

#ifdef __HID__
#ifdef __KEYBOARD__
#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t gu8KeyboardReportDesc_10[] =
#else
__align(4) uint8_t gu8KeyboardReportDesc_10[] =
#endif
{
    0x05, 0x01,        /* Usage Page(Generic Desktop Controls) */
    0x09, 0x06,        /* Usage(Keyboard) */
    0xA1, 0x01,        /* Collection(Application) */
    0x05, 0x07,        /* Usage Page(Keyboard/Keypad) */
    0x19, 0xE0,        /* Usage Minimum(0xE0) */
    0x29, 0xE7,        /* Usage Maximum(0xE7) */ 
    0x15, 0x00,        /* Logical Minimum(0x0) */
    0x25, 0x01,        /* Logical Maximum(0x1) */
    0x75, 0x01,        /* Report Size(0x1) */
    0x95, 0x08,        /* Report Count(0x8) */
    0x81, 0x02,        /* Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field) */
    0x95, 0x01,        /* Report Count(0x1) */
    0x75, 0x08,        /* Report Size(0x8) */
    0x81, 0x01,        /* Input(Constant, Array, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field) */
    0x95, 0x05,        /* Report Count(0x5) */
    0x75, 0x01,        /* Report Size(0x1) */
    0x05, 0x08,        /* Usage Page(LEDs) */
    0x19, 0x01,        /* Usage Minimum(0x1) */
    0x29, 0x05,        /* Usage Maximum(0x5) */ 
    0x91, 0x02,        /* Output(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Non VolatileBit Field) */
    0x95, 0x01,        /* Report Count(0x1) */
    0x75, 0x03,        /* Report Size(0x3) */ 
    0x91, 0x01,        /* Output(Constant, Array, Absolute, No Wrap, Linear, Preferred State, No Null Position, Non VolatileBit Field) */
    0x95, 0x06,        /* Report Count(0x6) */
    0x75, 0x08,        /* Report Size(0x8) */ 
    0x15, 0x00,        /* Logical Minimum(0x0) */
    0x25, 0x65,        /* Logical Maximum(0x65) */ 
    0x05, 0x07,        /* Usage Page(Keyboard/Keypad) */
    0x19, 0x00,        /* Usage Minimum(0x0) */
    0x29, 0x65,        /* Usage Maximum(0x65) */
    0x81, 0x00,        /* Input(Data, Array, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field) */
    0xC0               /* End Collection */ 
};

#elif defined __MEDIAKEY__
#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t gu8KeyboardReportDesc_10[] =
#else
__align(4)  uint8_t gu8KeyboardReportDesc_10[] =
#endif
{
    0x05, 0x0C,        /* Usage Page (Consumer) */
    0x09, 0x01,        /* Usage(Consumer Control) */
    0xA1, 0x01,        /* Collection(Application) */
    0x15, 0x00,        /* Logical Minimum(0x0) */
    0x25, 0x01,        /* Logical Maximum(0x1) */
    0x09, 0xE2,        /* Usage(Mute) */
    0x09, 0xE9,        /* Usage(Volume Increment) */
    0x09, 0xEA,        /* Usage(Volume Decrement) */
    0x75, 0x01,        /* Report Size(0x1) */
    0x95, 0x03,        /* Report Count(0x3) */
    0x81, 0x02,        /* Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field) */
    0x75, 0x01,        /* Report Size(0x1) */
    0x95, 0x05,        /* Report Count(0x5) */
    0x81, 0x03,        /* Input(Constant, Variable, Absolute) */
    0x09, 0xB0,        /* Usage(Play) */
    0x09, 0xB7,        /* Usage(Stop) */
    0x09, 0xCD,        /* Usage(Play/Pause) */
    0x09, 0xB5,        /* Usage(Scan Next Track) */
    0x09, 0xB6,        /* Usage(Scan Previous Track) */
    0x09, 0xB2,        /* Usage(Record) */
    0x09, 0xB4,        /* Usage(Rewind) */
    0x09, 0xB3,        /* Usage(Fast Forward) */
    0x75, 0x01,        /* Report Size(0x1) */
    0x95, 0x08,        /* Report Count(0x8) */
    0x81, 0x02,        /* Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field) */
    0x09, 0x00,        /* Usage(Undefined) */
    0x75, 0x08,        /* Report Size(0x8) */
    0x95, 0x06,        /* Report Count(0x6) */
    0x81, 0x02,        /* Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field) */
    0x09, 0x00,        /* Usage(Undefined) */
    0x75, 0x08,        /* Report Size(0x8) */
    0x95, 0x08,        /* Report Count(0x8) */
    0x91, 0x00,        /* Output(Data, Array, Absolute, No Wrap, Linear, Preferred State, No Null Position, Non VolatileBit Field) */
    0xC0
};
#endif
#define HID_KEYBOARD_REPORT_DESC_SIZE_10 \
    sizeof(gu8KeyboardReportDesc_10) / sizeof(gu8KeyboardReportDesc_10[0])
const uint32_t gu32KeyboardReportDescSize_10 = HID_KEYBOARD_REPORT_DESC_SIZE_10;

#define HID_REPORT_DESCRIPTOR_SIZE_10   HID_KEYBOARD_REPORT_DESC_SIZE_10

#endif

/*!<USB Configure Descriptor */
#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t gu8ConfigDescriptor_10[] = {
#else
__align(4) uint8_t gu8ConfigDescriptor_10[] = {
#endif
    /* Configuration Descriptor */
    LEN_CONFIG,         /* bLength */
    DESC_CONFIG,        /* bDescriptorType */
#ifdef __HID__
    0x59, 0x01,         /* wTotalLength
                           Descriptor without HID                     (0x140)
                           HID Descriptor 												 
                             Interface Descriptor                      (0x09)
                             HID Descriptor                            (0x09)
                             Endpoint Descriptor                       (0x07)
														
                           0x140 + 0x09 + 0x09 + 0x07 = 0x159	
                        */		
    0x03,               /* bNumInterfaces - Interface 0, Interface 1 (Microphone), Interface 2 (HID) */
#else
    0x40, 0x01,         /* wTotalLength */
                        /* 
                           Configuration Descriptor                    (0x09)	
                           Interface Descriptor (Audio Class)          (0x09)
                           Audio Control Interface Header Descriptor   (0x09)		
                           Microphone - Audio Control   	 											
                             Audio Control Input Terminal Descriptor   (0x0C)
                             Audio Control Feature Unit Descriptor     (0x08)
                             Audio Control Output Terminal Descriptor  (0x09)                          
                           Microphone - Interface alternate 0
                             Standard AS interface                     (0x09)
                           Microphone - Interface alternate 1~6
                             Standard AS interface                                         (0x09,0x09,0x09,0x09,0x09,0x09)													 
                             Audio Streaming Class Specific Interface Descriptor           (0x07,0x07,0x07,0x07,0x07,0x07)	
                             Audio Streaming Format Type Descriptor                        (0x0E,0x0B,0x0E,0x0E,0x0B,0x0B) 
                             Endpoint Descriptor                                           (0x07,0x07,0x07,0x07,0x07,0x07)	
                             Audio Streaming Class Specific Audio Data Endpoint Descriptor (0x07,0x07,0x07,0x07,0x07,0x07)		
                             *Each Interface alternate Summary                             (0x2C,0x29,0x2C,0x2C,0x29,0x29)       											 
													 										 
                           0x09 + 0x09 + 0x9 + (0x0C + 0x08 + 0x09) +
                           0x09 + 0x2C + 0x29 + 0x2C + 0x2C + 0x29 + 0x29 = 0x140 
                        */		
    0x02,               /* bNumInterfaces - Interface 0, Interface 1 (Microphone) */
#endif	
    0x01,               /* bConfigurationValue */
    0x00,               /* iConfiguration */
    0x80,               /* bmAttributes */
    0x20,               /* Max power */

    /* Interface Descriptor (Audio Class) */
    0x09,               /* bLength */
    0x04,               /* bDescriptorType */
    0x00,               /* bInterfaceNumber */
    0x00,               /* bAlternateSetting */
    0x00,               /* bNumEndpoints */
    0x01,               /* bInterfaceClass:AUDIO */
    0x01,               /* bInterfaceSubClass:AUDIOCONTROL */
    0x00,               /* bInterfaceProtocol */
    0x00,               /* iInterface */
		
    /* Audio Control Interface Header Descriptor */
    0x09,               /* bLength */		
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x01,               /* bDescriptorSubType:HEADER */
    0x00, 0x01,         /* bcdADC:1.0 */
    0x26, 0x00,         /* wTotalLength 
                           Audio Control Interface Header Descriptor   (0x09)
                           Microphone - Audio Control   												
                             Audio Control Input Terminal Descriptor   (0x0C)
                             Audio Control Feature Unit Descriptor     (0x08)
                             Audio Control Output Terminal Descriptor  (0x09)                       
                           
                             0x09 + (0x0C + 0x08 + 0x09) = 0x26												
                        */		
    0x01,               /* bInCollection */
    0x01,               /* baInterfaceNr(1) - Microphone */	

    /* Audio Control Input Terminal Descriptor (Terminal ID 4) */
    0x0C,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x02,               /* bDescriptorSubType:INPUT_TERMINAL*/
    0x04,               /* bTerminalID*/
    0x01,0x02,          /* wTerminalType: 0x0201 microphone*/
    0x00,               /* bAssocTerminal*/
    0x02,               /* bNrChannels : a number that specifies how many logical audio channels are present in the cluster */
    0x03, 0x00,         /* wChannelConfig: a bit field that indicates which spatial locations are present in the cluster. 
                           The bit allocations are as follows:
                             D0: Left Front (L)
                             D1: Right Front (R)
                             D2: Center Front (C)
                             D3: Low Frequency Enhancement (LFE)
                             D4: Left Surround (LS)
                             D5: Right Surround (RS)
                             D6: Left of Center (LC)
                             D7: Right of Center (RC)
                             D8: Surround (S)
                             D9: Side Left (SL)
                             D10: Side Right (SR)
                             D11: Top (T)
                             D15..12: Reserved		
                        */	
    0x00,               /* iChannelNames*/
    0x00,               /* iTerminal*/

    /* Audio Control Feature Unit Descriptor - Microphone (UNIT ID 5 - Source 4) */
    0x08,               /* bLength */
    0x24,               /* bDescriptorType */
    0x06,               /* bDescriptorSubType */
    REC_FEATURE_UNITID, /* bUnitID */
    0x04,               /* bSourceID */
    0x01,               /* bControlSize - Size, in bytes, of the bmControls field: n */
    0x03,               /* bmaControls(0) */
                        /* A bit set to 1 indicates that the mentioned
                           Control is supported for master channel
                           0:
                           D0: Mute
                           D1: Volume
                           D2: Bass
                           D3: Mid
                           D4: Treble
                           D5: Graphic Equalizer
                           D6: Automatic Gain
                           D7: Delay
                           D8: Bass Boost
                           D9: Loudness
                           D10..(n*8-1): Reserved
                        */		
    0x00,               /* iFeature */
		
    /* Audio Control Output Terminal Descriptor - Microphone (Terminal ID 2 - Source ID 5) */
    0x09,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x03,               /* bDescriptorSubType:OUTPUT_TERMINAL */
    0x02,               /* bTerminalID */
    0x01,0x01,          /* wTerminalType: 0x0101 usb streaming */
    0x00,               /* bAssocTerminal */
    REC_FEATURE_UNITID, /* bSourceID */
    0x00,               /* iTerminal */

    /* Interface Descriptor - Interface 1, alternate 0 */
    0x09,               /* bLength */
    0x04,               /* bDescriptorType */
    0x01,               /* bInterfaceNumber */
    0x00,               /* bAlternateSetting */
    0x00,               /* bNumEndpoints */
    0x01,               /* bInterfaceClass:AUDIO */
    0x02,               /* bInterfaceSubClass:AUDIOSTREAMING */
    0x00,               /* bInterfaceProtocol */
    0x00,               /* iInterface */

    /* Interface Descriptor - Interface 1, alternate 1 */
    0x09,               /* bLength */
    0x04,               /* bDescriptorType */
    0x01,               /* bInterfaceNumber */
    0x01,               /* bAlternateSetting */
    0x01,               /* bNumEndpoints */
    0x01,               /* bInterfaceClass:AUDIO */
    0x02,               /* bInterfaceSubClass:AUDIOSTREAMING */
    0x00,               /* bInterfaceProtocol */
    0x00,               /* iInterface */

    /* Audio Streaming Class Specific Interface Descriptor (this interface's endpoint connect to Terminal ID 0x02) */
    0x07,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x01,               /* bDescriptorSubType:AS_GENERAL */
    0x02,               /* bTernimalLink */
    0x01,               /* bDelay */
    0x01,0x00,          /* wFormatTag:0x0001 PCM */

    /* Audio Streaming Format Type Descriptor */
    0x0E,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x02,               /* bDescriptorSubType:FORMAT_TYPE */
    0x01,               /* bFormatType:FORMAT_TYPE_I */
                        /* Standard AS interface 1, alternate 1 */ 			
    0x01,               /* bNrChannels    :  1 Channels */
    0x02,               /* bSubFrameSize  :  2 bytes per sample */
    0x10,               /* bBitResolution : 16 bits  per sample */
    0x02,               /* bSamFreqType : 
                           0 Continuous sampling frequency
                           1 The number of discrete sampling frequencies */
    /* bSamFreqType */
    REC_RATE_441K_LO,
    REC_RATE_441K_MD,
    REC_RATE_441K_HI,		
		
    REC_RATE_48K_LO,
    REC_RATE_48K_MD,
    REC_RATE_48K_HI,

    /* Endpoint Descriptor (ISO IN Audio Data Endpoint - alternate 1) */
    0x07,                             /* bLength */
    0x05,                             /* bDescriptorType */
    ISO_IN_EP_NUM | EP_INPUT,         /* bEndpointAddress */
    0x05,                             /* bmAttributes */
    /* wMaxPacketSize note */
    (96 + 24) & 0x00FF,
    ((96 + 24) & 0xFF00) >> 8,
    0x01,                             /* bInterval*/

    /* Audio Streaming Class Specific Audio Data Endpoint Descriptor */
    0x07,               /* bLength */
    0x25,               /* bDescriptorType:CS_ENDPOINT */
    0x01,               /* bDescriptorSubType:EP_GENERAL */
    0x01,               /* bmAttributes, Bit 0: Sampling Frequency */
    0x00,               /* bLockDelayUnits */
    0x00, 0x00,         /* wLockDelay */

    /* Interface Descriptor - Interface 1, alternate 2 */
    0x09,               /* bLength */
    0x04,               /* bDescriptorType */
    0x01,               /* bInterfaceNumber */
    0x02,               /* bAlternateSetting */
    0x01,               /* bNumEndpoints */
    0x01,               /* bInterfaceClass:AUDIO */
    0x02,               /* bInterfaceSubClass:AUDIOSTREAMING */
    0x00,               /* bInterfaceProtocol */
    0x00,               /* iInterface */

    /* Audio Streaming Class Specific Interface Descriptor (this interface's endpoint connect to Terminal ID 0x02 - Microphoe) */
    0x07,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x01,               /* bDescriptorSubType:AS_GENERAL */
    0x02,               /* bTernimalLink */
    0x01,               /* bDelay */
    0x01,0x00,          /* wFormatTag:0x0001 PCM */

    /* Audio Streaming Format Type Descriptor */
    0x0B,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x02,               /* bDescriptorSubType:FORMAT_TYPE */
    0x01,               /* bFormatType:FORMAT_TYPE_I */
                        /* Standard AS interface 1, alternate 2 */ 			
    0x01,               /* bNrChannels    :  1 Channels */
    0x02,               /* bSubFrameSize  :  2 bytes per sample */
    0x10,               /* bBitResolution : 16 bits  per sample */				
    0x01,               /* bSamFreqType : 
                           0 Continuous sampling frequency
                           1 The number of discrete sampling frequencies */
    /* bSamFreqType */
    REC_RATE_96K_LO,
    REC_RATE_96K_MD,
    REC_RATE_96K_HI,

    /* Endpoint Descriptor (ISO IN Audio Data Endpoint - alternate 2) */
    0x07,                             /* bLength */
    0x05,                             /* bDescriptorType */
    ISO_IN_EP_NUM | EP_INPUT,         /* bEndpointAddress */
    0x05,                             /* bmAttributes */
    /* wMaxPacketSize note */
    (192 + 24) & 0x00FF,
    ((192 + 24) & 0xFF00) >> 8,
    0x01,                             /* bInterval*/

    /* Audio Streaming Class Specific Audio Data Endpoint Descriptor */
    0x07,               /* bLength */
    0x25,               /* bDescriptorType:CS_ENDPOINT */
    0x01,               /* bDescriptorSubType:EP_GENERAL */
    0x01,               /* bmAttributes, Bit 0: Sampling Frequency */
    0x00,               /* bLockDelayUnits */
    0x00, 0x00,         /* wLockDelay */

    /* Interface Descriptor - Interface 1, alternate 3 */
    0x09,               /* bLength */
    0x04,               /* bDescriptorType */
    0x01,               /* bInterfaceNumber */
    0x03,               /* bAlternateSetting */
    0x01,               /* bNumEndpoints */
    0x01,               /* bInterfaceClass:AUDIO */
    0x02,               /* bInterfaceSubClass:AUDIOSTREAMING */
    0x00,               /* bInterfaceProtocol */
    0x00,               /* iInterface */

    /* Audio Streaming Class Specific Interface Descriptor (this interface's endpoint connect to Terminal ID 0x02 - Microphoe) */
    0x07,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x01,               /* bDescriptorSubType:AS_GENERAL */
    0x02,               /* bTernimalLink */
    0x01,               /* bDelay */
    0x01,0x00,          /* wFormatTag:0x0001 PCM */

    /* Audio Streaming Format Type Descriptor */
    0x0E,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x02,               /* bDescriptorSubType:FORMAT_TYPE */
    0x01,               /* bFormatType:FORMAT_TYPE_I */
                        /* Standard AS interface 1, alternate 3 */ 					
    0x02,               /* bNrChannels    :  2 Channels */
    0x02,               /* bSubFrameSize  :  2 bytes per sample */
    0x10,               /* bBitResolution : 16 bits  per sample */				
    0x02,               /* bSamFreqType : 
                           0 Continuous sampling frequency
                           1 The number of discrete sampling frequencies */		
    /* bSamFreqType  */
    REC_RATE_441K_LO,
    REC_RATE_441K_MD,
    REC_RATE_441K_HI,
		
    REC_RATE_48K_LO,
    REC_RATE_48K_MD,
    REC_RATE_48K_HI,

    /* Endpoint Descriptor (ISO IN Audio Data Endpoint - alternate 3) */
    0x07,                             /* bLength */
    0x05,                             /* bDescriptorType */
    ISO_IN_EP_NUM | EP_INPUT,         /* bEndpointAddress */
    0x05,                             /* bmAttributes */
    /* wMaxPacketSize note */
    (192 + 24) & 0x00FF,
    ((192 + 24) & 0xFF00) >> 8,
    0x01,                             /* bInterval*/

    /* Audio Streaming Class Specific Audio Data Endpoint Descriptor */
    0x07,               /* bLength */
    0x25,               /* bDescriptorType:CS_ENDPOINT */
    0x01,               /* bDescriptorSubType:EP_GENERAL */
    0x01,               /* bmAttributes, Bit 0: Sampling Frequency */
    0x00,               /* bLockDelayUnits */
    0x00, 0x00,         /* wLockDelay */
		
    /* Interface Descriptor - Interface 1, alternate 4 */
    0x09,               /* bLength */
    0x04,               /* bDescriptorType */
    0x01,               /* bInterfaceNumber */
    0x04,               /* bAlternateSetting */
    0x01,               /* bNumEndpoints */
    0x01,               /* bInterfaceClass:AUDIO */
    0x02,               /* bInterfaceSubClass:AUDIOSTREAMING */
    0x00,               /* bInterfaceProtocol */
    0x00,               /* iInterface */

    /* Audio Streaming Class Specific Interface Descriptor (this interface's endpoint connect to Terminal ID 0x02 - Microphoe) */
    0x07,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x01,               /* bDescriptorSubType:AS_GENERAL */
    0x02,               /* bTernimalLink */
    0x01,               /* bDelay */
    0x01,0x00,          /* wFormatTag:0x0001 PCM */

    /* Audio Streaming Format Type Descriptor */
    0x0E,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x02,               /* bDescriptorSubType:FORMAT_TYPE */
    0x01,               /* bFormatType:FORMAT_TYPE_I */
                        /* Standard AS interface 1, alternate 4 */ 					
    0x02,               /* bNrChannels    :  2 Channels */
    0x03,               /* bSubFrameSize  :  3 bytes per sample */
    0x18,               /* bBitResolution : 24 bits  per sample */					
    0x02,               /* bSamFreqType : 
                           0 Continuous sampling frequency
                           1 The number of discrete sampling frequencies */		
    /* bSamFreqType  */
    REC_RATE_441K_LO,
    REC_RATE_441K_MD,
    REC_RATE_441K_HI,
   
    REC_RATE_48K_LO,
    REC_RATE_48K_MD,
    REC_RATE_48K_HI,

    /* Endpoint Descriptor (ISO IN Audio Data Endpoint - alternate 4) */
    0x07,                             /* bLength */
    0x05,                             /* bDescriptorType */
    ISO_IN_EP_NUM | EP_INPUT,         /* bEndpointAddress */
    0x05,                             /* bmAttributes */
    /* wMaxPacketSize note */
    (288 + 24) & 0x00FF,
    ((288 + 24) & 0xFF00) >> 8,
    0x01,                             /* bInterval*/

    /* Audio Streaming Class Specific Audio Data Endpoint Descriptor */
    0x07,               /* bLength */
    0x25,               /* bDescriptorType:CS_ENDPOINT */
    0x01,               /* bDescriptorSubType:EP_GENERAL */
    0x01,               /* bmAttributes, Bit 0: Sampling Frequency */
    0x00,               /* bLockDelayUnits */
    0x00, 0x00,         /* wLockDelay */

    /* Interface Descriptor - Interface 1, alternate 5 */
    0x09,               /* bLength */
    0x04,               /* bDescriptorType */
    0x01,               /* bInterfaceNumber */
    0x05,               /* bAlternateSetting */
    0x01,               /* bNumEndpoints */
    0x01,               /* bInterfaceClass:AUDIO */
    0x02,               /* bInterfaceSubClass:AUDIOSTREAMING */
    0x00,               /* bInterfaceProtocol */
    0x00,               /* iInterface */

    /* Audio Streaming Class Specific Interface Descriptor (this interface's endpoint connect to Terminal ID 0x02 - Microphoe) */
    0x07,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x01,               /* bDescriptorSubType:AS_GENERAL */
    0x02,               /* bTernimalLink */
    0x01,               /* bDelay */
    0x01,0x00,          /* wFormatTag:0x0001 PCM */

    /* Audio Streaming Format Type Descriptor */
    0x0B,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x02,               /* bDescriptorSubType:FORMAT_TYPE */
    0x01,               /* bFormatType:FORMAT_TYPE_I */
                        /* Standard AS interface 1, alternate 5 */ 		
    0x02,               /* bNrChannels    :  2 Channels */
    0x02,               /* bSubFrameSize  :  2 bytes per sample */
    0x10,               /* bBitResolution : 16 bits  per sample */	
    0x01,               /* bSamFreqType : 
                           0 Continuous sampling frequency
                           1 The number of discrete sampling frequencies */		
    /* bSamFreqType  */
    REC_RATE_96K_LO,
    REC_RATE_96K_MD,
    REC_RATE_96K_HI,

    /* Endpoint Descriptor (ISO IN Audio Data Endpoint - alternate 5) */
    0x07,                             /* bLength */
    0x05,                             /* bDescriptorType */
    ISO_IN_EP_NUM | EP_INPUT,         /* bEndpointAddress */
    0x05,                             /* bmAttributes */
    /* wMaxPacketSize note */
    (384 + 24) & 0x00FF,
    ((384 + 24) & 0xFF00) >> 8,
    0x01,                             /* bInterval*/

    /* Audio Streaming Class Specific Audio Data Endpoint Descriptor */
    0x07,               /* bLength */
    0x25,               /* bDescriptorType:CS_ENDPOINT */
    0x01,               /* bDescriptorSubType:EP_GENERAL */
    0x01,               /* bmAttributes, Bit 0: Sampling Frequency */
    0x00,               /* bLockDelayUnits */
    0x00, 0x00,         /* wLockDelay */

    /* Interface Descriptor - Interface 1, alternate 6 */
    0x09,               /* bLength */
    0x04,               /* bDescriptorType */
    0x01,               /* bInterfaceNumber */
    0x06,               /* bAlternateSetting */
    0x01,               /* bNumEndpoints */
    0x01,               /* bInterfaceClass:AUDIO */
    0x02,               /* bInterfaceSubClass:AUDIOSTREAMING */
    0x00,               /* bInterfaceProtocol */
    0x00,               /* iInterface */

    /* Audio Streaming Class Specific Interface Descriptor (this interface's endpoint connect to Terminal ID 0x02 - Microphoe) */
    0x07,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x01,               /* bDescriptorSubType:AS_GENERAL */
    0x02,               /* bTernimalLink */
    0x01,               /* bDelay */
    0x01,0x00,          /* wFormatTag:0x0001 PCM */

    /* Audio Streaming Format Type Descriptor */
    0x0B,               /* bLength */
    0x24,               /* bDescriptorType:CS_INTERFACE */
    0x02,               /* bDescriptorSubType:FORMAT_TYPE */
    0x01,               /* bFormatType:FORMAT_TYPE_I */
                        /* Standard AS interface 1, alternate 6 */ 			
    0x02,               /* bNrChannels    :  2 Channels */
    0x03,               /* bSubFrameSize  :  3 bytes per sample */
    0x18,               /* bBitResolution : 24 bits  per sample */			
    0x01,               /* bSamFreqType : 
                           0 Continuous sampling frequency
                           1 The number of discrete sampling frequencies */		
    /* bSamFreqType  */
    REC_RATE_96K_LO,
    REC_RATE_96K_MD,
    REC_RATE_96K_HI,

    /* Endpoint Descriptor (ISO IN Audio Data Endpoint - alternate 6) */
    0x07,                             /* bLength */
    0x05,                             /* bDescriptorType */
    ISO_IN_EP_NUM | EP_INPUT,         /* bEndpointAddress */
    0x05,                             /* bmAttributes */
    /* wMaxPacketSize note */
    (576 + 24) & 0x00FF,
    ((576 + 24) & 0xFF00) >> 8,
    0x01,                             /* bInterval*/

    /* Audio Streaming Class Specific Audio Data Endpoint Descriptor */
    0x07,               /* bLength */
    0x25,               /* bDescriptorType:CS_ENDPOINT */
    0x01,               /* bDescriptorSubType:EP_GENERAL */
    0x01,               /* bmAttributes, Bit 0: Sampling Frequency */
    0x00,               /* bLockDelayUnits */
    0x00, 0x00,         /* wLockDelay */

#ifdef __HID__
    /* Interface Descriptor for HID */
    LEN_INTERFACE,      /* bLength */
    DESC_INTERFACE,     /* bDescriptorType */
    0x03,               /* bInterfaceNumber */
    0x00,               /* bAlternateSetting */
    0x01,               /* bNumEndpoints */
    0x03,               /* bInterfaceClass */
    0x01,               /* bInterfaceSubClass */
    0x01,               /* bInterfaceProtocol */
    0x00,               /* iInterface */

    /* HID Descriptor */
    LEN_HID,            /* Size of this descriptor in UINT8s */
    DESC_HID,           /* HID descriptor type. */
    0x10, 0x01,         /* HID Class Spec. release number.*/
    0x00,               /* H/W target country. */
    0x01,               /* Number of HID class descriptors to follow. */
    DESC_HID_RPT,       /* Dscriptor type. */

    /* Total length of report descriptor */
    HID_REPORT_DESCRIPTOR_SIZE_10 & 0x00FF,
    (HID_REPORT_DESCRIPTOR_SIZE_10 & 0xFF00) >> 8,

    /* Endpoint Descriptor (Interrupt IN Endpoint) */
    LEN_ENDPOINT,                     /* bLength */
    DESC_ENDPOINT,                    /* bDescriptorType */
    (HID_IN_EP_NUM | EP_INPUT),       /* bEndpointAddress */
    EP_INT,                           /* bmAttributes */   
    /* wMaxPacketSize */ 		
    EPC_MAX_PKT_SIZE & 0x00FF,        
    (EPC_MAX_PKT_SIZE & 0xFF00) >> 8,
    10                                /* bInterval */
#endif
};


/*!<USB Qualifier Descriptor */
#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t gu8QualifierDescriptor_10[] = {
#else
__align(4) uint8_t gu8QualifierDescriptor_10[] = {
#endif
    LEN_QUALIFIER,    /* bLength */
    DESC_QUALIFIER,   /* bDescriptorType */
    0x00, 0x02,       /* bcdUSB */
    0x00,             /* bDeviceClass */
    0x00,             /* bDeviceSubClass */
    0x00,             /* bDeviceProtocol */
    CEP_MAX_PKT_SIZE, /* bMaxPacketSize0 */
    0x01,             /* bNumConfigurations */
    0x00
};

/*!<USB Language String Descriptor */
#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t gu8StringLang_10[4] = {
#else
__align(4) uint8_t gu8StringLang_10[4] = {
#endif
    4,              /* bLength */
    DESC_STRING,    /* bDescriptorType */
    0x09, 0x04
};

/*!<USB Vendor String Descriptor */
#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t gu8VendorStringDesc_10[59] = {
#else
__align(4) uint8_t gu8VendorStringDesc_10[59] = {
#endif
    16,
    DESC_STRING,
    'N', 0, 'u', 0, 'v', 0, 'o', 0, 't', 0, 'o', 0, 'n', 0
};

/*!<USB Product String Descriptor */
#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t gu8ProductStringDesc_10[] = {
#else
__align(4) uint8_t gu8ProductStringDesc_10[] = {
#endif	
#ifdef __HID__
    68,
    DESC_STRING,
    'U', 0, 'A', 0, 'C', 0, ' ', 0, '1', 0, '.', 0, '0', 0, ' ', 0,
    'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'p', 0, 'h', 0, 'o', 0, 'n', 0, 'e', 0,	
    ' ', 0, '&', 0, ' ', 0, 'H', 0, 'I', 0, 'D', 0,
#ifdef __KEYBOARD__
    '-', 0, 'K', 0, 'e', 0, 'y', 0, 'b', 0, 'o', 0, 'a', 0, 'r', 0, 'd', 0,
#else
    '-', 0, 'M', 0, 'e', 0, 'd', 0, 'i', 0, 'a', 0, 'k', 0, 'e', 0, 'y', 0,
#endif			
#else
    38,
    DESC_STRING,
    'U', 0, 'A', 0, 'C', 0, ' ', 0, '1', 0, '.', 0, '0', 0, ' ', 0,
    'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'p', 0, 'h', 0, 'o', 0, 'n', 0, 'e', 0
#endif	
};
/*!<USB Serial String Descriptor */
#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t gu8StringSerial_10[] =
#else
__align(4) uint8_t gu8StringSerial_10[] =
#endif
{
    26,             // bLength
    DESC_STRING,    // bDescriptorType
    'A', 0, '0', 0, '2', 0, '0', 0, '1', 0, '6', 0, '0', 0, '8', 0, '9', 0, '0', 0, '0', 0, '0', 0
};
uint8_t *gpu8UsbString_10[4] = {
    gu8StringLang_10,
    gu8VendorStringDesc_10,
    gu8ProductStringDesc_10,
    gu8StringSerial_10,
};

uint8_t *gu8UsbHidReport_10[4] = {
    NULL,
    NULL,
    NULL,
#ifdef __HID__
    gu8KeyboardReportDesc_10
#else
    NULL
#endif
};

uint32_t gu32UsbHidReportLen_10[4] = {
    0,
    0,
    0,
#ifdef __HID__
    sizeof(gu8KeyboardReportDesc_10),
#else
		0
#endif
};

S_USBD_INFO_T gsInfo_10 = {
    gu8DeviceDescriptor_10,      /*!< Device descriptor */
    gu8ConfigDescriptor_10,      /*!< Config descriptor */
    gpu8UsbString_10,            /*!< Pointer for USB String Descriptor pointers */
    gu8QualifierDescriptor_10,   /*!< Qualifier descriptor */
    gu8ConfigDescriptor_10,      /*!< Full Speed Config descriptor */
    gu8ConfigDescriptor_10,      /*!< Other Speed Config descriptor for High Speed */
    gu8ConfigDescriptor_10,      /*!< Other Speed Config descriptor for Full Speed*/
    NULL,                        /*!< Pointer for HID CompositeDesc descriptor */
    gu8UsbHidReport_10,          /*!< Pointer for HID Report descriptor */ 
    gu32UsbHidReportLen_10,      /*!< Pointer for HID Report descriptor Size */   
};

