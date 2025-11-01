/**
 * @file kernel_core.c
 * @brief TweaknGeek Kernel Core Implementation
 * 
 * This file implements the core kernel functionality including
 * initialization, system state management, and basic services.
 */

#include "kernel.h"
#include "boot.h"
#include "memory.h"
#include "scheduler.h"
#include <string.h>

/* System state tracking */
static system_info_t system_info = {0};
static uint32_t tick_count = 0;
static uint32_t critical_nesting = 0;

/* Forward declarations for other kernel subsystems */
extern kernel_status_t interrupt_init(void);
extern kernel_status_t syscalls_init(void);

/**
 * @brief Initialize the TweaknGeek kernel
 * 
 * Performs complete kernel initialization including hardware setup,
 * memory management, scheduler, and interrupt handling.
 * 
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t kernel_init(void)
{
    kernel_status_t status;
    
    /* Initialize system info structure */
    memset(&system_info, 0, sizeof(system_info_t));
    system_info.state = SYSTEM_STATE_BOOT;
    system_info.boot_stage = BOOT_STAGE_START;
    system_info.total_memory = SRAM_SIZE;
    
    /* Initialize boot timing */
    boot_init_timing();
    
    /* Step 1: Hardware initialization */
    status = boot_init_hardware();
    if (status != KERNEL_OK) {
        system_info.state = SYSTEM_STATE_ERROR;
        return status;
    }
    
    /* Step 2: Clock initialization */
    status = boot_init_clocks();
    if (status != KERNEL_OK) {
        system_info.state = SYSTEM_STATE_ERROR;
        return status;
    }
    
    /* Step 3: Timer initialization */
    status = boot_init_timers();
    if (status != KERNEL_OK) {
        system_info.state = SYSTEM_STATE_ERROR;
        return status;
    }
    
    /* Step 4: Memory management initialization */
    boot_set_stage(BOOT_STAGE_MEMORY_INIT);
    status = memory_init();
    if (status != KERNEL_OK) {
        system_info.state = SYSTEM_STATE_ERROR;
        return status;
    }
    
    /* Step 5: Interrupt system initialization */
    boot_set_stage(BOOT_STAGE_INTERRUPT_INIT);
    status = interrupt_init();
    if (status != KERNEL_OK) {
        system_info.state = SYSTEM_STATE_ERROR;
        return status;
    }
    
    /* Initialize system calls */
    status = syscalls_init();
    if (status != KERNEL_OK) {
        system_info.state = SYSTEM_STATE_ERROR;
        return status;
    }
    
    /* Step 6: Scheduler initialization */
    boot_set_stage(BOOT_STAGE_SCHEDULER_INIT);
    status = scheduler_init();
    if (status != KERNEL_OK) {
        system_info.state = SYSTEM_STATE_ERROR;
        return status;
    }
    
    /* Boot sequence complete */
    boot_set_stage(BOOT_STAGE_COMPLETE);
    system_info.state = SYSTEM_STATE_INIT;
    system_info.boot_stage = BOOT_STAGE_COMPLETE;
    system_info.boot_time_ms = boot_get_elapsed_time();
    
    return KERNEL_OK;
}

/**
 * @brief Start kernel operation
 * 
 * Transitions kernel from initialization to running state.
 * 
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t kernel_start(void)
{
    if (system_info.state != SYSTEM_STATE_INIT) {
        return KERNEL_ERROR;
    }
    
    /* Start scheduler */
    scheduler_start();
    
    system_info.state = SYSTEM_STATE_RUNNING;
    return KERNEL_OK;
}

/**
 * @brief Shutdown kernel
 * 
 * Performs orderly kernel shutdown.
 */
void kernel_shutdown(void)
{
    system_info.state = SYSTEM_STATE_SHUTDOWN;
    
    /* Disable interrupts */
    kernel_enter_critical();
    
    /* TODO: Cleanup resources, save state, etc. */
    
    /* Enter infinite loop */
    while (1) {
        __asm__("wfi");
    }
}

/**
 * @brief Get system information
 * 
 * @return Pointer to system information structure
 */
system_info_t* kernel_get_system_info(void)
{
    /* Update dynamic fields */
    system_info.uptime_ms = kernel_get_uptime_ms();
    system_info.boot_stage = boot_get_stage();
    
    return &system_info;
}

/**
 * @brief Get current system state
 * 
 * @return Current system state
 */
system_state_t kernel_get_state(void)
{
    return system_info.state;
}

/**
 * @brief System tick handler (called from SysTick interrupt)
 * 
 * Updates system tick counter and handles time-based operations.
 */
void kernel_tick_handler(void)
{
    tick_count++;
    
    /* Update system uptime */
    system_info.uptime_ms = tick_count;
    
    /* Call scheduler tick handler */
    scheduler_tick();
}

/**
 * @brief Get current tick count
 * 
 * @return Current system tick count
 */
uint32_t kernel_get_tick_count(void)
{
    return tick_count;
}

/**
 * @brief Get system uptime in milliseconds
 * 
 * @return System uptime in milliseconds
 */
uint32_t kernel_get_uptime_ms(void)
{
    return tick_count;
}

/**
 * @brief Enter critical section
 * 
 * Disables interrupts and tracks nesting level.
 */
void kernel_enter_critical(void)
{
    __asm__ volatile ("cpsid i" ::: "memory");
    critical_nesting++;
}

/**
 * @brief Exit critical section
 * 
 * Re-enables interrupts when nesting level reaches zero.
 */
void kernel_exit_critical(void)
{
    if (critical_nesting > 0) {
        critical_nesting--;
        if (critical_nesting == 0) {
            __asm__ volatile ("cpsie i" ::: "memory");
        }
    }
}