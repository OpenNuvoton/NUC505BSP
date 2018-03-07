/**************************************************************************//**
 * @file     descriptors.c
 * @version  V1.00
 * $Date:    17/06/16 11:00a $
 * @brief    NUC505 USBD driver source file
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/*!<Includes */
#include <string.h>
#include "NUC505Series.h"
#include "spiflash_drv.h"
#include "massstorage.h"
#include "define.h"

#define MASS_BUFFER_ADDR        0x20005000
#define STORAGE_BUFFER_ADDR     0x20006000
#define SPI_BUFFER_ICP_MSC      0x20007000
#define MASS_FAT_ADDR           0x20007200

uint32_t volatile g_u32AddressSPI = FIRMWARE_CODE_ADDR;	/* SPI Flash Update Start Address */
extern uint32_t EndTagAddr;

/* Global variables for Control Pipe */
int32_t g_TotalSectors = 4096;
/* USB flow control variables */
uint8_t g_u8BulkState = BULK_NORMAL;
uint8_t g_u8Prevent = 0;
uint8_t volatile g_u8Remove = 0;
uint8_t volatile g_UpdateEnable = 0;
uint8_t volatile g_u8MscOutPacket = 0;
uint8_t g_au8SenseKey[4];
uint32_t *g_file_size;
uint32_t g_u32MSCMaxLun = 0;
uint32_t g_u32LbaAddress;
uint32_t g_u32DataTransferSector;
uint32_t g_u32MassBase, g_u32StorageBase;
uint32_t g_u32EpMaxPacketSize;
uint32_t g_u32CbwSize = 0;
uint8_t volatile g_TimerInit = 0;
uint8_t volatile g_u8UpdateDone = 0;
uint8_t volatile g_u8MACOS = 0, g_u8MACOS_Update = 0;
uint8_t volatile g_u8ShowFile = 0;

/* CBW/CSW variables */
struct CBW g_sCBW;
struct CSW g_sCSW;

uint8_t *pu8FAT = (uint8_t *)MASS_FAT_ADDR; /* 0x80 * 512 */

void UpdateSpiFlash(void);
#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t u8FormatData[512] = 
#else
__align(4) uint8_t u8FormatData[512] = 
#endif
{
    0xEB, 0x3C, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30, 0x00, 0x02, 0x01,
    /* Reserved Sector */
    0x06,    
    0x00,
    /* Byte 16 */
    0x02, 
    /* Byte 17 & 18 */
    0x00, 0x02,
    /* Byte 19 & 20 (number of sector IN THE FILE SYSTEM) */
    0x00, 0x20,
    /* Removable */
    0xF8, 
    /* Size of each FAT */
    0x10, 0x00,
    /* Sector per Track */
    0x01, 0x00,
    /* Number of heads */
    0x01, 0x00,
    /* Hidden sector */
    0x00, 0x00, 0x00, 0x00,
    /* Large sector */	
    0x00, 0x00, 0x00, 0x00,
    /* Physical Disk Number */
    0x00, 
    /* Reserved */
    0x00, 
    /* Sig */
    0x29, 
    0xB9, 0xC1, 0xAA, 0x42, 
    0x4E, 0x4F, 0x20, 0x4E, 0x41, 0x4D, 0x45, 
    0x20, 0x20, 0x20, 0x20, 0x46, 0x41,
    0x54, 0x31, 0x32, 0x20, 0x20, 0x20
};
#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t u8RootDirData[512] = 
#else
__align(4) uint8_t u8RootDirData[512] = 
#endif
{
    0x42, 0x20, 0x00, 0x49, 0x00, 0x6E, 0x00, 0x66,
    0x00, 0x6F, 0x00, 0x0F, 0x00, 0x72, 0x72, 0x00,
    0x6D, 0x00, 0x61, 0x00, 0x74, 0x00, 0x69, 0x00,
    0x6F, 0x00, 0x00, 0x00, 0x6E, 0x00, 0x00, 0x00,
    0x01, 0x53, 0x00, 0x79, 0x00, 0x73, 0x00, 0x74,
    0x00, 0x65, 0x00, 0x0F, 0x00, 0x72, 0x6D, 0x00,
    0x20, 0x00, 0x56, 0x00, 0x6F, 0x00, 0x6C, 0x00,
    0x75, 0x00, 0x00, 0x00, 0x6D, 0x00, 0x65, 0x00,
    0x53, 0x59, 0x53, 0x54, 0x45, 0x4D, 0x7E, 0x31,
    0x20, 0x20, 0x20, 0x16, 0x00, 0x99, 0x0D, 0x5C,
    0x6D, 0x43, 0x6D, 0x43, 0x00, 0x00, 0x0E, 0x5C,
    0x6D, 0x43, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,		
    0x30, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x20, 0x20, 0x20, 		
    0x08, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBA, 0x59,
    0x83, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t g_au8InquiryID[36] = {
    0x00,                   /* Peripheral Device Type */
    0x80,                   /* RMB */
    0x00,                   /* ISO/ECMA, ANSI Version */
    0x00,                   /* Response Data Format */
    0x1F, 0x00, 0x00, 0x00, /* Additional Length */

    /* Vendor Identification */
    'N', 'u', 'v', 'o', 't', 'o', 'n', ' ',

    /* Product Identification */
    'U', 'S', 'B', ' ', 'M', 'a', 's', 's', ' ', 'S', 't', 'o', 'r', 'a', 'g', 'e',

    /* Product Revision */
    '1', '.', '0', '0'
};

static uint8_t g_au8ModePage_01[12] = {
    0x01, 0x0A, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00
};

static uint8_t g_au8ModePage_05[32] = {
    0x05, 0x1E, 0x13, 0x88, 0x08, 0x20, 0x02, 0x00,
    0x01, 0xF4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x05, 0x1E, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x68, 0x00, 0x00
};

static uint8_t g_au8ModePage_1B[12] = {
    0x1B, 0x0A, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

static uint8_t g_au8ModePage_1C[8] = {
    0x1C, 0x06, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00
};

static uint8_t g_au8ModePage[24] = {
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x02, 0x00, 0x1C, 0x0A, 0x80, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

void UpdateSpiFlash(void)
{
    int volatile j;
    /* Write SPI flash */
    if((g_u32AddressSPI <= SPIFLAHS_SIZE) && ((g_u32AddressSPI + 512) <= SPIFLAHS_SIZE))
    {
        if((g_u32AddressSPI % 0x1000) == 0)
            SPIFlash_EraseSector(g_u32AddressSPI);

        for(j=0;j<2;j++)
            SPIFlash_WriteInPageData(g_u32AddressSPI + j * 256, 256, (uint8_t *)(SPI_BUFFER_ICP_MSC + j * 256));	
        g_u32AddressSPI+= 512;		
				
        /* Update Flow is done and set Timer for reset */
        if(((g_u32AddressSPI > (FIRMWARE_CODE_ADDR + *g_file_size)) && g_TimerInit == 0) && (*g_file_size != 0))
        {
            printf("\nFile Size: %dB\n",*g_file_size);

            g_TimerInit = 1;
					
            /* Enable IP clock */
            CLK_EnableModuleClock(TMR0_MODULE);
											
            /* Set timer frequency */
            TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 5);

            /* Enable timer interrupt */
            TIMER_EnableInt(TIMER0);
											
            NVIC_EnableIRQ(TMR0_IRQn);

            /* Start Timer 0 */
            TIMER_Start(TIMER0);				
        }			
    }							
};

/* Timer for Update check */
void TMR0_IRQHandler(void)
{
    /* clear timer interrupt flag */
    TIMER_ClearIntFlag(TIMER0);

    /* To avoid the synchronization issue between system and APB clock domain */
    TIMER_GetIntFlag(TIMER0);

    g_u8UpdateDone = 1;      /* Set Update Flow is done and set g_u8UpdateDone to 1 for reset */
}

void USBD_IRQHandler(void)
{
    __IO uint32_t IrqStL, IrqSt;

    IrqStL = USBD->GINTSTS & USBD->GINTEN;    /* get interrupt status */

    if (!IrqStL)    return;

    /* USB interrupt */
    if (IrqStL & USBD_GINTSTS_USBIF_Msk) {
        IrqSt = USBD->BUSINTSTS & USBD->BUSINTEN;

        if (IrqSt & USBD_BUSINTSTS_SOFIF_Msk)
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_SOFIF_Msk);

        if (IrqSt & USBD_BUSINTSTS_RSTIF_Msk) {
            USBD_SwReset();
            g_u8Remove = 0;
            g_u8BulkState = BULK_CBW;

            USBD_ResetDMA();
            USBD->EP[EPA].EPRSPCTL = USBD_EPRSPCTL_FLUSH_Msk;
            USBD->EP[EPB].EPRSPCTL = USBD_EPRSPCTL_FLUSH_Msk;

            if (USBD->OPER & 0x04)  /* high speed */
                MSC_InitForHighSpeed();
            else                    /* full speed */
                MSC_InitForFullSpeed();
            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk);
            USBD_SET_ADDR(0);
            USBD_ENABLE_BUS_INT(USBD_BUSINTEN_RSTIEN_Msk|USBD_BUSINTEN_RESUMEIEN_Msk|USBD_BUSINTEN_SUSPENDIEN_Msk);
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_RSTIF_Msk);
            USBD_CLR_CEP_INT_FLAG(0x1ffc);
        }

        if (IrqSt & USBD_BUSINTSTS_RESUMEIF_Msk) {
            USBD_ENABLE_BUS_INT(USBD_BUSINTEN_RSTIEN_Msk|USBD_BUSINTEN_SUSPENDIEN_Msk);
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_RESUMEIF_Msk);
        }

        if (IrqSt & USBD_BUSINTSTS_SUSPENDIF_Msk) {
            USBD_ENABLE_BUS_INT(USBD_BUSINTEN_RSTIEN_Msk | USBD_BUSINTEN_RESUMEIEN_Msk);
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_SUSPENDIF_Msk);
        }

        if (IrqSt & USBD_BUSINTSTS_HISPDIF_Msk) {
            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk);
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_HISPDIF_Msk);
        }

        if (IrqSt & USBD_BUSINTSTS_DMADONEIF_Msk) {
            g_usbd_DmaDone = 1;
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_DMADONEIF_Msk);

            if (USBD->DMACTL & USBD_DMACTL_DMARD_Msk) {
                if (g_usbd_ShortPacket == 1) {
                    USBD->EP[EPA].EPRSPCTL = USBD->EP[EPA].EPRSPCTL & 0x10 | USB_EP_RSPCTL_SHORTTXEN;    // packet end
                    g_usbd_ShortPacket = 0;
                }
            }
        }

        if (IrqSt & USBD_BUSINTSTS_PHYCLKVLDIF_Msk)
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_PHYCLKVLDIF_Msk);

        if (IrqSt & USBD_BUSINTSTS_VBUSDETIF_Msk) {
            if (USBD_IS_ATTACHED()) {
                /* USB Plug In */
                USBD_ENABLE_USB();
            } else {
                /* USB Un-plug */
                USBD_DISABLE_USB();
            }
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_VBUSDETIF_Msk);
        }
    }

    if (IrqStL & USBD_GINTSTS_CEPIF_Msk) {
        IrqSt = USBD->CEPINTSTS & USBD->CEPINTEN;

        if (IrqSt & USBD_CEPINTSTS_SETUPTKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_SETUPTKIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_SETUPPKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_SETUPPKIF_Msk);
            USBD_ProcessSetupPacket();
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_OUTTKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_OUTTKIF_Msk);
            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_STSDONEIEN_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_INTKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
            if (!(IrqSt & USBD_CEPINTSTS_STSDONEIF_Msk)) {
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_TXPKIF_Msk);
                USBD_ENABLE_CEP_INT(USBD_CEPINTEN_TXPKIEN_Msk);
                USBD_CtrlIn();
            } else {
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_TXPKIF_Msk);
                USBD_ENABLE_CEP_INT(USBD_CEPINTEN_TXPKIEN_Msk|USBD_CEPINTEN_STSDONEIEN_Msk);
            }
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_PINGIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_PINGIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_TXPKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STSDONEIF_Msk);
            USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
            if (g_usbd_CtrlInSize) {
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
            } else {
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STSDONEIF_Msk);
                USBD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk|USBD_CEPINTEN_STSDONEIEN_Msk);
            }
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_TXPKIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_RXPKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_RXPKIF_Msk);
            USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk|USBD_CEPINTEN_STSDONEIEN_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_NAKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_NAKIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_STALLIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STALLIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_ERRIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_ERRIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_STSDONEIF_Msk) {
            USBD_UpdateDeviceState();
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STSDONEIF_Msk);
            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_BUFFULLIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_BUFFULLIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_BUFEMPTYIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_BUFEMPTYIF_Msk);
            return;
        }
    }

    /* bulk in */
    if (IrqStL & USBD_GINTSTS_EPAIF_Msk) {
        IrqSt = USBD->EP[EPA].EPINTSTS & USBD->EP[EPA].EPINTEN;

        USBD_ENABLE_EP_INT(EPA, 0);
        USBD_CLR_EP_INT_FLAG(EPA, IrqSt);
    }
    /* bulk out */
    if (IrqStL & USBD_GINTSTS_EPBIF_Msk) {
        IrqSt = USBD->EP[EPB].EPINTSTS & USBD->EP[EPB].EPINTEN;
        if (IrqSt & USBD_EPINTSTS_RXPKIF_Msk) {
            g_u8MscOutPacket = 1;
        }

        USBD_CLR_EP_INT_FLAG(EPB, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPCIF_Msk) {
        IrqSt = USBD->EP[EPC].EPINTSTS & USBD->EP[EPC].EPINTEN;
        USBD_CLR_EP_INT_FLAG(EPC, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPDIF_Msk) {
        IrqSt = USBD->EP[EPD].EPINTSTS & USBD->EP[EPD].EPINTEN;
        USBD_CLR_EP_INT_FLAG(EPD, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPEIF_Msk) {
        IrqSt = USBD->EP[EPE].EPINTSTS & USBD->EP[EPE].EPINTEN;
        USBD_CLR_EP_INT_FLAG(EPE, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPFIF_Msk) {
        IrqSt = USBD->EP[EPF].EPINTSTS & USBD->EP[EPF].EPINTEN;
        USBD_CLR_EP_INT_FLAG(EPF, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPGIF_Msk) {
        IrqSt = USBD->EP[EPG].EPINTSTS & USBD->EP[EPG].EPINTEN;
        USBD_CLR_EP_INT_FLAG(EPG, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPHIF_Msk) {
        IrqSt = USBD->EP[EPH].EPINTSTS & USBD->EP[EPH].EPINTEN;
        USBD_CLR_EP_INT_FLAG(EPH, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPIIF_Msk) {
        IrqSt = USBD->EP[EPI].EPINTSTS & USBD->EP[EPI].EPINTEN;
        USBD_CLR_EP_INT_FLAG(EPI, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPJIF_Msk) {
        IrqSt = USBD->EP[EPJ].EPINTSTS & USBD->EP[EPJ].EPINTEN;
        USBD_CLR_EP_INT_FLAG(EPJ, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPKIF_Msk) {
        IrqSt = USBD->EP[EPK].EPINTSTS & USBD->EP[EPK].EPINTEN;
        USBD_CLR_EP_INT_FLAG(EPK, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPLIF_Msk) {
        IrqSt = USBD->EP[EPL].EPINTSTS & USBD->EP[EPL].EPINTEN;
        USBD_CLR_EP_INT_FLAG(EPL, IrqSt);
    }
}

void MSC_InitForHighSpeed(void)
{
    /*****************************************************/
    /* EPA ==> Bulk IN endpoint, address 1 */
    USBD_SetEpBufAddr(EPA, EPA_BUF_BASE, EPA_BUF_LEN);
    USBD_SET_MAX_PAYLOAD(EPA, EPA_MAX_PKT_SIZE);
    USBD_ConfigEp(EPA, BULK_IN_EP_NUM, USB_EP_CFG_TYPE_BULK, USB_EP_CFG_DIR_IN);

    /* EPB ==> Bulk OUT endpoint, address 2 */
    USBD_SetEpBufAddr(EPB, EPB_BUF_BASE, EPB_BUF_LEN);
    USBD_SET_MAX_PAYLOAD(EPB, EPB_MAX_PKT_SIZE);
    USBD_ConfigEp(EPB, BULK_OUT_EP_NUM, USB_EP_CFG_TYPE_BULK, USB_EP_CFG_DIR_OUT);
    USBD_ENABLE_EP_INT(EPB, USBD_EPINTEN_RXPKIEN_Msk);

    g_u32EpMaxPacketSize = EPA_MAX_PKT_SIZE;
}

void MSC_InitForFullSpeed(void)
{
    /*****************************************************/
    /* EPA ==> Bulk IN endpoint, address 1 */
    USBD_SetEpBufAddr(EPA, EPA_BUF_BASE, EPA_BUF_LEN);
    USBD_SET_MAX_PAYLOAD(EPA, EPA_HS_OTHER_MAX_PKT_SIZE);
    USBD_ConfigEp(EPA, BULK_IN_EP_NUM, USB_EP_CFG_TYPE_BULK, USB_EP_CFG_DIR_IN);

    /* EPB ==> Bulk OUT endpoint, address 2 */
    USBD_SetEpBufAddr(EPB, EPB_BUF_BASE, EPB_BUF_LEN);
    USBD_SET_MAX_PAYLOAD(EPB, EPB_HS_OTHER_MAX_PKT_SIZE);
    USBD_ConfigEp(EPB, BULK_OUT_EP_NUM, USB_EP_CFG_TYPE_BULK, USB_EP_CFG_DIR_OUT);
    USBD_ENABLE_EP_INT(EPB, USBD_EPINTEN_RXPKIEN_Msk);

    g_u32EpMaxPacketSize = EPA_HS_OTHER_MAX_PKT_SIZE;
}

void MSC_Init(void)
{
    int i;
    /* Configure USB controller */
    /* Enable USB BUS, CEP and EPA , EPB global interrupt */
    USBD_ENABLE_USB_INT(USBD_GINTEN_USBIEN_Msk|USBD_GINTEN_CEPIEN_Msk|USBD_GINTEN_EPAIEN_Msk|USBD_GINTEN_EPBIEN_Msk);
    /* Enable BUS interrupt */
    USBD_ENABLE_BUS_INT(USBD_BUSINTEN_DMADONEIEN_Msk|USBD_BUSINTEN_RESUMEIEN_Msk|USBD_BUSINTEN_RSTIEN_Msk|USBD_BUSINTEN_VBUSDETIEN_Msk);
    /* Reset Address to 0 */
    USBD_SET_ADDR(0);

    /*****************************************************/
    /* Control endpoint */
    USBD_SetEpBufAddr(CEP, CEP_BUF_BASE, CEP_BUF_LEN);
    USBD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk|USBD_CEPINTEN_STSDONEIEN_Msk);

    MSC_InitForHighSpeed();

    g_sCSW.dCSWSignature = CSW_SIGNATURE;
    g_u32MassBase = MASS_BUFFER_ADDR;
    g_u32StorageBase = STORAGE_BUFFER_ADDR;
		
    for(i =0;i<0x40*512;i++)
        pu8FAT[i] = 0;

    for(i =0;i<0x40;i++)
    {
        pu8FAT[i*512 + 0] = 0xF8;
        pu8FAT[i*512 + 1] = 0xFF;
        pu8FAT[i*512 + 2] = 0xFF;
        pu8FAT[i*512 + 3] = 0x00;		
    }	
}

void MSC_ClassRequest(void)
{
    if (gUsbCmd.bmRequestType & 0x80) { /* request data transfer direction */
        // Device to host
        switch (gUsbCmd.bRequest) {
        case GET_MAX_LUN: {
            /* Check interface number with cfg descriptor and check wValue = 0, wLength = 1 */
            if ((gUsbCmd.wValue == 0) && (gUsbCmd.wIndex == 0) && (gUsbCmd.wLength == 1)) {
            /* Return current configuration setting */
            USBD_PrepareCtrlIn((uint8_t *)&g_u32MSCMaxLun, 1);
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
            }
            else {/* Invalid Get MaxLun command */
                USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
            }
            break;
        }
        default: {
            /* Setup error, stall the device */
            USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
            break;
        }
        }
    } else {
        /* Host to device */
        switch (gUsbCmd.bRequest) {
        case BULK_ONLY_MASS_STORAGE_RESET: {
            /* Check interface number with cfg descriptor and check wValue = 0, wLength = 0 */
            if ((gUsbCmd.wValue == 0) && (gUsbCmd.wIndex == 0) && (gUsbCmd.wLength == 0)) {
                g_u8Prevent = 1;
                /* Status stage */
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STSDONEIF_Msk);
                USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
                USBD_ENABLE_CEP_INT(USBD_CEPINTEN_STSDONEIEN_Msk);
								USBD_SET_CEP_STATE(USB_CEPCTL_FLUSH); 
                g_u32EpStallLock = 0;

                USBD_ResetDMA();
                USBD->EP[EPA].EPRSPCTL = USBD_EPRSPCTL_FLUSH_Msk;
                USBD->EP[EPB].EPRSPCTL = USBD_EPRSPCTL_FLUSH_Msk;
                g_u8BulkState = BULK_CBW;
                g_u8MscOutPacket = 0;
            }
            else {/* Invalid Get MaxLun command */
                USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
            }
            break;
        }
        default: {
            /* Stall */
            /* Setup error, stall the device */
            USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
            break;
        }
        }
    }
}


void MSC_RequestSense(void)
{
    memset((uint8_t *)(g_u32MassBase), 0, 18);
    if (g_u8Prevent) {
        g_u8Prevent = 0;
        *(uint8_t *)(g_u32MassBase) = 0x70;
    } else
        *(uint8_t *)(g_u32MassBase) = 0xf0;

    *(uint8_t *)(g_u32MassBase + 2) = g_au8SenseKey[0];
    *(uint8_t *)(g_u32MassBase + 7) = 0x0a;
    *(uint8_t *)(g_u32MassBase + 12) = g_au8SenseKey[1];
    *(uint8_t *)(g_u32MassBase + 13) = g_au8SenseKey[2];
    MSC_BulkIn(g_u32MassBase, g_sCBW.dCBWDataTransferLength);

    g_au8SenseKey[0] = 0;
    g_au8SenseKey[1] = 0;
    g_au8SenseKey[2] = 0;
}

void MSC_ReadFormatCapacity(void)
{
    memset((uint8_t *)g_u32MassBase, 0, 36);

    *((uint8_t *)(g_u32MassBase+3)) = 0x10;
    *((uint8_t *)(g_u32MassBase+4)) = *((uint8_t *)&g_TotalSectors+3);
    *((uint8_t *)(g_u32MassBase+5)) = *((uint8_t *)&g_TotalSectors+2);
    *((uint8_t *)(g_u32MassBase+6)) = *((uint8_t *)&g_TotalSectors+1);
    *((uint8_t *)(g_u32MassBase+7)) = *((uint8_t *)&g_TotalSectors+0);
    *((uint8_t *)(g_u32MassBase+8)) = 0x02;
    *((uint8_t *)(g_u32MassBase+10)) = 0x02;
    *((uint8_t *)(g_u32MassBase+12)) = *((uint8_t *)&g_TotalSectors+3);
    *((uint8_t *)(g_u32MassBase+13)) = *((uint8_t *)&g_TotalSectors+2);
    *((uint8_t *)(g_u32MassBase+14)) = *((uint8_t *)&g_TotalSectors+1);
    *((uint8_t *)(g_u32MassBase+15)) = *((uint8_t *)&g_TotalSectors+0);
    *((uint8_t *)(g_u32MassBase+18)) = 0x02;

    MSC_BulkIn(g_u32MassBase, g_sCBW.dCBWDataTransferLength);
}

void MSC_ReadCapacity(void)
{
    uint32_t tmp;

    memset((uint8_t *)g_u32MassBase, 0, 36);

    tmp = g_TotalSectors - 1;
    *((uint8_t *)(g_u32MassBase+0)) = *((uint8_t *)&tmp+3);
    *((uint8_t *)(g_u32MassBase+1)) = *((uint8_t *)&tmp+2);
    *((uint8_t *)(g_u32MassBase+2)) = *((uint8_t *)&tmp+1);
    *((uint8_t *)(g_u32MassBase+3)) = *((uint8_t *)&tmp+0);
    *((uint8_t *)(g_u32MassBase+6)) = 0x02;

    MSC_BulkIn(g_u32MassBase, g_sCBW.dCBWDataTransferLength);
}

void MSC_ModeSense10(void)
{
    uint8_t i,j;
    uint8_t NumHead,NumSector;
    uint16_t NumCyl=0;

    /* Clear the command buffer */
    *((uint32_t *)g_u32MassBase  ) = 0;
    *((uint32_t *)g_u32MassBase + 1) = 0;

    switch (g_sCBW.au8Data[0]) {
    case 0x01:
        *((uint8_t *)g_u32MassBase) = 19;
        i = 8;
        for (j = 0; j<12; j++, i++)
            *((uint8_t *)(g_u32MassBase+i)) = g_au8ModePage_01[j];
        break;

    case 0x05:
        *((uint8_t *)g_u32MassBase) = 39;
        i = 8;
        for (j = 0; j<32; j++, i++)
            *((uint8_t *)(g_u32MassBase+i)) = g_au8ModePage_05[j];

        NumHead = 2;
        NumSector = 64;
        NumCyl = g_TotalSectors / 128;

        *((uint8_t *)(g_u32MassBase+12)) = NumHead;
        *((uint8_t *)(g_u32MassBase+13)) = NumSector;
        *((uint8_t *)(g_u32MassBase+16)) = (uint8_t)(NumCyl >> 8);
        *((uint8_t *)(g_u32MassBase+17)) = (uint8_t)(NumCyl & 0x00ff);
        break;

    case 0x1B:
        *((uint8_t *)g_u32MassBase) = 19;
        i = 8;
        for (j = 0; j<12; j++, i++)
            *((uint8_t *)(g_u32MassBase+i)) = g_au8ModePage_1B[j];
        break;

    case 0x1C:
        *((uint8_t *)g_u32MassBase) = 15;
        i = 8;
        for (j = 0; j<8; j++, i++)
            *((uint8_t *)(g_u32MassBase+i)) = g_au8ModePage_1C[j];
        break;

    case 0x3F:
        *((uint8_t *)g_u32MassBase) = 0x47;
        i = 8;
        for (j = 0; j<12; j++, i++)
            *((uint8_t *)(g_u32MassBase+i)) = g_au8ModePage_01[j];
        for (j = 0; j<32; j++, i++)
            *((uint8_t *)(g_u32MassBase+i)) = g_au8ModePage_05[j];
        for (j = 0; j<12; j++, i++)
            *((uint8_t *)(g_u32MassBase+i)) = g_au8ModePage_1B[j];
        for (j = 0; j<8; j++, i++)
            *((uint8_t *)(g_u32MassBase+i)) = g_au8ModePage_1C[j];

        NumHead = 2;
        NumSector = 64;
        NumCyl = g_TotalSectors / 128;

        *((uint8_t *)(g_u32MassBase+24)) = NumHead;
        *((uint8_t *)(g_u32MassBase+25)) = NumSector;
        *((uint8_t *)(g_u32MassBase+28)) = (uint8_t)(NumCyl >> 8);
        *((uint8_t *)(g_u32MassBase+29)) = (uint8_t)(NumCyl & 0x00ff);
        break;

    default:
        g_au8SenseKey[0] = 0x05;
        g_au8SenseKey[1] = 0x24;
        g_au8SenseKey[2] = 0x00;
    }
    MSC_BulkIn(g_u32MassBase, g_sCBW.dCBWDataTransferLength);
}

void MSC_ModeSense6(void)
{
    uint8_t i;

    for (i = 0; i<4; i++)
        *((uint8_t *)(g_u32MassBase+i)) = g_au8ModePage[i];

    MSC_BulkIn(g_u32MassBase, g_sCBW.dCBWDataTransferLength);
}

void MSC_BulkOut(uint32_t u32Addr, uint32_t u32Len)
{
    uint32_t u32Loop;
    uint32_t i;

    /* bulk out, dma write, epnum = 2 */
    USBD_SET_DMA_WRITE(BULK_OUT_EP_NUM);
    g_usbd_ShortPacket = 0;

    u32Loop = u32Len / USBD_MAX_DMA_LEN;
    for (i=0; i<u32Loop; i++) {
        MSC_ActiveDMA(u32Addr+i*USBD_MAX_DMA_LEN, USBD_MAX_DMA_LEN);
    }

    u32Loop = u32Len % USBD_MAX_DMA_LEN;
    if (u32Loop) {
        MSC_ActiveDMA(u32Addr+i*USBD_MAX_DMA_LEN, u32Loop);
    }
}

void MSC_BulkIn(uint32_t u32Addr, uint32_t u32Len)
{
    uint32_t u32Loop;
    uint32_t i, addr, count;

    /* bulk in, dma read, epnum = 1 */
    USBD_SET_DMA_READ(BULK_IN_EP_NUM);

    u32Loop = u32Len / USBD_MAX_DMA_LEN;
    for (i=0; i<u32Loop; i++) {
        USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_TXPKIEN_Msk);
        g_usbd_ShortPacket = 0;
        while(1) {
            if (USBD_GET_EP_INT_FLAG(EPA) & USBD_EPINTSTS_BUFEMPTYIF_Msk) {
                MSC_ActiveDMA(u32Addr+i*USBD_MAX_DMA_LEN, USBD_MAX_DMA_LEN);
                break;
            }
        }
    }

    addr = u32Addr + i * USBD_MAX_DMA_LEN;
    u32Loop = u32Len % USBD_MAX_DMA_LEN;
    if (u32Loop) {
        count = u32Loop / g_u32EpMaxPacketSize;
        if (count) {
            USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_TXPKIEN_Msk);
            g_usbd_ShortPacket = 0;
            while(1) {
                if (USBD_GET_EP_INT_FLAG(EPA) & USBD_EPINTSTS_BUFEMPTYIF_Msk) {
                    MSC_ActiveDMA(addr, count * g_u32EpMaxPacketSize);
                    break;
                }
            }
            addr += (count * g_u32EpMaxPacketSize);
        }
        count = u32Loop % g_u32EpMaxPacketSize;
        if (count) {
            USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_TXPKIEN_Msk);
            g_usbd_ShortPacket = 1;
            while(1) {
                if (USBD_GET_EP_INT_FLAG(EPA) & USBD_EPINTSTS_BUFEMPTYIF_Msk) {
                    MSC_ActiveDMA(addr, count);
                    break;
                }
            }
        }
    }
}


void MSC_ReceiveCBW(uint32_t u32Buf, uint32_t u32Len)
{
    uint32_t volatile i;

    /* bulk out, dma write, epnum = 2 */
    USBD_SET_DMA_WRITE(BULK_OUT_EP_NUM);

    /* Enable BUS interrupt */
    USBD_ENABLE_BUS_INT(USBD_BUSINTEN_DMADONEIEN_Msk|USBD_BUSINTEN_SUSPENDIEN_Msk|USBD_BUSINTEN_RSTIEN_Msk|USBD_BUSINTEN_VBUSDETIEN_Msk);
    USBD_SET_DMA_ADDR(u32Buf);
    USBD_SET_DMA_LEN(u32Len);

    g_usbd_DmaDone = 0;
    USBD_ENABLE_DMA();

    while(g_usbd_Configured) {
        if (g_usbd_DmaDone == 1)
            break;

        if (!USBD_IS_ATTACHED())
            break;
    }
}

#define FAT_SECTORS_ADDRESS      u8FormatData[14]
#define FAT_SECTORS				((u8FormatData[23] << 8) + u8FormatData[22])	* 2
#define ROOT_DIR_ADDRESS         FAT_SECTORS_ADDRESS + FAT_SECTORS
#define ROOT_DIR_SECTORS        ((((u8FormatData[18] << 8) + u8FormatData[17]) * 32) / 512)  
#define DATA_SECTOR_ADDRESS     ((FAT_SECTORS + FAT_SECTORS_ADDRESS + ROOT_DIR_SECTORS))

extern uint8_t *pu8FAT;     
#define FLASH_PAGE_SIZE      512

#ifdef __ICCARM__
#pragma data_alignment=4
uint32_t  u32buff[128];
#else
__align(4) uint32_t  u32buff[128];
#endif

int8_t i8FileIndex[FILE_NAME_LENGTH] = 
{   1,   3,   5,   7,   9,  14,  16,  18,  20,  22,  24,  28,  30,
	-31, -29, -27, -25, -23, -18, -16, -14, -12, -10,  -8,  -4,  -2,
  -63, -61, -59, -57, -55, -50, -48, -46, -44, -42, -40, -36, -34 
};

void Wt10_Command(void)
{
    uint32_t volatile lba;
    uint32_t volatile sector_count = 0;
    uint32_t volatile i,k,l;

    lba = get_be32(&g_sCBW.au8Data[0]) ;	
		
    sector_count = g_sCBW.dCBWDataTransferLength /512;
		
    for(i=0;i<sector_count;i++)
    {
        int sectorIdx = lba + i;  
        if (sectorIdx == 0x00000000)
            MSC_BulkOut((uint32_t)u8FormatData,  512);
        else
        {
            if ( (sectorIdx >= FAT_SECTORS_ADDRESS) && (sectorIdx <= (FAT_SECTORS_ADDRESS+FAT_SECTORS -1)) )
                MSC_BulkOut((uint32_t)(((uint32_t)pu8FAT) + ((sectorIdx - FAT_SECTORS_ADDRESS) % 0x80) * 512),  512);
            else if (sectorIdx == ROOT_DIR_ADDRESS) /* root dir */
						{
                MSC_BulkOut((uint32_t)u8RootDirData,  512);                                 

                if(g_UpdateEnable == 0 || (g_u8MACOS == 1 && g_u8ShowFile != 1))           /* Check File name (When Update Function is not Enabled or MAC OS) */
                {								
                    for(k =0;k<32;k++)                                                     /* Check Root Directory */
                    {
                        for(l=0;l<FILE_NAME_LENGTH;l++)
                        {
                            if(u8FileName[l] != 0)                                         /* Need to Check File Name - Character*/
                                if(u8RootDirData[k*16+i8FileIndex[l]] != u8FileName[l])    /* Check File Name */
                                    break; 
                        }
												
                        if(l == FILE_NAME_LENGTH)                                          /* Match */ 
                        {													
                            g_file_size = (uint32_t *)&u8RootDirData[k *16 + 60];          /* Get File Size */
													
                            if(*g_file_size == 0)
                                continue;
													
                            g_UpdateEnable = 1;                                            /* Enable Update Function */

                            printf("Erase 0x%X for EndTag\n", (EndTagAddr & ~0x3FFF));														
														SPIFlash_EraseSector(EndTagAddr & ~0x3FFF);
														
                            printf("File Name:");
                            for(l=0;l<FILE_NAME_LENGTH;l++)                                /* Display the Update File Name */
                            {
                                if((u8RootDirData[k *16 + i8FileIndex[l]] != 0) && (u8RootDirData[k *16 + i8FileIndex[l]+1] == 0) )
                                    printf("%c",u8RootDirData[k *16 + i8FileIndex[l]]);		
                                else
                                    break;	
                            }
                            if(g_u8MACOS == 1)
                            {
                                g_u8ShowFile = 1;
															
                                printf("\nFile Size: %dB\n",*g_file_size);

                                g_TimerInit = 1;
					
                                /* Enable IP clock */
                                CLK_EnableModuleClock(TMR0_MODULE);
											
                                /* Set timer frequency */
                                TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 5);

                                /* Enable timer interrupt */
                                TIMER_EnableInt(TIMER0);
											
                                NVIC_EnableIRQ(TMR0_IRQn);

                                /* Start Timer 0 */
                                TIMER_Start(TIMER0);				 															
                            }															  
                            break;
                        }								
                    }					
                }    
            }
            else if(sectorIdx >= DATA_SECTOR_ADDRESS)
            {				
                MSC_BulkOut(SPI_BUFFER_ICP_MSC,  512);				
                if(g_UpdateEnable)		
                    UpdateSpiFlash();		
                else
                {
                    uint8_t *pu8Data = (uint8_t *)SPI_BUFFER_ICP_MSC;
									
                    if(pu8Data[8] == 'M' && pu8Data[9] == 'a' && pu8Data[10] == 'c')									
                    {
                        if(g_u8MACOS == 0)	
                        {								
                            char *puchar = (char *)(&pu8Data[8]);
                            printf("%s\n",puchar);
                            g_u8MACOS = 1; 													
                        }					
                        /* Finder progresses a copy of a file - HFS type code 'brok' & HFS creator code 'MACS' */ 												 
                        if(pu8Data[50] == 'b' && pu8Data[51] == 'r' && pu8Data[52] == 'o' && pu8Data[53] == 'k' &&
                           pu8Data[54] == 'M' && pu8Data[55] == 'A' && pu8Data[56] == 'C' && pu8Data[57] == 'S')
                        {
                            g_u8MACOS_Update = 1;								
                        }
                    }			
								}
            }
            else
                MSC_BulkOut(SPI_BUFFER_ICP_MSC,  512 );
        }
    }
    if(g_u8MACOS_Update)      /* Finder progresses a copy of a file */
    {			
        printf("EndTAG ADDR %X Erase %X\n", EndTagAddr, (EndTagAddr & ~0x3FFF));														
		    SPIFlash_EraseSector(EndTagAddr & ~0x3FFF);
        g_UpdateEnable = 1;   /* Enable Update Function */
    }			
}


void Rd10_Command(void)
{
    uint32_t volatile lba,sector_count, i, j;

    lba = get_be32(&g_sCBW.au8Data[0]);
	
    sector_count = g_sCBW.dCBWDataTransferLength /512;	
	
    for(i=0;i<sector_count;i++)
    {	
        int sectorIdx = lba + i;  

        for (j = 0; j < FLASH_PAGE_SIZE/4; j++)
            u32buff[j] = 0;

        if (sectorIdx == 0x00000000)
        {
            u8FormatData[FLASH_PAGE_SIZE-4] = 0x00;    
            u8FormatData[FLASH_PAGE_SIZE-3] = 0x00;   
            u8FormatData[FLASH_PAGE_SIZE-2] = 0x55;   
            u8FormatData[FLASH_PAGE_SIZE-1] = 0xAA;   
            MSC_BulkIn((uint32_t)u8FormatData, 512);        
        }
        else
        {
            if ( (sectorIdx >= FAT_SECTORS_ADDRESS) && (sectorIdx <= (FAT_SECTORS_ADDRESS+FAT_SECTORS -1)) )
                MSC_BulkIn((((uint32_t)pu8FAT) + ((sectorIdx - FAT_SECTORS_ADDRESS) % 0x80) * 512), 512);
            else if (sectorIdx == ROOT_DIR_ADDRESS) /* root dir */
                MSC_BulkIn((uint32_t)u8RootDirData, 512);
            else
                MSC_BulkIn((uint32_t)u32buff, 512);
        }	 
    }			
}

void MSC_ProcessCmd(void)
{
    uint32_t i;
    uint32_t Hcount, Dcount;

    if (g_u8MscOutPacket) {
        g_u8MscOutPacket = 0;

        if (g_u8BulkState == BULK_CBW) {

            /* Check CBW */
            g_u32CbwSize = USBD->EP[EPB].EPDATCNT & 0xffff;
            MSC_ReceiveCBW(g_u32MassBase, g_u32CbwSize);

            /* Check Signature & length of CBW */
            if ((*(uint32_t *)(g_u32MassBase) != CBW_SIGNATURE) || (g_u32CbwSize != 31)) {
                /* Invalid CBW */
                g_u8Prevent = 1;
                USBD_SetEpStall(EPA);
                USBD_SetEpStall(EPB);
                g_u32EpStallLock = (1 << EPA) | (1 << EPB);
                return;
            }

            /* Get the CBW */
            for (i = 0; i < 31; i++)
                *((uint8_t *) (&g_sCBW.dCBWSignature) + i) = *(uint8_t *)(g_u32MassBase + i);

            /* Prepare to echo the tag from CBW to CSW */
            g_sCSW.dCSWTag = g_sCBW.dCBWTag;
            Hcount = g_sCBW.dCBWDataTransferLength;

            /* Parse Op-Code of CBW */
        switch (g_sCBW.u8OPCode) {
            case UFI_READ_12:
            case UFI_READ_10: {
                Dcount = (get_be32(&g_sCBW.au8Data[4])>>8) * 512;
                if (g_sCBW.bmCBWFlags == 0x80) {    /* IN */
                    if (Hcount == Dcount) { /* Hi == Di (Case 6)*/
                        g_sCSW.bCSWStatus = 0;
                    }
                    else if (Hcount < Dcount) {  /* Hn < Di (Case 2) || Hi < Di (Case 7) */
                        if (Hcount) {   /* Hi < Di (Case 7) */
                            g_u8Prevent = 1;
                            g_sCSW.bCSWStatus = 0x01;
                        }
                        else {  /* Hn < Di (Case 2) */
                            g_u8Prevent = 1;
                            g_sCSW.bCSWStatus = 0x01;
                            g_sCSW.dCSWDataResidue = 0;
                            MSC_AckCmd();
                            return;
                        }
                    }
                    else if (Hcount > Dcount) { /* Hi > Dn (Case 4) || Hi > Di (Case 5) */
                        g_u8Prevent = 1;
                        g_sCSW.bCSWStatus = 0x01;
                    }
                }
                else {  /* Ho <> Di (Case 10) */
                    g_u8Prevent = 1;
                    USBD_SetEpStall(EPB);
                    g_sCSW.bCSWStatus = 0x01;
                    g_sCSW.dCSWDataResidue = Hcount;
                    MSC_AckCmd();
                    return;
                }
                Rd10_Command();
                g_sCSW.dCSWDataResidue = 0;
                break;
            }
            case UFI_WRITE_12:
            case UFI_WRITE_10: {
                Dcount = (get_be32(&g_sCBW.au8Data[4])>>8) * 512;
                if (g_sCBW.bmCBWFlags == 0x00) {    /* OUT */
                    if (Hcount == Dcount) { /* Ho == Do (Case 12)*/
                        g_sCSW.bCSWStatus = 0;
                    }
                    else if (Hcount < Dcount) { /* Hn < Do (Case 3) || Ho < Do (Case 13) */
                        g_u8Prevent = 1;
                        g_sCSW.bCSWStatus = 0x1;
                        if (Hcount == 0) {  /* Hn < Do (Case 3) */
                            g_sCSW.dCSWDataResidue = 0;
                            MSC_AckCmd();
                            return;
                        }
                    }
                    else if (Hcount > Dcount) { /* Ho > Do (Case 11) */
                        g_u8Prevent = 1;
                        g_sCSW.bCSWStatus = 0x1;
                    }
										Wt10_Command();	
                    g_sCSW.dCSWDataResidue = 0;
                }
                else {  /* Hi <> Do (Case 8) */
                    g_u8Prevent = 1;
                    g_sCSW.bCSWStatus = 0x1;
                    USBD_SetEpStall(EPA);
                    g_sCSW.dCSWDataResidue = Hcount;
                    MSC_AckCmd();
                    return;
                }
            break;
        }
        case UFI_PREVENT_ALLOW_MEDIUM_REMOVAL: {
            if (g_sCBW.au8Data[2] & 0x01) {
                g_au8SenseKey[0] = 0x05;  //INVALID COMMAND
                g_au8SenseKey[1] = 0x24;
                g_au8SenseKey[2] = 0;
                g_u8Prevent = 1;
            } else
                g_u8Prevent = 0;
                g_sCSW.dCSWDataResidue = 0;
                g_sCSW.bCSWStatus = g_u8Prevent;
            break;
        }
            case UFI_TEST_UNIT_READY: {
                if (Hcount != 0) {
                    if (g_sCBW.bmCBWFlags == 0) {   /* Ho > Dn (Case 9) */
                        g_u8Prevent = 1;
                        USBD_SetEpStall(EPB);
                        g_sCSW.bCSWStatus = 0x1;
                        g_sCSW.dCSWDataResidue = Hcount;
                        MSC_AckCmd();
                    }
                }
                else {  /* Hn == Dn (Case 1) */
                    if (g_u8Remove) {
                        g_sCSW.dCSWDataResidue = 0;
                        g_sCSW.bCSWStatus = 1;
                        g_au8SenseKey[0] = 0x02;    /* Not ready */
                        g_au8SenseKey[1] = 0x3A;
                        g_au8SenseKey[2] = 0;
                        g_u8Prevent = 1;
                    }
                    else {
                        g_sCSW.bCSWStatus = 0;
                        g_sCSW.dCSWDataResidue = 0;
                    }
                    MSC_AckCmd();
                }
                return;
            }
            case UFI_START_STOP: {
                if ((g_sCBW.au8Data[2] & 0x03) == 0x2) {
                    g_u8Remove = 1;
                }
            }
            case UFI_VERIFY_10: {
                g_sCSW.dCSWDataResidue = 0;
                g_sCSW.bCSWStatus = 0;
                break;
            }
            case UFI_REQUEST_SENSE: {
                if ((Hcount > 0) && (Hcount <= 18)){
                    MSC_RequestSense();
                    g_sCSW.bCSWStatus = 0;
                    g_sCSW.dCSWDataResidue = 0;
                }
                else {
                    USBD_SetEpStall(EPA);
                    g_u8Prevent = 1;
                    g_sCSW.bCSWStatus = 0x01;
                    g_sCSW.dCSWDataResidue = 0;
                }
                break;
            }
            case UFI_READ_FORMAT_CAPACITY: {
                MSC_ReadFormatCapacity();
                g_sCSW.dCSWDataResidue = 0;
                g_sCSW.bCSWStatus = 0;
                break;
            }
            case UFI_READ_CAPACITY: {
                MSC_ReadCapacity();
                g_sCSW.dCSWDataResidue = 0;
                g_sCSW.bCSWStatus = 0;
                break;
            }
            case UFI_MODE_SELECT_6:
            case UFI_MODE_SELECT_10: {
                MSC_BulkOut(g_u32StorageBase, g_sCBW.dCBWDataTransferLength);
                g_sCSW.dCSWDataResidue = 0;
                g_sCSW.bCSWStatus = 0;
                break;
            }
            case UFI_MODE_SENSE_10: {
                MSC_ModeSense10();
                g_sCSW.dCSWDataResidue = 0;
                g_sCSW.bCSWStatus = 0;
                break;
            }
            case UFI_MODE_SENSE_6: {
                MSC_ModeSense6();
                g_sCSW.dCSWDataResidue = 0;
                g_sCSW.bCSWStatus = 0;
                break;
            }
            case UFI_INQUIRY: {
                if ((Hcount > 0) && (Hcount <= 36)){
                    /* Bulk IN buffer */
                    USBD_MemCopy((uint8_t *)(g_u32MassBase), (uint8_t *)g_au8InquiryID, Hcount);
                    MSC_BulkIn(g_u32MassBase, Hcount);
                    g_sCSW.bCSWStatus = 0;
                    g_sCSW.dCSWDataResidue = 0;
                }
                else {
                    USBD_SetEpStall(EPA);
                    g_u8Prevent = 1;
                    g_sCSW.bCSWStatus = 0x01;
                    g_sCSW.dCSWDataResidue = 0;
                }
                break;
            }
            case UFI_READ_16:
            {
                USBD_SetEpStall(EPA);
                g_u8Prevent = 1;
                g_sCSW.bCSWStatus = 0x01;
                g_sCSW.dCSWDataResidue = 0;
                break;
            }
            default: {
                /* Unsupported command */
                g_au8SenseKey[0] = 0x05;
                g_au8SenseKey[1] = 0x20;
                g_au8SenseKey[2] = 0x00;

                /* If CBW request for data phase, just return zero packet to end data phase */
                if (g_sCBW.dCBWDataTransferLength > 0)
                    g_sCSW.dCSWDataResidue = Hcount;
                else
                    g_sCSW.dCSWDataResidue = 0;
                g_sCSW.bCSWStatus = g_u8Prevent;
            }
        }
        MSC_AckCmd();
        }
    }

    /* For MSC compliance test, if received an invalid command should stall it */
    while(1) {
        if (USBD->EP[EPA].EPINTSTS & USBD_EPINTSTS_BUFEMPTYIF_Msk) {
            if (g_u32EpStallLock & 0x1)
                USBD_SetEpStall(EPA);
            if (g_u32EpStallLock & 0x2)
                USBD_SetEpStall(EPB);
            break;
        }
        else {
            if ((USBD_GetEpStall(EPA) == 0) && (!(USBD->EP[EPA].EPINTSTS & USBD_EPINTSTS_BUFEMPTYIF_Msk)))
                USBD->EP[EPA].EPRSPCTL = USBD->EP[EPA].EPRSPCTL & 0x10 | USB_EP_RSPCTL_SHORTTXEN;
        }
    }
}

void MSC_ActiveDMA(uint32_t u32Addr, uint32_t u32Len)
{
    /* Enable BUS interrupt */
    USBD_ENABLE_BUS_INT(USBD_BUSINTEN_DMADONEIEN_Msk|USBD_BUSINTEN_SUSPENDIEN_Msk|USBD_BUSINTEN_RSTIEN_Msk|USBD_BUSINTEN_VBUSDETIEN_Msk);

    USBD_SET_DMA_ADDR(u32Addr);
    USBD_SET_DMA_LEN(u32Len);
    g_usbd_DmaDone = 0;
    USBD_ENABLE_DMA();
    while(g_usbd_Configured) {
        if (g_usbd_DmaDone)
            break;

        if (!USBD_IS_ATTACHED())
            break;
    }
}

void MSC_AckCmd(void)
{
    USBD_MemCopy((uint8_t *)g_u32MassBase, (uint8_t *)&g_sCSW.dCSWSignature, 13);
    MSC_BulkIn(g_u32MassBase, 13);
    g_u8BulkState = BULK_CBW;
    g_u8MscOutPacket = 0;
}
