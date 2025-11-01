/**
 * @file interrupt.h
 * @brief TweaknGeek Interrupt Handling Definitions
 * 
 * This file contains interrupt handling data structures, constants,
 * and function declarations for the TweaknGeek kernel.
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>
#include <stdbool.h>
#include "kernel.h"

/* Interrupt Numbers for STM32WB55 */
typedef enum {
    IRQ_WWDG = 0,
    IRQ_PVD_PVM = 1,
    IRQ_TAMP_STAMP_LSECSS = 2,
    IRQ_RTC_WKUP = 3,
    IRQ_FLASH = 4,
    IRQ_RCC = 5,
    IRQ_EXTI0 = 6,
    IRQ_EXTI1 = 7,
    IRQ_EXTI2 = 8,
    IRQ_EXTI3 = 9,
    IRQ_EXTI4 = 10,
    IRQ_DMA1_CH1 = 11,
    IRQ_DMA1_CH2 = 12,
    IRQ_DMA1_CH3 = 13,
    IRQ_DMA1_CH4 = 14,
    IRQ_DMA1_CH5 = 15,
    IRQ_DMA1_CH6 = 16,
    IRQ_DMA1_CH7 = 17,
    IRQ_ADC1 = 18,
    IRQ_USB_HP = 19,
    IRQ_USB_LP = 20,
    IRQ_C2SEV_PWR_C2H = 21,
    IRQ_COMP = 22,
    IRQ_EXTI9_5 = 23,
    IRQ_TIM1_BRK = 24,
    IRQ_TIM1_UP_TIM16 = 25,
    IRQ_TIM1_TRG_COM_TIM17 = 26,
    IRQ_TIM1_CC = 27,
    IRQ_TIM2 = 28,
    IRQ_PKA = 29,
    IRQ_I2C1_EV = 30,
    IRQ_I2C1_ER = 31,
    IRQ_I2C3_EV = 32,
    IRQ_I2C3_ER = 33,
    IRQ_SPI1 = 34,
    IRQ_SPI2 = 35,
    IRQ_USART1 = 36,
    IRQ_LPUART1 = 37,
    IRQ_SAI1 = 38,
    IRQ_TSC = 39,
    IRQ_EXTI15_10 = 40,
    IRQ_RTC_ALARM = 41,
    IRQ_CRS = 42,
    IRQ_PWR_SOTF_BLEACT_802ACT_RFPHASE = 43,
    IRQ_IPCC_C1_RX = 44,
    IRQ_IPCC_C1_TX = 45,
    IRQ_HSEM = 46,
    IRQ_LPTIM1 = 47,
    IRQ_LPTIM2 = 48,
    IRQ_LCD = 49,
    IRQ_QUADSPI = 50,
    IRQ_AES1 = 51,
    IRQ_AES2 = 52,
    IRQ_RNG = 53,
    IRQ_FPU = 54,
    IRQ_DMA2_CH1 = 55,
    IRQ_DMA2_CH2 = 56,
    IRQ_DMA2_CH3 = 57,
    IRQ_DMA2_CH4 = 58,
    IRQ_DMA2_CH5 = 59,
    IRQ_DMA2_CH6 = 60,
    IRQ_DMA2_CH7 = 61,
    IRQ_DMAMUX1_OVR = 62,
    IRQ_MAX_COUNT = 63
} irq_number_t;

/* Interrupt Priority Levels */
typedef enum {
    IRQ_PRIORITY_HIGHEST = 0,
    IRQ_PRIORITY_HIGH = 1,
    IRQ_PRIORITY_NORMAL = 2,
    IRQ_PRIORITY_LOW = 3,
    IRQ_PRIORITY_LOWEST = 4
} irq_priority_t;

/* Interrupt Handler Function Type */
typedef void (*irq_handler_t)(void);

/* Interrupt Descriptor */
typedef struct {
    irq_number_t irq_number;
    irq_handler_t handler;
    irq_priority_t priority;
    bool enabled;
    uint32_t count;
    char name[16];
} interrupt_descriptor_t;

/* System Call Numbers */
typedef enum {
    SYSCALL_PROCESS_CREATE = 0,
    SYSCALL_PROCESS_TERMINATE = 1,
    SYSCALL_MEMORY_ALLOC = 2,
    SYSCALL_MEMORY_FREE = 3,
    SYSCALL_SCHEDULER_YIELD = 4,
    SYSCALL_GET_SYSTEM_INFO = 5,
    SYSCALL_MAX_COUNT = 6
} syscall_number_t;

/* System Call Handler Function Type */
typedef uint32_t (*syscall_handler_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);

/* Interrupt Statistics */
typedef struct {
    uint32_t total_interrupts;
    uint32_t nested_interrupts;
    uint32_t max_nesting_level;
    uint32_t current_nesting_level;
    uint32_t system_calls;
    uint32_t interrupt_latency_us;
} interrupt_stats_t;

/* NVIC Register Definitions */
#define NVIC_BASE           0xE000E100UL
#define NVIC_ISER(n)        (*(volatile uint32_t*)(NVIC_BASE + 0x000 + (n) * 4))
#define NVIC_ICER(n)        (*(volatile uint32_t*)(NVIC_BASE + 0x080 + (n) * 4))
#define NVIC_ISPR(n)        (*(volatile uint32_t*)(NVIC_BASE + 0x100 + (n) * 4))
#define NVIC_ICPR(n)        (*(volatile uint32_t*)(NVIC_BASE + 0x180 + (n) * 4))
#define NVIC_IPR(n)         (*(volatile uint32_t*)(NVIC_BASE + 0x300 + (n) * 4))

/* SCB Register Definitions */
#define SCB_BASE            0xE000ED00UL
#define SCB_VTOR            (*(volatile uint32_t*)(SCB_BASE + 0x08))
#define SCB_SHPR1           (*(volatile uint32_t*)(SCB_BASE + 0x18))
#define SCB_SHPR2           (*(volatile uint32_t*)(SCB_BASE + 0x1C))
#define SCB_SHPR3           (*(volatile uint32_t*)(SCB_BASE + 0x20))

/* Interrupt Management Functions */
kernel_status_t interrupt_init(void);
kernel_status_t interrupt_register(irq_number_t irq, irq_handler_t handler, 
                                  irq_priority_t priority, const char* name);
kernel_status_t interrupt_unregister(irq_number_t irq);
kernel_status_t interrupt_enable(irq_number_t irq);
kernel_status_t interrupt_disable(irq_number_t irq);
kernel_status_t interrupt_set_priority(irq_number_t irq, irq_priority_t priority);

/* System Call Interface */
kernel_status_t syscall_register(syscall_number_t syscall, syscall_handler_t handler);
uint32_t syscall_invoke(syscall_number_t syscall, uint32_t arg1, uint32_t arg2, 
                       uint32_t arg3, uint32_t arg4);

/* Interrupt Control */
void interrupt_global_enable(void);
void interrupt_global_disable(void);
bool interrupt_is_in_isr(void);
uint32_t interrupt_get_nesting_level(void);

/* Statistics */
interrupt_stats_t* interrupt_get_stats(void);

/* Default Interrupt Handlers */
void default_irq_handler(void);
void svc_handler(uint32_t* stack_frame);

/* System Call Initialization */
kernel_status_t syscalls_init(void);

#endif /* INTERRUPT_H */