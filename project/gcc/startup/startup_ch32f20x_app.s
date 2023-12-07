/********************************** (C) COPYRIGHT *******************************
* File Name          : startup_ch32f20x.s
* Author             : SecondHandCoder
* Version            : V1.0.0
* Date               : 2023/11/18
* Description        : CH32F20x vector table for GCC toolchain.
*******************************************************************************/
    
    .syntax unified
    .cpu cortex-m3
    .thumb

/*******************************************************************************
 Vector Table Mapped to Address 0 at Reset.
*******************************************************************************/
    .section  .vectors,"a",%progbits
    .type  __isr_vector, %object
    .global  __isr_vector
__isr_vector:
    .word  __StackTop
    .word  Reset_Handler
    .word  NMI_Handler
    .word  HardFault_Handler
    .word  MemManage_Handler
    .word  BusFault_Handler
    .word  UsageFault_Handler
    .word  0
    .word  0
    .word  0
    .word  0
    .word  SVC_Handler
    .word  DebugMon_Handler
    .word  0
    .word  PendSV_Handler
    .word  SysTick_Handler

/*******************************************************************************
 External Interrupts
*******************************************************************************/
    .word  WWDG_IRQHandler
    .word  PVD_IRQHandler
    .word  TAMPER_IRQHandler
    .word  RTC_IRQHandler
    .word  FLASH_IRQHandler
    .word  RCC_IRQHandler
    .word  EXTI0_IRQHandler
    .word  EXTI1_IRQHandler
    .word  EXTI2_IRQHandler
    .word  EXTI3_IRQHandler
    .word  EXTI4_IRQHandler
    .word  DMA1_Channel1_IRQHandler
    .word  DMA1_Channel2_IRQHandler
    .word  DMA1_Channel3_IRQHandler
    .word  DMA1_Channel4_IRQHandler
    .word  DMA1_Channel5_IRQHandler
    .word  DMA1_Channel6_IRQHandler
    .word  DMA1_Channel7_IRQHandler
    .word  ADC1_2_IRQHandler
    .word  USB_HP_CAN1_TX_IRQHandler
    .word  USB_LP_CAN1_RX0_IRQHandler
    .word  CAN1_RX1_IRQHandler
    .word  CAN1_SCE_IRQHandler
    .word  EXTI9_5_IRQHandler
    .word  TIM1_BRK_IRQHandler
    .word  TIM1_UP_IRQHandler
    .word  TIM1_TRG_COM_IRQHandler
    .word  TIM1_CC_IRQHandler
    .word  TIM2_IRQHandler
    .word  TIM3_IRQHandler
    .word  TIM4_IRQHandler
    .word  I2C1_EV_IRQHandler
    .word  I2C1_ER_IRQHandler
    .word  I2C2_EV_IRQHandler
    .word  I2C2_ER_IRQHandler
    .word  SPI1_IRQHandler
    .word  SPI2_IRQHandler
    .word  USART1_IRQHandler
    .word  USART2_IRQHandler
    .word  USART3_IRQHandler
    .word  EXTI15_10_IRQHandler
    .word  RTCAlarm_IRQHandler
    .word  USBWakeUp_IRQHandler
    .word  TIM8_BRK_IRQHandler
    .word  TIM8_UP_IRQHandler
    .word  TIM8_TRG_COM_IRQHandler
    .word  TIM8_CC_IRQHandler
    .word  RNG_IRQHandler
    .word  FSMC_IRQHandler
    .word  SDIO_IRQHandler
    .word  TIM5_IRQHandler
    .word  SPI3_IRQHandler
    .word  UART4_IRQHandler
    .word  UART5_IRQHandler
    .word  TIM6_IRQHandler
    .word  TIM7_IRQHandler
    .word  DMA2_Channel1_IRQHandler
    .word  DMA2_Channel2_IRQHandler
    .word  DMA2_Channel3_IRQHandler
    .word  DMA2_Channel4_IRQHandler
    .word  DMA2_Channel5_IRQHandler
    .word  ETH_IRQHandler
    .word  ETH_WKUP_IRQHandler
    .word  CAN2_TX_IRQHandler
    .word  CAN2_RX0_IRQHandler
    .word  CAN2_RX1_IRQHandler
    .word  CAN2_SCE_IRQHandler
    .word  OTG_FS_IRQHandler
    .word  USBHSWakeup_IRQHandler
    .word  USBHS_IRQHandler
    .word  DVP_IRQHandler
    .word  UART6_IRQHandler
    .word  UART7_IRQHandler
    .word  UART8_IRQHandler
    .word  TIM9_BRK_IRQHandler
    .word  TIM9_UP_IRQHandler
    .word  TIM9_TRG_COM_IRQHandler
    .word  TIM9_CC_IRQHandler
    .word  TIM10_BRK_IRQHandler
    .word  TIM10_UP_IRQHandler
    .word  TIM10_TRG_COM_IRQHandler
    .word  TIM10_CC_IRQHandler
    .word  DMA2_Channel6_IRQHandler
    .word  DMA2_Channel7_IRQHandler
    .word  DMA2_Channel8_IRQHandler
    .word  DMA2_Channel9_IRQHandler
    .word  DMA2_Channel10_IRQHandler
    .word  DMA2_Channel11_IRQHandler

    .size    __isr_vector, . - __isr_vector

/* Reset Handler */
    .thumb_func
    .align 2
    .globl   Reset_Handler
    .weak    Reset_Handler
    .type    Reset_Handler, %function
Reset_Handler:
    cpsid   i

    ldr     r0,=__StackTop
    mov     r13,r0

    ldr     r0,=init_data_bss
    blx     r0

    ldr     r0,=SystemInit
    blx     r0

    cpsie   i               
    bl      __main

    .pool
    .size Reset_Handler, . - Reset_Handler

/*******************************************************************************
 Dummy Exception Handlers (infinite loops which can be modified)
*******************************************************************************/
.align  1
    .thumb_func
    .weak DefaultISR
    .type DefaultISR, %function
DefaultISR:
    b       DefaultISR
    .size DefaultISR, . - DefaultISR

    .macro def_irq_handler	handler_name
    .weak \handler_name
    .set  \handler_name, DefaultISR
    .endm

/* Exception Handlers */
    def_irq_handler   NMI_Handler
    def_irq_handler   HardFault_Handler
    def_irq_handler   MemManage_Handler
    def_irq_handler   BusFault_Handler
    def_irq_handler   UsageFault_Handler
    def_irq_handler   SVC_Handler
    def_irq_handler   DebugMon_Handler
    def_irq_handler   PendSV_Handler
    def_irq_handler   SysTick_Handler
    def_irq_handler   WWDG_IRQHandler
    def_irq_handler   PVD_IRQHandler
    def_irq_handler   TAMPER_IRQHandler
    def_irq_handler   RTC_IRQHandler
    def_irq_handler   FLASH_IRQHandler
    def_irq_handler   RCC_IRQHandler
    def_irq_handler   EXTI0_IRQHandler
    def_irq_handler   EXTI1_IRQHandler
    def_irq_handler   EXTI2_IRQHandler
    def_irq_handler   EXTI3_IRQHandler
    def_irq_handler   EXTI4_IRQHandler
    def_irq_handler   DMA1_Channel1_IRQHandler
    def_irq_handler   DMA1_Channel2_IRQHandler
    def_irq_handler   DMA1_Channel3_IRQHandler
    def_irq_handler   DMA1_Channel4_IRQHandler
    def_irq_handler   DMA1_Channel5_IRQHandler
    def_irq_handler   DMA1_Channel6_IRQHandler
    def_irq_handler   DMA1_Channel7_IRQHandler
    def_irq_handler   ADC1_2_IRQHandler
    def_irq_handler   USB_HP_CAN1_TX_IRQHandler
    def_irq_handler   USB_LP_CAN1_RX0_IRQHandler
    def_irq_handler   CAN1_RX1_IRQHandler
    def_irq_handler   CAN1_SCE_IRQHandler
    def_irq_handler   EXTI9_5_IRQHandler
    def_irq_handler   TIM1_BRK_IRQHandler
    def_irq_handler   TIM1_UP_IRQHandler
    def_irq_handler   TIM1_TRG_COM_IRQHandler
    def_irq_handler   TIM1_CC_IRQHandler
    def_irq_handler   TIM2_IRQHandler
    def_irq_handler   TIM3_IRQHandler
    def_irq_handler   TIM4_IRQHandler
    def_irq_handler   I2C1_EV_IRQHandler
    def_irq_handler   I2C1_ER_IRQHandler
    def_irq_handler   I2C2_EV_IRQHandler
    def_irq_handler   I2C2_ER_IRQHandler
    def_irq_handler   SPI1_IRQHandler
    def_irq_handler   SPI2_IRQHandler
    def_irq_handler   USART1_IRQHandler
    def_irq_handler   USART2_IRQHandler
    def_irq_handler   USART3_IRQHandler
    def_irq_handler   EXTI15_10_IRQHandler
    def_irq_handler   RTCAlarm_IRQHandler
    def_irq_handler   USBWakeUp_IRQHandler
    def_irq_handler   TIM8_BRK_IRQHandler
    def_irq_handler   TIM8_UP_IRQHandler
    def_irq_handler   TIM8_TRG_COM_IRQHandler
    def_irq_handler   TIM8_CC_IRQHandler
    def_irq_handler   RNG_IRQHandler
    def_irq_handler   FSMC_IRQHandler
    def_irq_handler   SDIO_IRQHandler
    def_irq_handler   TIM5_IRQHandler
    def_irq_handler   SPI3_IRQHandler
    def_irq_handler   UART4_IRQHandler
    def_irq_handler   UART5_IRQHandler
    def_irq_handler   TIM6_IRQHandler
    def_irq_handler   TIM7_IRQHandler
    def_irq_handler   DMA2_Channel1_IRQHandler
    def_irq_handler   DMA2_Channel2_IRQHandler
    def_irq_handler   DMA2_Channel3_IRQHandler
    def_irq_handler   DMA2_Channel4_IRQHandler
    def_irq_handler   DMA2_Channel5_IRQHandler
    def_irq_handler   ETH_IRQHandler
    def_irq_handler   ETH_WKUP_IRQHandler
    def_irq_handler   CAN2_TX_IRQHandler
    def_irq_handler   CAN2_RX0_IRQHandler
    def_irq_handler   CAN2_RX1_IRQHandler
    def_irq_handler   CAN2_SCE_IRQHandler
    def_irq_handler   OTG_FS_IRQHandler
    def_irq_handler   USBHSWakeup_IRQHandler
    def_irq_handler   USBHS_IRQHandler
    def_irq_handler   DVP_IRQHandler
    def_irq_handler   UART6_IRQHandler
    def_irq_handler   UART7_IRQHandler
    def_irq_handler   UART8_IRQHandler
    def_irq_handler   TIM9_BRK_IRQHandler
    def_irq_handler   TIM9_UP_IRQHandler
    def_irq_handler   TIM9_TRG_COM_IRQHandler
    def_irq_handler   TIM9_CC_IRQHandler
    def_irq_handler   TIM10_BRK_IRQHandler
    def_irq_handler   TIM10_UP_IRQHandler
    def_irq_handler   TIM10_TRG_COM_IRQHandler
    def_irq_handler   TIM10_CC_IRQHandler
    def_irq_handler   DMA2_Channel6_IRQHandler
    def_irq_handler   DMA2_Channel7_IRQHandler
    def_irq_handler   DMA2_Channel8_IRQHandler
    def_irq_handler   DMA2_Channel9_IRQHandler
    def_irq_handler   DMA2_Channel10_IRQHandler
    def_irq_handler   DMA2_Channel11_IRQHandler

    .end
