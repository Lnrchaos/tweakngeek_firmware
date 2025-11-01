/**
 * @file startup_stm32wb55.s
 * @brief STM32WB55 Startup Code
 * 
 * This file contains the startup code for STM32WB55 including
 * vector table, reset handler, and early initialization.
 */

.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

/* Memory layout definitions */
.equ STACK_SIZE, 0x2000        /* 8KB stack */
.equ HEAP_SIZE,  0x8000        /* 32KB heap */

/* Vector table */
.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object
.size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
    .word _estack                           /* Top of Stack */
    .word Reset_Handler                     /* Reset Handler */
    .word NMI_Handler                       /* NMI Handler */
    .word HardFault_Handler                 /* Hard Fault Handler */
    .word MemManage_Handler                 /* MPU Fault Handler */
    .word BusFault_Handler                  /* Bus Fault Handler */
    .word UsageFault_Handler                /* Usage Fault Handler */
    .word 0                                 /* Reserved */
    .word 0                                 /* Reserved */
    .word 0                                 /* Reserved */
    .word 0                                 /* Reserved */
    .word SVC_Handler                       /* SVCall Handler */
    .word DebugMon_Handler                  /* Debug Monitor Handler */
    .word 0                                 /* Reserved */
    .word PendSV_Handler                    /* PendSV Handler */
    .word SysTick_Handler                   /* SysTick Handler */
    
    /* External Interrupts */
    .word WWDG_IRQHandler                   /* Window WatchDog */
    .word PVD_PVM_IRQHandler                /* PVD/PVM through EXTI */
    .word TAMP_STAMP_LSECSS_IRQHandler      /* Tamper and TimeStamps */
    .word RTC_WKUP_IRQHandler               /* RTC Wakeup */
    .word FLASH_IRQHandler                  /* FLASH */
    .word RCC_IRQHandler                    /* RCC */
    .word EXTI0_IRQHandler                  /* EXTI Line0 */
    .word EXTI1_IRQHandler                  /* EXTI Line1 */
    .word EXTI2_IRQHandler                  /* EXTI Line2 */
    .word EXTI3_IRQHandler                  /* EXTI Line3 */
    .word EXTI4_IRQHandler                  /* EXTI Line4 */
    .word DMA1_Channel1_IRQHandler          /* DMA1 Channel 1 */
    .word DMA1_Channel2_IRQHandler          /* DMA1 Channel 2 */
    .word DMA1_Channel3_IRQHandler          /* DMA1 Channel 3 */
    .word DMA1_Channel4_IRQHandler          /* DMA1 Channel 4 */
    .word DMA1_Channel5_IRQHandler          /* DMA1 Channel 5 */
    .word DMA1_Channel6_IRQHandler          /* DMA1 Channel 6 */
    .word DMA1_Channel7_IRQHandler          /* DMA1 Channel 7 */
    .word ADC1_IRQHandler                   /* ADC1 */
    .word USB_HP_IRQHandler                 /* USB HP */
    .word USB_LP_IRQHandler                 /* USB LP */
    .word C2SEV_PWR_C2H_IRQHandler          /* CPU2 SEV */
    .word COMP_IRQHandler                   /* COMP1 and COMP2 */
    .word EXTI9_5_IRQHandler                /* External Line[9:5] */
    .word TIM1_BRK_IRQHandler               /* TIM1 Break */
    .word TIM1_UP_TIM16_IRQHandler          /* TIM1 Update and TIM16 */
    .word TIM1_TRG_COM_TIM17_IRQHandler     /* TIM1 Trigger and TIM17 */
    .word TIM1_CC_IRQHandler                /* TIM1 Capture Compare */
    .word TIM2_IRQHandler                   /* TIM2 */
    .word PKA_IRQHandler                    /* PKA */
    .word I2C1_EV_IRQHandler                /* I2C1 Event */
    .word I2C1_ER_IRQHandler                /* I2C1 Error */
    .word I2C3_EV_IRQHandler                /* I2C3 Event */
    .word I2C3_ER_IRQHandler                /* I2C3 Error */
    .word SPI1_IRQHandler                   /* SPI1 */
    .word SPI2_IRQHandler                   /* SPI2 */
    .word USART1_IRQHandler                 /* USART1 */
    .word LPUART1_IRQHandler                /* LPUART1 */
    .word SAI1_IRQHandler                   /* SAI1 */
    .word TSC_IRQHandler                    /* TSC */
    .word EXTI15_10_IRQHandler              /* External Line[15:10] */
    .word RTC_Alarm_IRQHandler              /* RTC Alarm */
    .word CRS_IRQHandler                    /* CRS */
    .word PWR_SOTF_BLEACT_802ACT_RFPHASE_IRQHandler /* PWR */
    .word IPCC_C1_RX_IRQHandler             /* IPCC CPU1 RX */
    .word IPCC_C1_TX_IRQHandler             /* IPCC CPU1 TX */
    .word HSEM_IRQHandler                   /* HSEM */
    .word LPTIM1_IRQHandler                 /* LPTIM1 */
    .word LPTIM2_IRQHandler                 /* LPTIM2 */
    .word LCD_IRQHandler                    /* LCD */
    .word QUADSPI_IRQHandler                /* QUADSPI */
    .word AES1_IRQHandler                   /* AES1 */
    .word AES2_IRQHandler                   /* AES2 */
    .word RNG_IRQHandler                    /* RNG */
    .word FPU_IRQHandler                    /* FPU */
    .word DMA2_Channel1_IRQHandler          /* DMA2 Channel 1 */
    .word DMA2_Channel2_IRQHandler          /* DMA2 Channel 2 */
    .word DMA2_Channel3_IRQHandler          /* DMA2 Channel 3 */
    .word DMA2_Channel4_IRQHandler          /* DMA2 Channel 4 */
    .word DMA2_Channel5_IRQHandler          /* DMA2 Channel 5 */
    .word DMA2_Channel6_IRQHandler          /* DMA2 Channel 6 */
    .word DMA2_Channel7_IRQHandler          /* DMA2 Channel 7 */
    .word DMAMUX1_OVR_IRQHandler            /* DMAMUX1 overrun */

/* Reset handler */
.section .text.Reset_Handler
.weak Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
    ldr r0, =_estack
    mov sp, r0          /* Set stack pointer */

    /* Copy the data segment initializers from flash to SRAM */
    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sidata
    movs r3, #0
    b LoopCopyDataInit

CopyDataInit:
    ldr r4, [r2, r3]
    str r4, [r0, r3]
    adds r3, r3, #4

LoopCopyDataInit:
    adds r4, r0, r3
    cmp r4, r1
    bcc CopyDataInit

    /* Zero fill the bss segment */
    ldr r2, =_sbss
    ldr r4, =_ebss
    movs r3, #0
    b LoopFillZerobss

FillZerobss:
    str r3, [r2]
    adds r2, r2, #4

LoopFillZerobss:
    cmp r2, r4
    bcc FillZerobss

    /* Enable FPU if available */
    ldr r0, =0xE000ED88    /* CPACR */
    ldr r1, [r0]
    orr r1, r1, #(0xF << 20)  /* Enable CP10 and CP11 */
    str r1, [r0]
    dsb
    isb

    /* Call system initialization function */
    bl SystemInit

    /* Call main function */
    bl main

    /* Infinite loop if main returns */
LoopForever:
    b LoopForever

.size Reset_Handler, .-Reset_Handler

/* System initialization function */
.section .text.SystemInit
.weak SystemInit
.type SystemInit, %function
SystemInit:
    bx lr
.size SystemInit, .-SystemInit

/* Default interrupt handlers */
.section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
    b Infinite_Loop
.size Default_Handler, .-Default_Handler

/* SysTick handler - calls kernel tick handler */
.section .text.SysTick_Handler
.weak SysTick_Handler
.type SysTick_Handler, %function
SysTick_Handler:
    push {lr}
    bl kernel_tick_handler
    pop {lr}
    bx lr
.size SysTick_Handler, .-SysTick_Handler

/* Weak aliases for interrupt handlers */
.weak NMI_Handler
.thumb_set NMI_Handler,Default_Handler

.weak HardFault_Handler
.thumb_set HardFault_Handler,Default_Handler

.weak MemManage_Handler
.thumb_set MemManage_Handler,Default_Handler

.weak BusFault_Handler
.thumb_set BusFault_Handler,Default_Handler

.weak UsageFault_Handler
.thumb_set UsageFault_Handler,Default_Handler

.weak SVC_Handler
.thumb_set SVC_Handler,Default_Handler

.weak DebugMon_Handler
.thumb_set DebugMon_Handler,Default_Handler

.weak PendSV_Handler
.thumb_set PendSV_Handler,Default_Handler

/* External interrupt handlers */
.weak WWDG_IRQHandler
.thumb_set WWDG_IRQHandler,Default_Handler

.weak PVD_PVM_IRQHandler
.thumb_set PVD_PVM_IRQHandler,Default_Handler

.weak TAMP_STAMP_LSECSS_IRQHandler
.thumb_set TAMP_STAMP_LSECSS_IRQHandler,Default_Handler

.weak RTC_WKUP_IRQHandler
.thumb_set RTC_WKUP_IRQHandler,Default_Handler

.weak FLASH_IRQHandler
.thumb_set FLASH_IRQHandler,Default_Handler

.weak RCC_IRQHandler
.thumb_set RCC_IRQHandler,Default_Handler

.weak EXTI0_IRQHandler
.thumb_set EXTI0_IRQHandler,Default_Handler

.weak EXTI1_IRQHandler
.thumb_set EXTI1_IRQHandler,Default_Handler

.weak EXTI2_IRQHandler
.thumb_set EXTI2_IRQHandler,Default_Handler

.weak EXTI3_IRQHandler
.thumb_set EXTI3_IRQHandler,Default_Handler

.weak EXTI4_IRQHandler
.thumb_set EXTI4_IRQHandler,Default_Handler

.weak DMA1_Channel1_IRQHandler
.thumb_set DMA1_Channel1_IRQHandler,Default_Handler

.weak DMA1_Channel2_IRQHandler
.thumb_set DMA1_Channel2_IRQHandler,Default_Handler

.weak DMA1_Channel3_IRQHandler
.thumb_set DMA1_Channel3_IRQHandler,Default_Handler

.weak DMA1_Channel4_IRQHandler
.thumb_set DMA1_Channel4_IRQHandler,Default_Handler

.weak DMA1_Channel5_IRQHandler
.thumb_set DMA1_Channel5_IRQHandler,Default_Handler

.weak DMA1_Channel6_IRQHandler
.thumb_set DMA1_Channel6_IRQHandler,Default_Handler

.weak DMA1_Channel7_IRQHandler
.thumb_set DMA1_Channel7_IRQHandler,Default_Handler

.weak ADC1_IRQHandler
.thumb_set ADC1_IRQHandler,Default_Handler

.weak USB_HP_IRQHandler
.thumb_set USB_HP_IRQHandler,Default_Handler

.weak USB_LP_IRQHandler
.thumb_set USB_LP_IRQHandler,Default_Handler

.weak C2SEV_PWR_C2H_IRQHandler
.thumb_set C2SEV_PWR_C2H_IRQHandler,Default_Handler

.weak COMP_IRQHandler
.thumb_set COMP_IRQHandler,Default_Handler

.weak EXTI9_5_IRQHandler
.thumb_set EXTI9_5_IRQHandler,Default_Handler

.weak TIM1_BRK_IRQHandler
.thumb_set TIM1_BRK_IRQHandler,Default_Handler

.weak TIM1_UP_TIM16_IRQHandler
.thumb_set TIM1_UP_TIM16_IRQHandler,Default_Handler

.weak TIM1_TRG_COM_TIM17_IRQHandler
.thumb_set TIM1_TRG_COM_TIM17_IRQHandler,Default_Handler

.weak TIM1_CC_IRQHandler
.thumb_set TIM1_CC_IRQHandler,Default_Handler

.weak TIM2_IRQHandler
.thumb_set TIM2_IRQHandler,Default_Handler

.weak PKA_IRQHandler
.thumb_set PKA_IRQHandler,Default_Handler

.weak I2C1_EV_IRQHandler
.thumb_set I2C1_EV_IRQHandler,Default_Handler

.weak I2C1_ER_IRQHandler
.thumb_set I2C1_ER_IRQHandler,Default_Handler

.weak I2C3_EV_IRQHandler
.thumb_set I2C3_EV_IRQHandler,Default_Handler

.weak I2C3_ER_IRQHandler
.thumb_set I2C3_ER_IRQHandler,Default_Handler

.weak SPI1_IRQHandler
.thumb_set SPI1_IRQHandler,Default_Handler

.weak SPI2_IRQHandler
.thumb_set SPI2_IRQHandler,Default_Handler

.weak USART1_IRQHandler
.thumb_set USART1_IRQHandler,Default_Handler

.weak LPUART1_IRQHandler
.thumb_set LPUART1_IRQHandler,Default_Handler

.weak SAI1_IRQHandler
.thumb_set SAI1_IRQHandler,Default_Handler

.weak TSC_IRQHandler
.thumb_set TSC_IRQHandler,Default_Handler

.weak EXTI15_10_IRQHandler
.thumb_set EXTI15_10_IRQHandler,Default_Handler

.weak RTC_Alarm_IRQHandler
.thumb_set RTC_Alarm_IRQHandler,Default_Handler

.weak CRS_IRQHandler
.thumb_set CRS_IRQHandler,Default_Handler

.weak PWR_SOTF_BLEACT_802ACT_RFPHASE_IRQHandler
.thumb_set PWR_SOTF_BLEACT_802ACT_RFPHASE_IRQHandler,Default_Handler

.weak IPCC_C1_RX_IRQHandler
.thumb_set IPCC_C1_RX_IRQHandler,Default_Handler

.weak IPCC_C1_TX_IRQHandler
.thumb_set IPCC_C1_TX_IRQHandler,Default_Handler

.weak HSEM_IRQHandler
.thumb_set HSEM_IRQHandler,Default_Handler

.weak LPTIM1_IRQHandler
.thumb_set LPTIM1_IRQHandler,Default_Handler

.weak LPTIM2_IRQHandler
.thumb_set LPTIM2_IRQHandler,Default_Handler

.weak LCD_IRQHandler
.thumb_set LCD_IRQHandler,Default_Handler

.weak QUADSPI_IRQHandler
.thumb_set QUADSPI_IRQHandler,Default_Handler

.weak AES1_IRQHandler
.thumb_set AES1_IRQHandler,Default_Handler

.weak AES2_IRQHandler
.thumb_set AES2_IRQHandler,Default_Handler

.weak RNG_IRQHandler
.thumb_set RNG_IRQHandler,Default_Handler

.weak FPU_IRQHandler
.thumb_set FPU_IRQHandler,Default_Handler

.weak DMA2_Channel1_IRQHandler
.thumb_set DMA2_Channel1_IRQHandler,Default_Handler

.weak DMA2_Channel2_IRQHandler
.thumb_set DMA2_Channel2_IRQHandler,Default_Handler

.weak DMA2_Channel3_IRQHandler
.thumb_set DMA2_Channel3_IRQHandler,Default_Handler

.weak DMA2_Channel4_IRQHandler
.thumb_set DMA2_Channel4_IRQHandler,Default_Handler

.weak DMA2_Channel5_IRQHandler
.thumb_set DMA2_Channel5_IRQHandler,Default_Handler

.weak DMA2_Channel6_IRQHandler
.thumb_set DMA2_Channel6_IRQHandler,Default_Handler

.weak DMA2_Channel7_IRQHandler
.thumb_set DMA2_Channel7_IRQHandler,Default_Handler

.weak DMAMUX1_OVR_IRQHandler
.thumb_set DMAMUX1_OVR_IRQHandler,Default_Handler