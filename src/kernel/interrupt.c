/**
 * @file interrupt.c
 * @brief TweaknGeek Interrupt Handling Implementation
 * 
 * This file implements the interrupt handling system for the TweaknGeek kernel,
 * including interrupt registration, management, and system call interface.
 */

#include "interrupt.h"
#include "memory.h"
#include "scheduler.h"
#include <string.h>

/* CMSIS-style intrinsic functions */
static inline uint32_t __get_PRIMASK(void)
{
    uint32_t result;
    __asm volatile ("MRS %0, primask" : "=r" (result));
    return result;
}

static inline void __disable_irq(void)
{
    __asm volatile ("cpsid i" : : : "memory");
}

static inline void __enable_irq(void)
{
    __asm volatile ("cpsie i" : : : "memory");
}

/* Global interrupt management state */
static interrupt_descriptor_t interrupt_table[IRQ_MAX_COUNT];
static syscall_handler_t syscall_table[SYSCALL_MAX_COUNT];
static interrupt_stats_t interrupt_statistics;
static volatile uint32_t interrupt_nesting_level = 0;
static volatile bool interrupt_system_initialized = false;

/* External vector table from startup code */
extern uint32_t g_pfnVectors[];

/**
 * @brief Initialize the interrupt handling system
 * 
 * Sets up the interrupt vector table, initializes interrupt descriptors,
 * and configures the NVIC for proper interrupt handling.
 * 
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t interrupt_init(void)
{
    /* Clear interrupt table */
    memset(interrupt_table, 0, sizeof(interrupt_table));
    memset(syscall_table, 0, sizeof(syscall_table));
    memset(&interrupt_statistics, 0, sizeof(interrupt_statistics));
    
    /* Initialize interrupt descriptors */
    for (int i = 0; i < IRQ_MAX_COUNT; i++) {
        interrupt_table[i].irq_number = i;
        interrupt_table[i].handler = default_irq_handler;
        interrupt_table[i].priority = IRQ_PRIORITY_NORMAL;
        interrupt_table[i].enabled = false;
        interrupt_table[i].count = 0;
        snprintf(interrupt_table[i].name, sizeof(interrupt_table[i].name), "IRQ_%d", i);
    }
    
    /* Set vector table offset register */
    SCB_VTOR = (uint32_t)g_pfnVectors;
    
    /* Configure system handler priorities */
    /* SVC (System Call) - Highest priority */
    SCB_SHPR2 = (SCB_SHPR2 & 0x00FFFFFF) | (0x00 << 24);
    /* PendSV (Context Switch) - Lowest priority */
    SCB_SHPR3 = (SCB_SHPR3 & 0x00FFFFFF) | (0xFF << 16);
    /* SysTick - Normal priority */
    SCB_SHPR3 = (SCB_SHPR3 & 0xFF00FFFF) | (0x80 << 24);
    
    interrupt_nesting_level = 0;
    interrupt_system_initialized = true;
    
    return KERNEL_OK;
}

/**
 * @brief Register an interrupt handler
 * 
 * @param irq Interrupt number to register
 * @param handler Function pointer to interrupt handler
 * @param priority Interrupt priority level
 * @param name Human-readable name for the interrupt
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t interrupt_register(irq_number_t irq, irq_handler_t handler, 
                                  irq_priority_t priority, const char* name)
{
    if (!interrupt_system_initialized) {
        return KERNEL_ERROR;
    }
    
    if (irq >= IRQ_MAX_COUNT || handler == NULL) {
        return KERNEL_ERROR_INVALID_PARAM;
    }
    
    /* Disable interrupts during registration */
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    
    /* Update interrupt descriptor */
    interrupt_table[irq].handler = handler;
    interrupt_table[irq].priority = priority;
    interrupt_table[irq].count = 0;
    
    if (name != NULL) {
        strncpy(interrupt_table[irq].name, name, sizeof(interrupt_table[irq].name) - 1);
        interrupt_table[irq].name[sizeof(interrupt_table[irq].name) - 1] = '\0';
    }
    
    /* Set interrupt priority in NVIC */
    uint32_t priority_group = irq / 4;
    uint32_t priority_offset = (irq % 4) * 8;
    uint32_t nvic_priority = priority << 4; /* Use upper 4 bits */
    
    NVIC_IPR(priority_group) = (NVIC_IPR(priority_group) & ~(0xFF << priority_offset)) |
                               (nvic_priority << priority_offset);
    
    /* Restore interrupt state */
    if (!primask) {
        __enable_irq();
    }
    
    return KERNEL_OK;
}

/**
 * @brief Unregister an interrupt handler
 * 
 * @param irq Interrupt number to unregister
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t interrupt_unregister(irq_number_t irq)
{
    if (!interrupt_system_initialized || irq >= IRQ_MAX_COUNT) {
        return KERNEL_ERROR_INVALID_PARAM;
    }
    
    /* Disable the interrupt first */
    interrupt_disable(irq);
    
    /* Reset to default handler */
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    
    interrupt_table[irq].handler = default_irq_handler;
    interrupt_table[irq].priority = IRQ_PRIORITY_NORMAL;
    interrupt_table[irq].enabled = false;
    interrupt_table[irq].count = 0;
    snprintf(interrupt_table[irq].name, sizeof(interrupt_table[irq].name), "IRQ_%d", irq);
    
    if (!primask) {
        __enable_irq();
    }
    
    return KERNEL_OK;
}

/**
 * @brief Enable an interrupt
 * 
 * @param irq Interrupt number to enable
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t interrupt_enable(irq_number_t irq)
{
    if (!interrupt_system_initialized || irq >= IRQ_MAX_COUNT) {
        return KERNEL_ERROR_INVALID_PARAM;
    }
    
    uint32_t register_index = irq / 32;
    uint32_t bit_position = irq % 32;
    
    /* Enable in NVIC */
    NVIC_ISER(register_index) = (1 << bit_position);
    
    /* Update descriptor */
    interrupt_table[irq].enabled = true;
    
    return KERNEL_OK;
}

/**
 * @brief Disable an interrupt
 * 
 * @param irq Interrupt number to disable
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t interrupt_disable(irq_number_t irq)
{
    if (!interrupt_system_initialized || irq >= IRQ_MAX_COUNT) {
        return KERNEL_ERROR_INVALID_PARAM;
    }
    
    uint32_t register_index = irq / 32;
    uint32_t bit_position = irq % 32;
    
    /* Disable in NVIC */
    NVIC_ICER(register_index) = (1 << bit_position);
    
    /* Update descriptor */
    interrupt_table[irq].enabled = false;
    
    return KERNEL_OK;
}

/**
 * @brief Set interrupt priority
 * 
 * @param irq Interrupt number
 * @param priority New priority level
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t interrupt_set_priority(irq_number_t irq, irq_priority_t priority)
{
    if (!interrupt_system_initialized || irq >= IRQ_MAX_COUNT) {
        return KERNEL_ERROR_INVALID_PARAM;
    }
    
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    
    /* Update descriptor */
    interrupt_table[irq].priority = priority;
    
    /* Set priority in NVIC */
    uint32_t priority_group = irq / 4;
    uint32_t priority_offset = (irq % 4) * 8;
    uint32_t nvic_priority = priority << 4; /* Use upper 4 bits */
    
    NVIC_IPR(priority_group) = (NVIC_IPR(priority_group) & ~(0xFF << priority_offset)) |
                               (nvic_priority << priority_offset);
    
    if (!primask) {
        __enable_irq();
    }
    
    return KERNEL_OK;
}

/**
 * @brief Register a system call handler
 * 
 * @param syscall System call number
 * @param handler Function pointer to system call handler
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t syscall_register(syscall_number_t syscall, syscall_handler_t handler)
{
    if (!interrupt_system_initialized || syscall >= SYSCALL_MAX_COUNT || handler == NULL) {
        return KERNEL_ERROR_INVALID_PARAM;
    }
    
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    
    syscall_table[syscall] = handler;
    
    if (!primask) {
        __enable_irq();
    }
    
    return KERNEL_OK;
}

/**
 * @brief Invoke a system call
 * 
 * @param syscall System call number
 * @param arg1 First argument
 * @param arg2 Second argument
 * @param arg3 Third argument
 * @param arg4 Fourth argument
 * @return System call return value
 */
uint32_t syscall_invoke(syscall_number_t syscall, uint32_t arg1, uint32_t arg2, 
                       uint32_t arg3, uint32_t arg4)
{
    if (!interrupt_system_initialized || syscall >= SYSCALL_MAX_COUNT) {
        return 0xFFFFFFFF; /* Error return value */
    }
    
    /* Trigger SVC interrupt with syscall number */
    __asm volatile ("svc %0" : : "I" (syscall) : "memory");
    
    /* Return value will be set by SVC handler */
    return 0;
}

/**
 * @brief Enable global interrupts
 */
void interrupt_global_enable(void)
{
    __enable_irq();
}

/**
 * @brief Disable global interrupts
 */
void interrupt_global_disable(void)
{
    __disable_irq();
}

/**
 * @brief Check if currently in interrupt service routine
 * 
 * @return true if in ISR, false otherwise
 */
bool interrupt_is_in_isr(void)
{
    return interrupt_nesting_level > 0;
}

/**
 * @brief Get current interrupt nesting level
 * 
 * @return Current nesting level
 */
uint32_t interrupt_get_nesting_level(void)
{
    return interrupt_nesting_level;
}

/**
 * @brief Get interrupt statistics
 * 
 * @return Pointer to interrupt statistics structure
 */
interrupt_stats_t* interrupt_get_stats(void)
{
    return &interrupt_statistics;
}

/**
 * @brief Default interrupt handler
 * 
 * Called for unregistered interrupts. Increments statistics and returns.
 */
void default_irq_handler(void)
{
    interrupt_nesting_level++;
    interrupt_statistics.total_interrupts++;
    
    if (interrupt_nesting_level > interrupt_statistics.max_nesting_level) {
        interrupt_statistics.max_nesting_level = interrupt_nesting_level;
    }
    
    interrupt_nesting_level--;
}

/**
 * @brief SVC (System Call) handler
 * 
 * Handles system calls by dispatching to registered handlers.
 * 
 * @param stack_frame Pointer to saved register context
 */
void svc_handler(uint32_t* stack_frame)
{
    interrupt_nesting_level++;
    interrupt_statistics.system_calls++;
    
    /* Extract SVC number from instruction */
    uint8_t* svc_instruction = (uint8_t*)(stack_frame[6] - 2);
    uint8_t svc_number = svc_instruction[0];
    
    if (svc_number < SYSCALL_MAX_COUNT && syscall_table[svc_number] != NULL) {
        /* Call registered system call handler */
        uint32_t result = syscall_table[svc_number](
            stack_frame[0], /* R0 */
            stack_frame[1], /* R1 */
            stack_frame[2], /* R2 */
            stack_frame[3]  /* R3 */
        );
        
        /* Store result in R0 for return */
        stack_frame[0] = result;
    } else {
        /* Invalid system call */
        stack_frame[0] = 0xFFFFFFFF;
    }
    
    interrupt_nesting_level--;
}

/**
 * @brief Common interrupt entry point
 * 
 * Called by all interrupt handlers to update statistics and call
 * the registered handler function.
 * 
 * @param irq_number Interrupt number
 */
void interrupt_common_handler(irq_number_t irq_number)
{
    if (!interrupt_system_initialized || irq_number >= IRQ_MAX_COUNT) {
        return;
    }
    
    interrupt_nesting_level++;
    interrupt_statistics.total_interrupts++;
    interrupt_statistics.current_nesting_level = interrupt_nesting_level;
    
    if (interrupt_nesting_level > 1) {
        interrupt_statistics.nested_interrupts++;
    }
    
    if (interrupt_nesting_level > interrupt_statistics.max_nesting_level) {
        interrupt_statistics.max_nesting_level = interrupt_nesting_level;
    }
    
    /* Update interrupt count */
    interrupt_table[irq_number].count++;
    
    /* Call registered handler */
    if (interrupt_table[irq_number].handler != NULL) {
        interrupt_table[irq_number].handler();
    }
    
    interrupt_nesting_level--;
    interrupt_statistics.current_nesting_level = interrupt_nesting_level;
}