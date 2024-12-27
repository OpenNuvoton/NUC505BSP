/**************************************************************************//**
 * @file     startup_NUC505Series.c
 * @version  V1.00
 * @brief    CMSIS Device Startup File for NuMicro NUC505
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2024 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <inttypes.h>
#include <stdio.h>
#include "NuMicro.h"

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;
extern uint32_t __STACK_LIMIT;

extern __NO_RETURN void __PROGRAM_START(void);

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler(void);
__NO_RETURN void Default_Handler(void);

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
void NMI_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)        __attribute__((weak));
void MemManage_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)         __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void)       __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)         __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)           __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)          __attribute__((weak, alias("Default_Handler")));
    
/* External Interrupts */
void PWR_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));    // 0: Power On Interrupt
void WDT_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));    // 1: Watch Dog Interrupt
void APU_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));    // 2: Audio Process Unit Interrupt
void I2S_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));    // 3: I2S interrupt
void EINT0_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));    // 4: External GPIO Group 0
void EINT1_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));    // 5: External GPIO Group 1
void EINT2_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));    // 6: External GPIO Group 2
void EINT3_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));    // 7: External GPIO Group 3
void SPIM_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 8: SPIM Interrupt
void USBD_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 9: USB Device 2.0 Interrupt
void TMR0_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 10: Timer 0 Interrupt
void TMR1_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 11: Timer 1 Interrupt
void TMR2_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 12: Timer 2 Interrupt
void TMR3_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 13: Timer 3 Interrupt
void SDH_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));    // 14: SD Host Interrupt
void PWM0_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 15: PWM0 Interrupt
void PWM1_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 16: PWM1 Interrupt
void PWM2_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 17: PWM2 Interrupt
void PWM3_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 18: PWM3 Interrupt
void RTC_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));    // 19: Real Time Clock Interrupt
void SPI0_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 20: SPI0 Interrupt
void I2C1_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 21: I2C1 Interrupt
void I2C0_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 22: I2C0 Interrupt
void UART0_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));    // 23: UART0 Interrupt
void UART1_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));    // 24: UART1 Interrupt
void ADC_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));    // 25: ADC Interrupt
void WWDT_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 26: Window Watch Dog Timer Interrupt
void USBH_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 27: USB Host 1.1 Interrupt
void UART2_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));    // 28: UART2 Interrupt
void LVD_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));    // 29: Low Voltage Detection Interrupt
void SPI1_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));    // 30: SPI1 Interrupt

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
#if defined ( __GNUC__ )
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
#endif


extern const VECTOR_TABLE_Type __VECTOR_TABLE[];
const VECTOR_TABLE_Type __VECTOR_TABLE[] __VECTOR_TABLE_ATTRIBUTE =
{
    (VECTOR_TABLE_Type)(&__INITIAL_SP),       /*       Initial Stack Pointer                            */
    Reset_Handler,                            /*       Reset Handler                                    */
    NMI_Handler,                              /*   -14 NMI Handler                                      */
    HardFault_Handler,                        /*   -13 Hard Fault Handler                               */
    MemManage_Handler,                        /*   -12 MPU Fault Handler                                */
    BusFault_Handler,                         /*   -11 Bus Fault Handler                                */
    UsageFault_Handler,                       /*   -10 Usage Fault Handler                              */
    0,                                        /*    -9 Reserved                                         */
    0,                                        /*    -8 Reserved                                         */
    0,                                        /*    -7 Reserved                                         */
    0,                                        /*    -6 Reserved                                         */
    SVC_Handler,                              /*    -5 SVC Handler                                      */
    DebugMon_Handler,                         /*    -4 Reserved                                         */
    0,                                        /*    -3 Reserved                                         */
    PendSV_Handler,                           /*    -2 PendSV Handler Handler                           */
    SysTick_Handler,                          /*    -1 SysTick Handler                                  */

    /* Interrupts */
    PWR_IRQHandler,                           /*    0: Power On                                         */
    WDT_IRQHandler,                           /*    1: Watchdog timer                                   */
    APU_IRQHandler,                           /*    2: Audio Process Unit                               */
    I2S_IRQHandler,                           /*    3: I2S                                              */
    EINT0_IRQHandler,                         /*    4: External Input 0                                 */
    EINT1_IRQHandler,                         /*    5: External Input 1                                 */
    EINT2_IRQHandler,                         /*    6: External Input 2                                 */
    EINT3_IRQHandler,                         /*    7: External Input 3                                 */
    SPIM_IRQHandler,                          /*    8: SPIM                                             */
    USBD_IRQHandler,                          /*    9: USB Device 2.0                                   */
    TMR0_IRQHandler,                          /*    10: Timer 0                                         */
    TMR1_IRQHandler,                          /*    11: Timer 1                                         */
    TMR2_IRQHandler,                          /*    12: Timer 2                                         */
    TMR3_IRQHandler,                          /*    13: Timer 3                                         */
    SDH_IRQHandler,                           /*    14: SD Host                                         */
    PWM0_IRQHandler,                          /*    15: PWM0                                            */
    PWM1_IRQHandler,                          /*    16: PWM1                                            */
    PWM2_IRQHandler,                          /*    17: PWM2                                            */
    PWM3_IRQHandler,                          /*    18: PWM3                                            */
    RTC_IRQHandler,                           /*    19: Real Time Clock                                 */
    SPI0_IRQHandler,                          /*    20: SPI0                                            */
    I2C1_IRQHandler,                          /*    21: I2C1                                            */
    I2C0_IRQHandler,                          /*    22: I2C0                                            */
    UART0_IRQHandler,                         /*    23: UART0                                           */
    UART1_IRQHandler,                         /*    24: UART1                                           */
    ADC_IRQHandler,                           /*    25: ADC                                             */
    WWDT_IRQHandler,                          /*    26: Window watchdog timer                           */
    USBH_IRQHandler,                          /*    27: USB Host 1.1                                    */
    UART2_IRQHandler,                         /*    28: UART2                                           */
    LVD_IRQHandler,                           /*    29: Low Voltage Detection                           */
    SPI1_IRQHandler,                          /*    30: SPI1                                            */
};

#if defined ( __GNUC__ )
    #pragma GCC diagnostic pop
#endif

__WEAK void Reset_Handler_PreInit(void)
{
    // Empty function
}

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler(void)
{
    __set_PSP((uint32_t)(&__INITIAL_SP));
    __set_MSP((uint32_t)(&__STACK_LIMIT));
    __set_PSP((uint32_t)(&__STACK_LIMIT));

    Reset_Handler_PreInit();
    /* Unlock protected registers */
    SYS_UnlockReg();

    SystemInit();               /* CMSIS System Initialization */
    
    /* Lock protected registers */
    SYS_LockReg();

    __PROGRAM_START();          /* Enter PreMain (C library entry point) */
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif

/*---------------------------------------------------------------------------
  Hard Fault Handler
 *---------------------------------------------------------------------------*/
__WEAK void HardFault_Handler(void)
{
    __ASM(
        "MOV     R0, LR  \n"
        "MRS     R1, MSP \n"
        "MRS     R2, PSP \n"
        "LDR     R3, =ProcessHardFault \n"
        "BLX     R3 \n"
        "BX      R0 \n"
    );
}

/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
    while (1);
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
    #pragma clang diagnostic pop
#endif
