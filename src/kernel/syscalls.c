/**
 * @file syscalls.c
 * @brief System Call Implementations
 * 
 * This file implements the basic system calls for the TweaknGeek kernel,
 * providing a controlled interface for applications to access kernel services.
 */

#include "interrupt.h"
#include "memory.h"
#include "scheduler.h"
#include "kernel.h"

/* Forward declarations */
static uint32_t syscall_process_create(uint32_t entry_point, uint32_t stack_size, uint32_t priority, uint32_t flags);
static uint32_t syscall_process_terminate(uint32_t process_id, uint32_t exit_code, uint32_t unused1, uint32_t unused2);
static uint32_t syscall_memory_alloc(uint32_t size, uint32_t alignment, uint32_t flags, uint32_t unused);
static uint32_t syscall_memory_free(uint32_t address, uint32_t unused1, uint32_t unused2, uint32_t unused3);
static uint32_t syscall_scheduler_yield(uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4);
static uint32_t syscall_get_system_info(uint32_t info_buffer, uint32_t buffer_size, uint32_t unused1, uint32_t unused2);

/**
 * @brief Initialize system call handlers
 * 
 * Registers all system call handlers with the interrupt system.
 * 
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t syscalls_init(void)
{
    kernel_status_t status;
    
    /* Register system call handlers */
    status = syscall_register(SYSCALL_PROCESS_CREATE, syscall_process_create);
    if (status != KERNEL_OK) return status;
    
    status = syscall_register(SYSCALL_PROCESS_TERMINATE, syscall_process_terminate);
    if (status != KERNEL_OK) return status;
    
    status = syscall_register(SYSCALL_MEMORY_ALLOC, syscall_memory_alloc);
    if (status != KERNEL_OK) return status;
    
    status = syscall_register(SYSCALL_MEMORY_FREE, syscall_memory_free);
    if (status != KERNEL_OK) return status;
    
    status = syscall_register(SYSCALL_SCHEDULER_YIELD, syscall_scheduler_yield);
    if (status != KERNEL_OK) return status;
    
    status = syscall_register(SYSCALL_GET_SYSTEM_INFO, syscall_get_system_info);
    if (status != KERNEL_OK) return status;
    
    return KERNEL_OK;
}

/**
 * @brief System call: Create a new process
 * 
 * @param entry_point Process entry point address
 * @param stack_size Stack size in bytes
 * @param priority Process priority
 * @param flags Process creation flags
 * @return Process ID on success, 0 on failure
 */
static uint32_t syscall_process_create(uint32_t entry_point, uint32_t stack_size, uint32_t priority, uint32_t flags)
{
    /* Validate parameters */
    if (entry_point == 0 || stack_size < 512) {
        return 0; /* Invalid parameters */
    }
    
    /* TODO: Implement process creation when scheduler is enhanced */
    /* For now, return error */
    return 0;
}

/**
 * @brief System call: Terminate a process
 * 
 * @param process_id Process ID to terminate
 * @param exit_code Exit code
 * @param unused1 Unused parameter
 * @param unused2 Unused parameter
 * @return 0 on success, error code on failure
 */
static uint32_t syscall_process_terminate(uint32_t process_id, uint32_t exit_code, uint32_t unused1, uint32_t unused2)
{
    (void)unused1;
    (void)unused2;
    
    /* Validate process ID */
    if (process_id == 0) {
        return 1; /* Invalid process ID */
    }
    
    /* TODO: Implement process termination when scheduler is enhanced */
    /* For now, return error */
    return 1;
}

/**
 * @brief System call: Allocate memory
 * 
 * @param size Size in bytes to allocate
 * @param alignment Memory alignment requirement
 * @param flags Allocation flags
 * @param unused Unused parameter
 * @return Memory address on success, 0 on failure
 */
static uint32_t syscall_memory_alloc(uint32_t size, uint32_t alignment, uint32_t flags, uint32_t unused)
{
    (void)alignment;
    (void)flags;
    (void)unused;
    
    /* Validate size */
    if (size == 0 || size > (1024 * 1024)) { /* Max 1MB allocation */
        return 0;
    }
    
    /* Use kernel memory allocator */
    void* ptr = memory_alloc(size);
    return (uint32_t)ptr;
}

/**
 * @brief System call: Free memory
 * 
 * @param address Memory address to free
 * @param unused1 Unused parameter
 * @param unused2 Unused parameter
 * @param unused3 Unused parameter
 * @return 0 on success, error code on failure
 */
static uint32_t syscall_memory_free(uint32_t address, uint32_t unused1, uint32_t unused2, uint32_t unused3)
{
    (void)unused1;
    (void)unused2;
    (void)unused3;
    
    /* Validate address */
    if (address == 0) {
        return 1; /* Invalid address */
    }
    
    /* Use kernel memory allocator */
    memory_free((void*)address);
    return 0;
}

/**
 * @brief System call: Yield processor to scheduler
 * 
 * @param unused1 Unused parameter
 * @param unused2 Unused parameter
 * @param unused3 Unused parameter
 * @param unused4 Unused parameter
 * @return Always returns 0
 */
static uint32_t syscall_scheduler_yield(uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4)
{
    (void)unused1;
    (void)unused2;
    (void)unused3;
    (void)unused4;
    
    /* Trigger scheduler to run next task */
    scheduler_yield();
    return 0;
}

/**
 * @brief System call: Get system information
 * 
 * @param info_buffer Buffer to store system information
 * @param buffer_size Size of the buffer
 * @param unused1 Unused parameter
 * @param unused2 Unused parameter
 * @return 0 on success, error code on failure
 */
static uint32_t syscall_get_system_info(uint32_t info_buffer, uint32_t buffer_size, uint32_t unused1, uint32_t unused2)
{
    (void)unused1;
    (void)unused2;
    
    /* Validate parameters */
    if (info_buffer == 0 || buffer_size < sizeof(system_info_t)) {
        return 1; /* Invalid parameters */
    }
    
    /* Get system information from kernel */
    system_info_t* sys_info = kernel_get_system_info();
    if (sys_info == NULL) {
        return 2; /* System info not available */
    }
    
    /* Copy system information to user buffer */
    /* TODO: Add memory protection checks when MMU is implemented */
    system_info_t* user_buffer = (system_info_t*)info_buffer;
    *user_buffer = *sys_info;
    
    return 0;
}