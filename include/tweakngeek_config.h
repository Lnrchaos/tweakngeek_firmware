/**
 * @file tweakngeek_config.h
 * @brief TweaknGeek Firmware Configuration
 * 
 * This file contains system-wide configuration constants and settings
 * for the TweaknGeek firmware.
 */

#ifndef TWEAKNGEEK_CONFIG_H
#define TWEAKNGEEK_CONFIG_H

#include <stdint.h>

/* Firmware Version Information */
#define TWEAKNGEEK_VERSION_MAJOR    1
#define TWEAKNGEEK_VERSION_MINOR    0
#define TWEAKNGEEK_VERSION_PATCH    0
#define TWEAKNGEEK_VERSION_STRING   "1.0.0"

/* Hardware Configuration */
#define TARGET_HARDWARE_FLIPPER_ZERO    1
#define MCU_STM32WB55RG                 1
#define CPU_FREQUENCY_HZ                64000000UL

/* Memory Configuration */
#define FLASH_SIZE                      (1024 * 1024)   /* 1MB Flash */
#define SRAM_SIZE                       (256 * 1024)    /* 256KB SRAM */
#define HEAP_SIZE                       (32 * 1024)     /* 32KB Heap */
#define STACK_SIZE                      (8 * 1024)      /* 8KB Stack */

/* System Configuration */
#define MAX_PROCESSES                   16
#define MAX_APPLICATIONS                32
#define SYSTEM_TICK_HZ                  1000
#define WATCHDOG_TIMEOUT_MS             5000

/* Hardware Abstraction Layer Configuration */
#define HAL_GPIO_PINS                   64
#define HAL_RADIO_CHANNELS              256
#define HAL_DISPLAY_WIDTH               128
#define HAL_DISPLAY_HEIGHT              64

/* Application Runtime Configuration */
#define APP_MAX_MEMORY_SIZE             (64 * 1024)     /* 64KB per app */
#define APP_MAX_STACK_SIZE              (4 * 1024)      /* 4KB per app */
#define APP_SANDBOX_ENABLED             1

/* WiFi Emulation Configuration */
#define WIFI_EMU_MAX_CONNECTIONS        4
#define WIFI_EMU_BUFFER_SIZE            1024
#define WIFI_EMU_CHANNEL_DEFAULT        6

/* Debug Configuration */
#ifdef DEBUG
    #define DEBUG_ENABLED               1
    #define DEBUG_UART_ENABLED          1
    #define DEBUG_LOG_LEVEL             3   /* 0=Error, 1=Warn, 2=Info, 3=Debug */
#else
    #define DEBUG_ENABLED               0
    #define DEBUG_UART_ENABLED          0
    #define DEBUG_LOG_LEVEL             0
#endif

/* Feature Flags */
#define FEATURE_WIFI_EMULATION          1
#define FEATURE_CUSTOM_LAYOUT           1
#define FEATURE_SECURE_BOOT             1
#define FEATURE_POWER_MANAGEMENT        1
#define FEATURE_HARDWARE_PROFILING      1

#endif /* TWEAKNGEEK_CONFIG_H */