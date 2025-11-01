/**
 * @file boot.c
 * @brief TweaknGeek Kernel Boot Sequence Implementation
 * 
 * This file implements the kernel boot sequence including hardware
 * initialization, clock setup, and early system configuration.
 */

#include "kernel.h"
#include "boot.h"
#include <string.h>

/* Boot sequence state tracking */
static boot_stage_t current_boot_stage = BOOT_STAGE_START;
static uint32_t boot_start_time = 0;
static bool boot_error_flag = false;

/* Hardware register definitions for STM32WB55 */
#define RCC_BASE            0x58000000UL
#define RCC_CR              (*(volatile uint32_t*)(RCC_BASE + 0x00))
#define RCC_CFGR            (*(volatile uint32_t*)(RCC_BASE + 0x08))
#define RCC_PLLCFGR         (*(volatile uint32_t*)(RCC_BASE + 0x0C))

#define FLASH_BASE          0x58004000UL
#define FLASH_ACR           (*(volatile uint32_t*)(FLASH_BASE + 0x00))

#define PWR_BASE            0x58000400UL
#define PWR_CR1             (*(volatile uint32_t*)(PWR_BASE + 0x00))

#define SYSTICK_BASE        0xE000E010UL
#define SYSTICK_CTRL        (*(volatile uint32_t*)(SYSTICK_BASE + 0x00))
#define SYSTICK_LOAD        (*(volatile uint32_t*)(SYSTICK_BASE + 0x04))
#define SYSTICK_VAL         (*(volatile uint32_t*)(SYSTICK_BASE + 0x08))

/* Clock configuration constants */
#define HSE_FREQUENCY       32000000UL  /* 32MHz HSE */
#define PLL_M               4           /* HSE/4 = 8MHz */
#define PLL_N               16          /* 8MHz * 16 = 128MHz */
#define PLL_R               2           /* 128MHz / 2 = 64MHz */

/**
 * @brief Initialize hardware components
 * 
 * Performs basic hardware initialization including power management,
 * flash configuration, and GPIO setup.
 * 
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t boot_init_hardware(void)
{
    boot_set_stage(BOOT_STAGE_HARDWARE_INIT);
    
    /* Configure power management */
    PWR_CR1 |= (1 << 9);  /* Enable voltage regulator */
    
    /* Configure flash wait states for 64MHz operation */
    FLASH_ACR = (FLASH_ACR & ~0x7) | 0x3;  /* 3 wait states */
    
    /* Enable instruction cache and prefetch */
    FLASH_ACR |= (1 << 9) | (1 << 8);
    
    return KERNEL_OK;
}

/**
 * @brief Initialize system clocks
 * 
 * Configures the system clock tree to run at 64MHz using HSE and PLL.
 * 
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t boot_init_clocks(void)
{
    boot_set_stage(BOOT_STAGE_CLOCK_INIT);
    
    uint32_t timeout = 10000;
    
    /* Enable HSE oscillator */
    RCC_CR |= (1 << 16);  /* HSEON */
    
    /* Wait for HSE to be ready */
    while (!(RCC_CR & (1 << 17)) && timeout--) {
        /* HSE ready flag */
    }
    
    if (timeout == 0) {
        boot_error_flag = true;
        return KERNEL_ERROR_TIMEOUT;
    }
    
    /* Configure PLL */
    RCC_PLLCFGR = (PLL_R << 25) |     /* PLLR */
                  (1 << 24) |         /* PLLREN */
                  (PLL_N << 8) |      /* PLLN */
                  (PLL_M << 4) |      /* PLLM */
                  (2 << 0);           /* PLLSRC = HSE */
    
    /* Enable PLL */
    RCC_CR |= (1 << 24);  /* PLLON */
    
    /* Wait for PLL to be ready */
    timeout = 10000;
    while (!(RCC_CR & (1 << 25)) && timeout--) {
        /* PLL ready flag */
    }
    
    if (timeout == 0) {
        boot_error_flag = true;
        return KERNEL_ERROR_TIMEOUT;
    }
    
    /* Switch system clock to PLL */
    RCC_CFGR = (RCC_CFGR & ~0x3) | 0x3;  /* SW = PLL */
    
    /* Wait for clock switch to complete */
    timeout = 10000;
    while (((RCC_CFGR >> 2) & 0x3) != 0x3 && timeout--) {
        /* SWS = PLL */
    }
    
    if (timeout == 0) {
        boot_error_flag = true;
        return KERNEL_ERROR_TIMEOUT;
    }
    
    return KERNEL_OK;
}

/**
 * @brief Initialize system timers
 * 
 * Configures SysTick timer for kernel scheduling and timekeeping.
 * 
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t boot_init_timers(void)
{
    /* Configure SysTick for 1ms interrupts */
    SYSTICK_LOAD = (CPU_FREQUENCY_HZ / SYSTEM_TICK_HZ) - 1;
    SYSTICK_VAL = 0;
    
    /* Enable SysTick with processor clock and interrupt */
    SYSTICK_CTRL = (1 << 2) |  /* CLKSOURCE = processor clock */
                   (1 << 1) |  /* TICKINT = enable interrupt */
                   (1 << 0);   /* ENABLE = enable counter */
    
    return KERNEL_OK;
}

/**
 * @brief Set current boot stage
 * 
 * @param stage Boot stage to set
 */
void boot_set_stage(boot_stage_t stage)
{
    current_boot_stage = stage;
}

/**
 * @brief Get current boot stage
 * 
 * @return Current boot stage
 */
boot_stage_t boot_get_stage(void)
{
    return current_boot_stage;
}

/**
 * @brief Check if boot sequence encountered errors
 * 
 * @return true if boot errors occurred, false otherwise
 */
bool boot_has_errors(void)
{
    return boot_error_flag;
}

/**
 * @brief Get boot elapsed time in milliseconds
 * 
 * @return Boot time in milliseconds
 */
uint32_t boot_get_elapsed_time(void)
{
    return kernel_get_tick_count() - boot_start_time;
}

/**
 * @brief Initialize boot sequence timing
 */
void boot_init_timing(void)
{
    boot_start_time = kernel_get_tick_count();
}