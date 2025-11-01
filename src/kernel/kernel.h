/**
 * @file kernel.h
 * @brief TweaknGeek Kernel Core Definitions
 * 
 * This file contains the core kernel data structures, constants,
 * and function declarations for the TweaknGeek kernel.
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stdbool.h>
#include "tweakngeek_config.h"

/* Kernel Status Codes */
typedef enum {
    KERNEL_OK = 0,
    KERNEL_ERROR = -1,
    KERNEL_ERROR_INVALID_PARAM = -2,
    KERNEL_ERROR_OUT_OF_MEMORY = -3,
    KERNEL_ERROR_TIMEOUT = -4,
    KERNEL_ERROR_BUSY = -5
} kernel_status_t;

/* System State */
typedef enum {
    SYSTEM_STATE_BOOT = 0,
    SYSTEM_STATE_INIT,
    SYSTEM_STATE_RUNNING,
    SYSTEM_STATE_SLEEP,
    SYSTEM_STATE_ERROR,
    SYSTEM_STATE_SHUTDOWN
} system_state_t;

/* Boot Stage Tracking */
typedef enum {
    BOOT_STAGE_START = 0,
    BOOT_STAGE_HARDWARE_INIT,
    BOOT_STAGE_CLOCK_INIT,
    BOOT_STAGE_MEMORY_INIT,
    BOOT_STAGE_INTERRUPT_INIT,
    BOOT_STAGE_SCHEDULER_INIT,
    BOOT_STAGE_COMPLETE
} boot_stage_t;

/* System Information Structure */
typedef struct {
    uint32_t boot_time_ms;
    uint32_t uptime_ms;
    system_state_t state;
    boot_stage_t boot_stage;
    uint32_t free_memory;
    uint32_t total_memory;
    uint8_t cpu_usage_percent;
} system_info_t;

/* Core Kernel Functions */
kernel_status_t kernel_init(void);
kernel_status_t kernel_start(void);
void kernel_shutdown(void);
system_info_t* kernel_get_system_info(void);
system_state_t kernel_get_state(void);

/* Boot Sequence Functions */
kernel_status_t boot_init_hardware(void);
kernel_status_t boot_init_clocks(void);
kernel_status_t boot_init_timers(void);
void boot_set_stage(boot_stage_t stage);
boot_stage_t boot_get_stage(void);

/* System Tick Functions */
void kernel_tick_handler(void);
uint32_t kernel_get_tick_count(void);
uint32_t kernel_get_uptime_ms(void);

/* Critical Section Management */
void kernel_enter_critical(void);
void kernel_exit_critical(void);

#endif /* KERNEL_H */