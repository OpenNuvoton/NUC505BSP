/****************************************************************************
 * @file     main.c
 * @version  V3.00
 * $Revision: 4 $
 * $Date: 14/06/06 11:24a $
 * @brief    NUC505 Series UART Interface Controller Driver Sample Code
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "NUC505Series.h"


/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
extern char GetChar(void);
int32_t main(void);
void IrDA_FunctionRxTest(void);


/*---------------------------------------------------------------------------------------------------------*/
/*  IrDA Function Receive Test                                                                            */
/*---------------------------------------------------------------------------------------------------------*/
void IrDA_FunctionRxTest()
{
    uint8_t u8InChar = 0xFF;

    printf("\n");
    printf("+-----------------------------------------------------------+\n");
    printf("|     Pin Configure                                         |\n");
    printf("+-----------------------------------------------------------+\n");
    printf("|  ______                                          _______  |\n");
    printf("| |      |                                        |       | |\n");
    printf("| |Master|--UART1_TXD(PB.6) <==> UART1_RXD(PB.7)--|Slave  | |\n");
    printf("| |      |                                        |       | |\n");
    printf("| |______|                                        |_______| |\n");
    printf("|                                                           |\n");
    printf("+-----------------------------------------------------------+\n");

    printf("\n");
    printf("+-----------------------------------------------------------+\n");
    printf("|     IrDA Function Test                                    |\n");
    printf("+-----------------------------------------------------------+\n");
    printf("|  Description :                                            |\n");
    printf("|    The sample code needs two boards. One is Master and    |\n");
    printf("|    the other is slave.  Master will send data based on    |\n");
    printf("|    terminal input and Slave will printf received data on  |\n");
    printf("|    terminal screen.                                       |\n");
    printf("+-----------------------------------------------------------+\n");

    /*
        UART0 is set to debug port and connect with PC firstly.
        The IrDA sample code needs two module board to execute.
        Set the master board is IrDA TX Mode and the other is IrDA Rx mode.
        Inputting char on terminal will be sent to the UART0 of master.
        After the master receiving, the inputting char will send to UART0 again.
        At the same time, it also sends to UART1 TX pin by IrDA mode.
        Slave will print received char after UART1 send out.
        Note that IrDA mode is ONLY used when baud rate equation is selected mode 0.

    */

    printf("\n");
    printf("+-----------------------------------------------------------+\n");
    printf("|     IrDA Function Mode Test (Slave)                       |\n");
    printf("+-----------------------------------------------------------+\n");
    printf("| 1). Polling RDA_Flag to check data input though UART1     |\n");
    printf("| 2). If received data is '0', the program will exit.       |\n");
    printf("|     Otherwise, print received data on terminal            |\n");
    printf("+-----------------------------------------------------------+\n\n");

    /* Set IrDA Rx Mode, Baud Rate configuration must be used MODE0 */
    UART_SelectIrDAMode(UART1, 57600, UART_IRDA_RXEN);

    /* Reset Rx FIFO */
    UART1->FIFO |= UART_FIFO_RXRST_Msk;
    while(UART1->FIFO & UART_FIFO_RXRST_Msk);

    printf("Waiting...\n");

    /* Use polling method to wait master data */
    do {
        if(UART_IS_RX_READY(UART1)) {
            u8InChar = UART_READ(UART1);
            printf("   Input: %c \n", u8InChar);
        }
    } while(u8InChar != '0');

}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable  XTAL */
    CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk;

    CLK_SetCoreClock(96000000);

    /* Set PCLK divider */
    CLK_SetModuleClock(PCLK_MODULE, NULL, 1);

    /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Enable IP clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(UART1_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_UART0_SRC_EXT, 0);
    CLK_SetModuleClock(UART1_MODULE, CLK_UART1_SRC_EXT, 0);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Configure multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB0MFP_Msk) ) | SYS_GPB_MFPL_PB0MFP_UART0_TXD;
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB1MFP_Msk) ) | SYS_GPB_MFPL_PB1MFP_UART0_RXD;

    /* Configure multi-function pins for UART1 RXD and TXD */
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB6MFP_Msk) ) | SYS_GPB_MFPL_PB6MFP_UART1_TXD;
    SYS->GPB_MFPL  = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB7MFP_Msk) ) | SYS_GPB_MFPL_PB7MFP_UART1_RXD;

}

void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART0 module */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 Baudrate */
    UART_Open(UART0, 115200);
}

void UART1_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART1 module */
    SYS_ResetModule(UART1_RST);

    /* Configure UART1 and set UART1 Baudrate */
    UART_Open(UART1, 115200);
}

/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Init UART0 for printf */
    UART0_Init();

    /* Init UART1 for testing */
    UART1_Init();

    /*---------------------------------------------------------------------------------------------------------*/
    /* SAMPLE CODE                                                                                             */
    /*---------------------------------------------------------------------------------------------------------*/

    printf("\n\nCPU @ %dHz\n", SystemCoreClock);

    printf("\nUART Sample Program\n");

    /* UART sample IrDA Slave function */
    IrDA_FunctionRxTest();

    while(1);

}

