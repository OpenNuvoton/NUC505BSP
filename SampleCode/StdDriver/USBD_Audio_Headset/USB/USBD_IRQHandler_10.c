#include "NUC505Series.h"

#include "AudioLib.h"
#include "usbd_audio_10.h"

extern uint8_t  g_uac_10_flag;
extern uint8_t  g_uac_20_flag;
extern uint32_t g_usbd_rx_flag;
extern uint32_t g_usbd_tx_flag;
extern uint32_t g_u32ClassOUT_10;
extern uint8_t  g_set_timer_flag;

/*--------------------------------------------------------------------------*/
/**
 * @brief       USBD Interrupt Service Routine
 *
 * @param[in]   None
 *
 * @return      None
 *
 * @details     This function is the USBD ISR
 */
void USBD_IRQHandler_10(S_AUDIO_LIB* psAudioLib)
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
            USBD_ResetDMA();
            g_usbd_tx_flag = 0;
            g_usbd_rx_flag = 0;
            g_u32ClassOUT_10 = 0;
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
        
        if (IrqSt & USBD_BUSINTSTS_DMADONEIF_Msk)
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_DMADONEIF_Msk);
        
        if (IrqSt & USBD_BUSINTSTS_PHYCLKVLDIF_Msk)
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_PHYCLKVLDIF_Msk);
        
        if (IrqSt & USBD_BUSINTSTS_VBUSDETIF_Msk) {
            if (USBD_IS_ATTACHED()) {
                /* USB Plug In */
                USBD_ENABLE_USB();
            } else {
                /* USB Un-plug */
                USBD_DISABLE_USB();
                USBD->EP[EPA].EPRSPCTL |= USBD_EPRSPCTL_FLUSH_Msk;
                USBD->EP[EPB].EPRSPCTL |= USBD_EPRSPCTL_FLUSH_Msk;
            }
            g_usbd_tx_flag = 0;
            g_usbd_rx_flag = 0;
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_VBUSDETIF_Msk);
        }
    }
    
    if (IrqStL & USBD_GINTSTS_CEPIF_Msk) {
        IrqSt = USBD->CEPINTSTS & USBD->CEPINTEN;
        
        if (IrqSt & USBD_CEPINTSTS_SETUPTKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_SETUPTKIF_Msk);
        }
        
        if (IrqSt & USBD_CEPINTSTS_SETUPPKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_SETUPPKIF_Msk);
            USBD_ProcessSetupPacket();
        }
        
        if (IrqSt & USBD_CEPINTSTS_OUTTKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_OUTTKIF_Msk);
        }
        
        if (IrqSt & USBD_CEPINTSTS_INTKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
            if(g_u32ClassOUT_10)
            {
                UAC_ClassOUT_10(psAudioLib);
                g_u32ClassOUT_10 = 0;
                USBD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk);
            }
            else
            {
                if (!(IrqSt & USBD_CEPINTSTS_STSDONEIF_Msk)) {
                    USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_TXPKIF_Msk);
                    USBD_ENABLE_CEP_INT(USBD_CEPINTEN_TXPKIEN_Msk);
                    USBD_CtrlIn();
                } else {
                    USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_TXPKIF_Msk);
                    USBD_ENABLE_CEP_INT(USBD_CEPINTEN_TXPKIEN_Msk|USBD_CEPINTEN_STSDONEIEN_Msk);
                }
            }
        }
        
        if (IrqSt & USBD_CEPINTSTS_PINGIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_PINGIF_Msk);
        }
        
        if (IrqSt & USBD_CEPINTSTS_TXPKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STSDONEIF_Msk);
            USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
            if (g_usbd_CtrlInSize) {
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                USBD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
            } else {
                if (g_usbd_CtrlZero == 1)
                    USBD_SET_CEP_STATE(USB_CEPCTL_ZEROLEN);
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STSDONEIF_Msk);
                USBD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk|USBD_CEPINTEN_STSDONEIEN_Msk);
            }
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_TXPKIF_Msk);
        }
        
        if (IrqSt & USBD_CEPINTSTS_RXPKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_RXPKIF_Msk);
            USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk|USBD_CEPINTEN_STSDONEIEN_Msk);
        }
        
        if (IrqSt & USBD_CEPINTSTS_NAKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_NAKIF_Msk);
        }
        
        if (IrqSt & USBD_CEPINTSTS_STALLIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STALLIF_Msk);
        }
        
        if (IrqSt & USBD_CEPINTSTS_ERRIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_ERRIF_Msk);
        }
        
        if (IrqSt & USBD_CEPINTSTS_STSDONEIF_Msk) {
            USBD_UpdateDeviceState();
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STSDONEIF_Msk);
            USBD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk);
        }
        
        if (IrqSt & USBD_CEPINTSTS_BUFFULLIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_BUFFULLIF_Msk);
        }
        
        if (IrqSt & USBD_CEPINTSTS_BUFEMPTYIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_BUFEMPTYIF_Msk);
        }
    }
    
    /*
       To avoid USB Buffer Arbitration issue (Can't read USB Buffer when Host read ISO IN buffer)    
        - Write data to EPA Buffer when EPA IN Token Interrupt occurs.
        - Read EPB Buffer after EPA Buffer is empty or EPA Data Transmitted 
          (UAC 1.0 Host issues IN Token to read data ervery 1 ms, Host will not read ISO IN buffer in 1ms).      
    */    
    
    /* EPA (Isochronous IN Endpoint) */
    if (IrqStL & USBD_GINTSTS_EPAIF_Msk) {
        IrqSt = USBD->EP[EPA].EPINTSTS & USBD->EP[EPA].EPINTEN;
        if(IrqSt & USBD_EPINTSTS_INTKIF_Msk)          /* EPA IN Toekn (Host request data) */
        {
            USBD_CLR_EP_INT_FLAG(EPA, USBD_EPINTSTS_INTKIF_Msk);
          
            /* Write Audio data to EPA for ISO IN (Record) */          
            EPA_Handler(psAudioLib);
          
            /* 
               Enable EPA IN Token Interrupt for Audio Record (For Write data to EPA Buffer) &
               Enable Data Transmitted Interrupt for Audio Play (For Read data from EPB Buffer)
            */           
            USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_INTKIEN_Msk|USBD_EPINTEN_TXPKIEN_Msk);       
        }
        else if(IrqSt & USBD_EPINTSTS_BUFEMPTYIF_Msk) /* EPA Buffer Empty */
        {
            USBD_CLR_EP_INT_FLAG(EPA, USBD_EPINTSTS_BUFEMPTYIF_Msk);
                    
            if(g_usbd_rx_flag)     /* EPB Received data */
            {
                /*
                   For Speaker Only                   
                   Wait EPA Buffer Empty to avoid USB Buffer Arbitration issue
                   (Can't read USB Buffer when Host read ISO IN buffer)                             
                */                 
                /* Read Audio data from EPB for ISO OUT (Play) */          
                EPB_Handler(psAudioLib);
              
                g_usbd_rx_flag = 0;
            }
            /*
               Enable EPA IN Token Interrupt for Audio Record (For Write data to EPA Buffer) & 
               Disable EPA Data Transmitted Interrupt for Audio Play (For Read data from EPB Buffer)
            */    
            USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_INTKIEN_Msk);
        }
        else if(IrqSt & USBD_EPINTSTS_TXPKIF_Msk)     /* EPA Data Transmitted */
        {
            USBD_CLR_EP_INT_FLAG(EPA, USBD_EPINTSTS_TXPKIF_Msk);      
          
            if(g_usbd_rx_flag)     /* EPB Received data */
            {
                /*                          
                   UAC 1.0 Host issus IN Token to read data ervery 1 ms.                                               
                   Wait EPA Data Transmitted (Host will read ISO IN buffer after 1 ms)              
                   to avoid USB Buffer Arbitration issue
                   (Can't read USB Buffer when Host read ISO IN buffer)
                */                                 
                /* Read Audio data from EPB for ISO OUT (Play) */  
                EPB_Handler(psAudioLib);
              
                g_usbd_rx_flag = 0;
            }
            /*
               Enable EPA IN Token Interrupt for Audio Record (For Write data to EPA Buffer) & 
               Disable EPA Data Transmitted Interrupt for Audio Play (For Read data from EPB Buffer) 
            */      
            USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_INTKIEN_Msk);
        }  
    }
    /* EPB (Isochronous OUT Endpoint) */
    if (IrqStL & USBD_GINTSTS_EPBIF_Msk) {
        IrqSt = USBD->EP[EPB].EPINTSTS & USBD->EP[EPB].EPINTEN;
        if(IrqSt & USBD_EPINTSTS_RXPKIF_Msk)          /* EPB Data Received (Host send data) */
        {
            g_usbd_rx_flag++;  
          
            USBD_CLR_EP_INT_FLAG(EPB, USBD_EPINTSTS_RXPKIF_Msk);        
          
            /*
               Host doesn't issue IN Token (read data) during 4ms.
               Audio Record Interface may be closed. Flush EPA Buffer. 
            */
            if(g_usbd_rx_flag > 4)
            {
                if((USBD->EP[EPA].EPDATCNT & 0xFFFF) != 0)
                    USBD->EP[EPA].EPRSPCTL |= USBD_EPRSPCTL_FLUSH_Msk;
            }
            /* 
               Enable EPA IN Token Interrupt for Audio Record (Write data to EPA Buffer) &
               Enable EPA Data Transmitted Interrupts & EPA Empty Buffer for Audio Play (Read data from EPB Buffer)            
            */              
            USBD_ENABLE_EP_INT(EPA, USBD_EPINTEN_INTKIEN_Msk|USBD_EPINTEN_BUFEMPTYIEN_Msk|USBD_EPINTEN_TXPKIEN_Msk);                    
        }
    }
    
    if (IrqStL & USBD_GINTSTS_EPCIF_Msk) {
        IrqSt = USBD->EP[EPC].EPINTSTS & USBD->EP[EPC].EPINTEN;
        USBD_CLR_EP_INT_FLAG(EPC, IrqSt);
        #if defined __HID20__ || defined __HID__
        EPC_Handler();
        #endif
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
