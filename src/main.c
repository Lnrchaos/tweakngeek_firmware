/**
 * @file main.c
 * @brief TweaknGeek Firmware Main Entry Point
 * 
 * This file contains the main entry point for the TweaknGeek firmware.
 * It initializes all system components and starts the main execution loop.
 */

#include <stdint.h>
#include <stdbool.h>
#include "kernel/kernel.h"

// Forward declarations for system initialization
extern void hal_init(void);
extern void runtime_init(void);
extern void services_init(void);
extern void applications_init(void);

/**
 * @brief Main firmware entry point
 * 
 * Initializes all system components in the correct order:
 * 1. Kernel layer (memory, scheduling, interrupts)
 * 2. Hardware abstraction layer
 * 3. System services
 * 4. Application runtime
 * 5. Built-in applications
 * 
 * @return Should never return in normal operation
 */
int main(void)
{
    kernel_status_t status;
    
    // Initialize kernel layer first
    status = kernel_init();
    if (status != KERNEL_OK) {
        // Boot failed - enter error state
        while (1) {
            __asm__("wfi");
        }
    }
    
    // Start kernel operation
    status = kernel_start();
    if (status != KERNEL_OK) {
        // Kernel start failed
        while (1) {
            __asm__("wfi");
        }
    }
    
    // Initialize hardware abstraction layer
    hal_init();
    
    // Initialize system services
    services_init();
    
    // Initialize application runtime
    runtime_init();
    
    // Initialize built-in applications
    applications_init();
    
    // Main execution loop - should never exit
    while (1) {
        // System tick and scheduling handled by kernel
        // Applications run in their own contexts
        __asm__("wfi"); // Wait for interrupt
    }
    
    // Should never reach here
    return 0;
}