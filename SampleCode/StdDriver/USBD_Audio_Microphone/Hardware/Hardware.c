#include <string.h>

#include "NUC505Series.h"

#include "Hardware.h"

#define TIMER_OUT 20   /* Unit : 100 millisecond */

#ifdef __ICCARM__
#pragma data_alignment=4
volatile uint8_t g_timeout = TIMER_OUT;
#else   // __CC_ARM
volatile uint8_t g_timeout __attribute__((aligned(4))) = TIMER_OUT;
#endif

extern uint32_t g_u32count;
extern uint8_t  g_uac_20_flag;
extern uint8_t  g_uac_10_flag;
extern uint8_t  g_start_timer_flag;

#if CONFIG_I2C
typedef void (*_I2C_CALLBACK)(uint32_t u32Status);

#ifdef __ICCARM__
#pragma data_alignment=4
           static          uint8_t  s_au8TxData[4];
#pragma data_alignment=4
           static volatile uint16_t s_u16RxData;
#pragma data_alignment=4
           static volatile uint8_t  s_u8DataLen;
#pragma data_alignment=4
           static volatile uint8_t  s_u8EndFlag;
#else   // __CC_ARM
__align(4) static          uint8_t  s_au8TxData[4];
__align(4) static volatile uint16_t s_u16RxData;
__align(4) static volatile uint8_t  s_u8DataLen;
__align(4) static volatile uint8_t  s_u8EndFlag;
#endif

#if CONFIG_I2C0
static _I2C_CALLBACK s_I2C0HandlerFn;

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 IRQ Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C0_IRQHandler(void)
{
    uint32_t u32Status;
    
    u32Status = I2C_GET_STATUS(I2C0);
    
    if (I2C_GET_TIMEOUT_FLAG(I2C0)) {
        /* Clear I2C0 Timeout Flag */
        I2C_ClearTimeoutFlag(I2C0);
    } else {
        if (s_I2C0HandlerFn != NULL)
            s_I2C0HandlerFn(u32Status);
    }
    
    /* To avoid the synchronization issue between system and APB clock domain */
    u32Status = I2C_GET_STATUS(I2C0);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 Rx Callback Function                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void _I2C0_MasterRx(uint32_t u32Status)
{
    if (u32Status == 0x08) {                    /* START has been transmitted and prepare SLA+W */
        I2C_SET_DATA(I2C0, (DEVICE_ADDRESS << 1)); /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    } else if (u32Status == 0x18) {             /* SLA+W has been transmitted and ACK has been received */
        I2C_SET_DATA(I2C0, s_au8TxData[s_u8DataLen++]);
        I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    } else if (u32Status == 0x20) {             /* SLA+W has been transmitted and NACK has been received */
        I2C_SET_CONTROL_REG(I2C0, I2C_STA | I2C_STO | I2C_SI);
    } else if (u32Status == 0x28) {             /* DATA has been transmitted and ACK has been received */
        #if CONFIG_I2C_RX_2BYTES
        if (s_u8DataLen != 2) {
        #else   /* 1 byte */
        if (s_u8DataLen != 1) {
        #endif
            I2C_SET_DATA(I2C0, s_au8TxData[s_u8DataLen++]);
            I2C_SET_CONTROL_REG(I2C0, I2C_SI);
        } else {
            I2C_SET_CONTROL_REG(I2C0, I2C_STA | I2C_SI);
        }
    } else if (u32Status == 0x10) {             /* Repeat START has been transmitted and prepare SLA+R */
        I2C_SET_DATA(I2C0, (DEVICE_ADDRESS << 1) | 0x01);  /* Write SLA+R to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    } else if (u32Status == 0x40) {             /* SLA+R has been transmitted and ACK has been received */
        I2C_SET_CONTROL_REG(I2C0, I2C_SI | I2C_AA);
    } else if (u32Status == 0x50) {             /* DATA has been received and NACK has been returned */
        s_u16RxData = I2C_GET_DATA(I2C0);
        I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    } else if (u32Status == 0x58) {             /* DATA has been received and NACK has been returned */
        s_u16RxData = ((s_u16RxData << 8) | I2C_GET_DATA(I2C0));
        I2C_SET_CONTROL_REG(I2C0, I2C_STO | I2C_SI);
        s_u8EndFlag = 1;
    } else {
        /* note block here if error */
        printf("I2C0 Rx Status 0x%x is NOT processed\n", u32Status);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 Tx Callback Function                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void _I2C0_MasterTx(uint32_t u32Status)
{
    if (u32Status == 0x08) {                    /* START has been transmitted */
        I2C_SET_DATA(I2C0, (DEVICE_ADDRESS << 1));  /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    } else if (u32Status == 0x18) {             /* SLA+W has been transmitted and ACK has been received */
        I2C_SET_DATA(I2C0, s_au8TxData[s_u8DataLen++]);
        I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    } else if (u32Status == 0x20) {             /* SLA+W has been transmitted and NACK has been received */
        I2C_SET_CONTROL_REG(I2C0, I2C_STA | I2C_STO | I2C_SI);
    } else if (u32Status == 0x28) {             /* DATA has been transmitted and ACK has been received */
        #if CONFIG_I2C_TX_4BYTES
        if (s_u8DataLen != 4) {
        #else   /* 2 bytes */
        if (s_u8DataLen != 2) {
        #endif
            I2C_SET_DATA(I2C0, s_au8TxData[s_u8DataLen++]);
            I2C_SET_CONTROL_REG(I2C0, I2C_SI);
        } else {
            I2C_SET_CONTROL_REG(I2C0, I2C_STO | I2C_SI);
            s_u8EndFlag = 1;
        }
    } else {
        /* note block here if error */
        printf("I2C0 Tx Status 0x%x is NOT processed\n", u32Status);
    }
}
#endif  // CONFIG_I2C0

#if CONFIG_I2C1
static _I2C_CALLBACK s_I2C1HandlerFn;

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C1 IRQ Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C1_IRQHandler(void)
{
    uint32_t u32Status;
    
    u32Status = I2C_GET_STATUS(I2C1);
    
    if (I2C_GET_TIMEOUT_FLAG(I2C1)) {
        /* Clear I2C1 Timeout Flag */
        I2C_ClearTimeoutFlag(I2C1);
    } else {
        if (s_I2C1HandlerFn != NULL)
            s_I2C1HandlerFn(u32Status);
    }
    
    /* To avoid the synchronization issue between system and APB clock domain */
    u32Status = I2C_GET_STATUS(I2C1);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C1 Rx Callback Function                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void _I2C1_MasterRx(uint32_t u32Status)
{
    if (u32Status == 0x08) {                    /* START has been transmitted and prepare SLA+W */
        I2C_SET_DATA(I2C1, (DEVICE_ADDRESS << 1)); /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C1, I2C_SI);
    } else if (u32Status == 0x18) {             /* SLA+W has been transmitted and ACK has been received */
        I2C_SET_DATA(I2C1, s_au8TxData[s_u8DataLen++]);
        I2C_SET_CONTROL_REG(I2C1, I2C_SI);
    } else if (u32Status == 0x20) {             /* SLA+W has been transmitted and NACK has been received */
        I2C_SET_CONTROL_REG(I2C1, I2C_STA | I2C_STO | I2C_SI);
    } else if (u32Status == 0x28) {             /* DATA has been transmitted and ACK has been received */
        #if CONFIG_I2C_RX_2BYTES
        if (s_u8DataLen != 2) {
        #else   /* 1 byte */
        if (s_u8DataLen != 1) {
        #endif
            I2C_SET_DATA(I2C1, s_au8TxData[s_u8DataLen++]);
            I2C_SET_CONTROL_REG(I2C1, I2C_SI);
        } else {
            I2C_SET_CONTROL_REG(I2C1, I2C_STA | I2C_SI);
        }
    } else if (u32Status == 0x10) {             /* Repeat START has been transmitted and prepare SLA+R */
        I2C_SET_DATA(I2C1, (DEVICE_ADDRESS << 1) | 0x01);  /* Write SLA+R to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C1, I2C_SI);
    } else if (u32Status == 0x40) {             /* SLA+R has been transmitted and ACK has been received */
        I2C_SET_CONTROL_REG(I2C1, I2C_SI | I2C_AA);
    } else if (u32Status == 0x50) {             /* DATA has been received and NACK has been returned */
        s_u16RxData = I2C_GET_DATA(I2C1);
        I2C_SET_CONTROL_REG(I2C1, I2C_SI);
    } else if (u32Status == 0x58) {             /* DATA has been received and NACK has been returned */
        s_u16RxData = ((s_u16RxData << 8) | I2C_GET_DATA(I2C1));
        I2C_SET_CONTROL_REG(I2C1, I2C_STO | I2C_SI);
        s_u8EndFlag = 1;
    } else {
        /* note block here if error */
        printf("I2C1 Rx Status 0x%x is NOT processed\n", u32Status);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C1 Tx Callback Function                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void _I2C1_MasterTx(uint32_t u32Status)
{
    if (u32Status == 0x08) {                    /* START has been transmitted */
        I2C_SET_DATA(I2C1, (DEVICE_ADDRESS << 1));  /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C1, I2C_SI);
    } else if (u32Status == 0x18) {             /* SLA+W has been transmitted and ACK has been received */
        I2C_SET_DATA(I2C1, s_au8TxData[s_u8DataLen++]);
        I2C_SET_CONTROL_REG(I2C1, I2C_SI);
    } else if (u32Status == 0x20) {             /* SLA+W has been transmitted and NACK has been received */
        I2C_SET_CONTROL_REG(I2C1, I2C_STA | I2C_STO | I2C_SI);
    } else if (u32Status == 0x28) {             /* DATA has been transmitted and ACK has been received */
        #if CONFIG_I2C_TX_4BYTES
        if (s_u8DataLen != 4) {
        #else   /* 2 bytes */
        if (s_u8DataLen != 2) {
        #endif
            I2C_SET_DATA(I2C1, s_au8TxData[s_u8DataLen++]);
            I2C_SET_CONTROL_REG(I2C1, I2C_SI);
        } else {
            I2C_SET_CONTROL_REG(I2C1, I2C_STO | I2C_SI);
            s_u8EndFlag = 1;
        }
    } else {
        /* note block here if error */
        printf("I2C1 Tx Status 0x%x is NOT processed\n", u32Status);
    }
}
#endif  // CONFIG_I2C1

void _I2C_SetTxCallback(void)
{
    #if CONFIG_I2C0
    /* I2C function to write data to slave */
    s_I2C0HandlerFn = (_I2C_CALLBACK)_I2C0_MasterTx;
    #endif  // CONFIG_I2C0
    
    #if CONFIG_I2C1
    /* I2C function to write data to slave */
    s_I2C1HandlerFn = (_I2C_CALLBACK)_I2C1_MasterTx;
    #endif  // CONFIG_I2C1
}

void _I2C_SetRxCallback(void)
{
    #if CONFIG_I2C0
    s_I2C0HandlerFn = (_I2C_CALLBACK)_I2C0_MasterRx;
    #endif  // CONFIG_I2C0
    
    #if CONFIG_I2C1
    s_I2C1HandlerFn = (_I2C_CALLBACK)_I2C1_MasterRx;
    #endif  // CONFIG_I2C1
}

void _I2C_WriteData(uint16_t addr, uint16_t data)
{
    #if CONFIG_I2C_TX_4BYTES
    s_au8TxData[0] = (uint8_t)((addr >> 8) & 0x00FF);   //addr [15:8]
    s_au8TxData[1] = (uint8_t)(addr & 0x00FF);          //addr [ 7:0]
    s_au8TxData[2] = (uint8_t)((data >> 8) & 0x00FF);   //data [15:8]
    s_au8TxData[3] = (uint8_t)(data & 0x00FF);          //data [ 7:0]
    #else   /* 2 bytes */
    s_au8TxData[0] = (uint8_t)((addr << 1)  | (data >> 8));     //addr(7bit) + data(first bit)
    s_au8TxData[1] = (uint8_t)(data & 0x00FF);                  //data(8bit)
    #endif
    
    s_u8DataLen = 0;
    s_u8EndFlag = 0;
    
    #if CONFIG_I2C0
    /* I2C as master sends START signal */
    I2C_SET_CONTROL_REG(I2C0, I2C_STA);
    #endif  // CONFIG_I2C0
    
    #if CONFIG_I2C1
    /* I2C as master sends START signal */
    I2C_SET_CONTROL_REG(I2C1, I2C_STA);
    #endif  // CONFIG_I2C1
    
    /* Wait I2C Tx Finish */
    while ( s_u8EndFlag == 0 );
    s_u8EndFlag = 0;
}

void _I2C_ReadData(uint16_t addr)
{
    #if CONFIG_I2C_TX_4BYTES
    s_au8TxData[0] = (uint8_t)((addr >> 8) & 0x00FF);   //addr [15:8]
    s_au8TxData[1] = (uint8_t)(addr & 0x00FF);          //addr [ 7:0]
    #else   /* 2 bytes */
    s_au8TxData[0] = (uint8_t)(addr << 1);   //addr(7bit) + 0 (lsb)
    #endif
    
    s_u8DataLen = 0;
    s_u8EndFlag = 0;
    
    #if CONFIG_I2C0
    /* I2C as master sends START signal */
    I2C_SET_CONTROL_REG(I2C0, I2C_STA);
    #endif  // CONFIG_I2C0
    
    #if CONFIG_I2C1
    /* I2C as master sends START signal */
    I2C_SET_CONTROL_REG(I2C1, I2C_STA);
    #endif  // CONFIG_I2C1
    
    /* Wait I2C Rx Finish */
    while ( s_u8EndFlag == 0 );
    s_u8EndFlag = 0;
    printf("%04X\n", s_u16RxData);
}
#endif  // CONFIG_I2C

#ifdef __ICCARM__
#pragma data_alignment=4
volatile uint32_t g_u32TimerCnt;
#else   // __CC_ARM
volatile uint32_t g_u32TimerCnt __attribute__((aligned(4)));
#endif

void TMR0_IRQHandler(void)
{
    /* clear timer interrupt flag */
    TIMER_ClearIntFlag(TIMER0);
    
    /* To avoid the synchronization issue between system and APB clock domain */
    TIMER_GetIntFlag(TIMER0);
	
    if(g_start_timer_flag)
    {			
        if(g_u32count > TIMER_OUT)
        {
            if(g_uac_20_flag == 0)
            {
                if(g_uac_10_flag == 0)    /* Try UAC 2.0 mode */
                {
                    g_uac_10_flag = 1;    /* Try UAC 1.0 mode now */
                    printf("  UAC2.0 USB Bus Enumeration Fail!!\n");
                }
                g_u32count++;             /* For the delay before UAC 1.0 */
            }
        }
        else
            g_u32count++;
    }				
    
    g_u32TimerCnt ++;
}

void SysTick_Handler(void)
{
    
}

void Hardware_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    
    /* Init System, IP clock and multi-function I/O */
    
    /* Unlock protected registers */
    //SYS_UnlockReg();
    
    /* Enable  XTAL */
    CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk;
    
    /* PCLK divider = 1 (/2) */
    CLK_SetModuleClock(PCLK_MODULE, 0, 1);
    
    /* Update System Core Clock */
    /* Note too high system clock will cause over-spec of SPI Flash read command on running code on SPI Flash. */
    CLK_SetCoreClock(100000000);
    SystemCoreClockUpdate();
    
    /* Lock protected registers */
    //SYS_LockReg();
    
    /* Enable USB IP clock */
    CLK_EnableModuleClock(USBD_MODULE);
    
    /* Select USB IP clock source */
    CLK_SetModuleClock(USBD_MODULE, CLK_USBD_SRC_EXT, 0);
    
    #if CONFIG_UART0
    /* Note default debug port is UART0 defined in retarget.c, change to other UART port if needed. */
    /* Enable UART0 Module clock */
    CLK_EnableModuleClock(UART0_MODULE);
    
    /* UART0 module clock from EXT */
    CLK_SetModuleClock(UART0_MODULE, CLK_UART0_SRC_EXT, 0);
    
    /* Reset IP */
    SYS_ResetModule(UART0_RST);
    
    /* Configure UART0 and set UART0 Baudrate */
    UART_Open(UART0, 115200);
    
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Configure multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB0MFP_Msk) ) | SYS_GPB_MFPL_PB0MFP_UART0_TXD;
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB1MFP_Msk) ) | SYS_GPB_MFPL_PB1MFP_UART0_RXD;
    #endif  // CONFIG_UART0
    
    #if CONFIG_UART1
    /* Enable UART1 Module clock */
    CLK_EnableModuleClock(UART1_MODULE);
    
    /* UART1 module clock from EXT */
    CLK_SetModuleClock(UART1_MODULE, CLK_UART1_SRC_EXT, 0);
    
    /* Reset IP */
    SYS_ResetModule(UART1_RST);
    
    /* Configure UART1 and set UART1 Baudrate */
    UART_Open(UART1, 115200);
    
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Configure multi-function pins for UART1 RXD and TXD */
    SYS->GPA_MFPH  = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA8MFP_Msk) ) | SYS_GPA_MFPH_PA8MFP_UART1_TXD;
    SYS->GPA_MFPH  = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA9MFP_Msk) ) | SYS_GPA_MFPH_PA9MFP_UART1_RXD;
    #endif  // CONFIG_UART1
    
    #if CONFIG_VECTOR
    /* Relocate vector table in SRAM for fast interrupt handling. */
    {
    #if defined ( __CC_ARM )
        extern uint32_t __Vectors[];
        extern uint32_t __Vectors_Size[];
        extern uint32_t Image$$ER_VECTOR2$$ZI$$Base[];
        
        //printf("Relocate vector table in SRAM (0x%08X) for fast interrupt handling.\n", Image$$ER_VECTOR2$$ZI$$Base);
        memcpy((void *) Image$$ER_VECTOR2$$ZI$$Base, (void *) __Vectors, (unsigned int) __Vectors_Size);
        SCB->VTOR = (uint32_t) Image$$ER_VECTOR2$$ZI$$Base;
        
    #elif defined (__ICCARM__)
        #pragma section = "VECTOR2"
        extern uint32_t __Vectors[];
        extern uint32_t __Vectors_Size[];
        
        //printf("Relocate vector table in SRAM (0x%08X) for fast interrupt handling.\n", __section_begin("VECTOR2"));
        memcpy((void *) __section_begin("VECTOR2"), (void *) __Vectors, (unsigned int) __Vectors_Size);
        SCB->VTOR = (uint32_t) __section_begin("VECTOR2");
    #endif
    }
    
    //printf("Main code is now running on SRAM from SPI Flash for fast execution.\n");
    #endif  // CONFIG_VECTOR
    
    #if CONFIG_I2C0
    /* Enable I2C0 Module clock */
    CLK_EnableModuleClock(I2C0_MODULE);
    /* Reset IP */
    SYS_ResetModule(I2C0_RST);
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPA14,GPA15 multi-function pins for I2C0 */
    SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA14MFP_Msk) ) | SYS_GPA_MFPH_PA14MFP_I2C0_SCL;
    SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA15MFP_Msk) ) | SYS_GPA_MFPH_PA15MFP_I2C0_SDA;
    
    s_I2C0HandlerFn = NULL;
    
    /* Open I2C0 and set clock to 100k */
    I2C_Open(I2C0, 100000);
    
    /* Get I2C0 Bus Clock */
    printf("I2C0 clock %d Hz\n", I2C_GetBusClockFreq(I2C0));
    
    I2C_EnableInt(I2C0);
    NVIC_EnableIRQ(I2C0_IRQn);
    #endif  // CONFIG_I2C0
    
    #if CONFIG_I2C1
    /* Enable I2C1 Module clock */
    CLK_EnableModuleClock(I2C1_MODULE);
    /* Reset IP */
    SYS_ResetModule(I2C1_RST);
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPA10,GPA11 multi-function pins for I2C1 */
    SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA10MFP_Msk) ) | SYS_GPA_MFPH_PA10MFP_I2C1_SCL;
    SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA11MFP_Msk) ) | SYS_GPA_MFPH_PA11MFP_I2C1_SDA;
    
    s_I2C1HandlerFn = NULL;
    
    /* Open I2C1 and set clock to 100k */
    I2C_Open(I2C1, 100000);
    
    /* Get I2C1 Bus Clock */
    printf("I2C1 clock %d Hz\n", I2C_GetBusClockFreq(I2C1));
    
    I2C_EnableInt(I2C1);
    NVIC_EnableIRQ(I2C1_IRQn);
    #endif  // CONFIG_I2C1
    
    CLK_EnableModuleClock(TMR0_MODULE);
    
    CLK_SetModuleClock(TMR0_MODULE, CLK_TMR0_SRC_EXT, 0);
    
    /* freq. 10 means: 10 counts per 1 second */
    /* so the period is 100ms means: 1 count per 100 millisecond */
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 10);
    
    TIMER_EnableInt(TIMER0);
    
    NVIC_EnableIRQ(TMR0_IRQn);
    
    TIMER_Start(TIMER0);
}
