/**
 * @file interrupt_handlers.c
 * @brief Interrupt Handler Wrappers
 * 
 * This file provides C wrapper functions for interrupt handlers that
 * connect the assembly vector table to the C interrupt management system.
 */

#include "interrupt.h"

/* External declaration of common handler */
extern void interrupt_common_handler(irq_number_t irq_number);

/* System exception handlers */
void NMI_Handler(void)
{
    /* Non-maskable interrupt - handle critical system errors */
    while (1) {
        /* System halt - requires reset */
    }
}

void HardFault_Handler(void)
{
    /* Hard fault - critical system error */
    while (1) {
        /* System halt - requires reset */
    }
}

void MemManage_Handler(void)
{
    /* Memory management fault */
    while (1) {
        /* System halt - requires reset */
    }
}

void BusFault_Handler(void)
{
    /* Bus fault */
    while (1) {
        /* System halt - requires reset */
    }
}

void UsageFault_Handler(void)
{
    /* Usage fault */
    while (1) {
        /* System halt - requires reset */
    }
}

void SVC_Handler(void)
{
    /* System call handler - implemented in interrupt.c */
    __asm volatile (
        "tst lr, #4\n"
        "ite eq\n"
        "mrseq r0, msp\n"
        "mrsne r0, psp\n"
        "b svc_handler\n"
    );
}

void DebugMon_Handler(void)
{
    /* Debug monitor - not implemented */
}

void PendSV_Handler(void)
{
    /* PendSV handler - used by scheduler for context switching */
    /* Implementation will be added when scheduler is enhanced */
}

/* External interrupt handlers - these call the common handler */
void WWDG_IRQHandler(void)
{
    interrupt_common_handler(IRQ_WWDG);
}

void PVD_PVM_IRQHandler(void)
{
    interrupt_common_handler(IRQ_PVD_PVM);
}

void TAMP_STAMP_LSECSS_IRQHandler(void)
{
    interrupt_common_handler(IRQ_TAMP_STAMP_LSECSS);
}

void RTC_WKUP_IRQHandler(void)
{
    interrupt_common_handler(IRQ_RTC_WKUP);
}

void FLASH_IRQHandler(void)
{
    interrupt_common_handler(IRQ_FLASH);
}

void RCC_IRQHandler(void)
{
    interrupt_common_handler(IRQ_RCC);
}

void EXTI0_IRQHandler(void)
{
    interrupt_common_handler(IRQ_EXTI0);
}

void EXTI1_IRQHandler(void)
{
    interrupt_common_handler(IRQ_EXTI1);
}

void EXTI2_IRQHandler(void)
{
    interrupt_common_handler(IRQ_EXTI2);
}

void EXTI3_IRQHandler(void)
{
    interrupt_common_handler(IRQ_EXTI3);
}

void EXTI4_IRQHandler(void)
{
    interrupt_common_handler(IRQ_EXTI4);
}

void DMA1_Channel1_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA1_CH1);
}

void DMA1_Channel2_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA1_CH2);
}

void DMA1_Channel3_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA1_CH3);
}

void DMA1_Channel4_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA1_CH4);
}

void DMA1_Channel5_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA1_CH5);
}

void DMA1_Channel6_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA1_CH6);
}

void DMA1_Channel7_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA1_CH7);
}

void ADC1_IRQHandler(void)
{
    interrupt_common_handler(IRQ_ADC1);
}

void USB_HP_IRQHandler(void)
{
    interrupt_common_handler(IRQ_USB_HP);
}

void USB_LP_IRQHandler(void)
{
    interrupt_common_handler(IRQ_USB_LP);
}

void C2SEV_PWR_C2H_IRQHandler(void)
{
    interrupt_common_handler(IRQ_C2SEV_PWR_C2H);
}

void COMP_IRQHandler(void)
{
    interrupt_common_handler(IRQ_COMP);
}

void EXTI9_5_IRQHandler(void)
{
    interrupt_common_handler(IRQ_EXTI9_5);
}

void TIM1_BRK_IRQHandler(void)
{
    interrupt_common_handler(IRQ_TIM1_BRK);
}

void TIM1_UP_TIM16_IRQHandler(void)
{
    interrupt_common_handler(IRQ_TIM1_UP_TIM16);
}

void TIM1_TRG_COM_TIM17_IRQHandler(void)
{
    interrupt_common_handler(IRQ_TIM1_TRG_COM_TIM17);
}

void TIM1_CC_IRQHandler(void)
{
    interrupt_common_handler(IRQ_TIM1_CC);
}

void TIM2_IRQHandler(void)
{
    interrupt_common_handler(IRQ_TIM2);
}

void PKA_IRQHandler(void)
{
    interrupt_common_handler(IRQ_PKA);
}

void I2C1_EV_IRQHandler(void)
{
    interrupt_common_handler(IRQ_I2C1_EV);
}

void I2C1_ER_IRQHandler(void)
{
    interrupt_common_handler(IRQ_I2C1_ER);
}

void I2C3_EV_IRQHandler(void)
{
    interrupt_common_handler(IRQ_I2C3_EV);
}

void I2C3_ER_IRQHandler(void)
{
    interrupt_common_handler(IRQ_I2C3_ER);
}

void SPI1_IRQHandler(void)
{
    interrupt_common_handler(IRQ_SPI1);
}

void SPI2_IRQHandler(void)
{
    interrupt_common_handler(IRQ_SPI2);
}

void USART1_IRQHandler(void)
{
    interrupt_common_handler(IRQ_USART1);
}

void LPUART1_IRQHandler(void)
{
    interrupt_common_handler(IRQ_LPUART1);
}

void SAI1_IRQHandler(void)
{
    interrupt_common_handler(IRQ_SAI1);
}

void TSC_IRQHandler(void)
{
    interrupt_common_handler(IRQ_TSC);
}

void EXTI15_10_IRQHandler(void)
{
    interrupt_common_handler(IRQ_EXTI15_10);
}

void RTC_Alarm_IRQHandler(void)
{
    interrupt_common_handler(IRQ_RTC_ALARM);
}

void CRS_IRQHandler(void)
{
    interrupt_common_handler(IRQ_CRS);
}

void PWR_SOTF_BLEACT_802ACT_RFPHASE_IRQHandler(void)
{
    interrupt_common_handler(IRQ_PWR_SOTF_BLEACT_802ACT_RFPHASE);
}

void IPCC_C1_RX_IRQHandler(void)
{
    interrupt_common_handler(IRQ_IPCC_C1_RX);
}

void IPCC_C1_TX_IRQHandler(void)
{
    interrupt_common_handler(IRQ_IPCC_C1_TX);
}

void HSEM_IRQHandler(void)
{
    interrupt_common_handler(IRQ_HSEM);
}

void LPTIM1_IRQHandler(void)
{
    interrupt_common_handler(IRQ_LPTIM1);
}

void LPTIM2_IRQHandler(void)
{
    interrupt_common_handler(IRQ_LPTIM2);
}

void LCD_IRQHandler(void)
{
    interrupt_common_handler(IRQ_LCD);
}

void QUADSPI_IRQHandler(void)
{
    interrupt_common_handler(IRQ_QUADSPI);
}

void AES1_IRQHandler(void)
{
    interrupt_common_handler(IRQ_AES1);
}

void AES2_IRQHandler(void)
{
    interrupt_common_handler(IRQ_AES2);
}

void RNG_IRQHandler(void)
{
    interrupt_common_handler(IRQ_RNG);
}

void FPU_IRQHandler(void)
{
    interrupt_common_handler(IRQ_FPU);
}

void DMA2_Channel1_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA2_CH1);
}

void DMA2_Channel2_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA2_CH2);
}

void DMA2_Channel3_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA2_CH3);
}

void DMA2_Channel4_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA2_CH4);
}

void DMA2_Channel5_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA2_CH5);
}

void DMA2_Channel6_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA2_CH6);
}

void DMA2_Channel7_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMA2_CH7);
}

void DMAMUX1_OVR_IRQHandler(void)
{
    interrupt_common_handler(IRQ_DMAMUX1_OVR);
}