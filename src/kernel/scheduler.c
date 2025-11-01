/**
 * @file scheduler.c
 * @brief TweaknGeek Process Scheduler Implementation
 * 
 * This file implements the preemptive multitasking scheduler including
 * process management, context switching, and task scheduling.
 */

#include "scheduler.h"
#include "memory.h"
#include <string.h>

/* Scheduler Configuration */
#define DEFAULT_TIME_SLICE_MS   10
#define IDLE_PROCESS_ID         0
#define MAX_PROCESSES           MAX_PROCESSES
#define MAX_TASKS               16

/* Process and Task Lists */
static process_control_block_t* process_list_head = NULL;
static process_control_block_t* current_process = NULL;
static task_control_block_t* task_list_head = NULL;
static task_control_block_t* current_task = NULL;

/* Scheduler State */
static bool scheduler_running = false;
static bool scheduler_locked = false;
static uint32_t next_process_id = 1;
static uint32_t next_task_id = 1;
static scheduler_stats_t scheduler_statistics = {0};

/* Idle Process */
static process_control_block_t idle_pcb;
static uint8_t idle_stack[1024] __attribute__((aligned(8)));

/* Forward Declarations */
static process_control_block_t* get_next_ready_process(void);
static task_control_block_t* get_next_ready_task(void);
static void add_process_to_list(process_control_block_t* pcb);
static void remove_process_from_list(process_control_block_t* pcb);
static void add_task_to_list(task_control_block_t* tcb);
static void remove_task_from_list(task_control_block_t* tcb);

/**
 * @brief Initialize process scheduler
 * 
 * Sets up scheduler data structures and creates idle process.
 * 
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t scheduler_init(void)
{
    /* Initialize scheduler state */
    process_list_head = NULL;
    current_process = NULL;
    task_list_head = NULL;
    current_task = NULL;
    scheduler_running = false;
    scheduler_locked = false;
    next_process_id = 1;
    next_task_id = 1;
    
    /* Initialize statistics */
    memset(&scheduler_statistics, 0, sizeof(scheduler_stats_t));
    
    /* Create idle process */
    memset(&idle_pcb, 0, sizeof(process_control_block_t));
    idle_pcb.process_id = IDLE_PROCESS_ID;
    strcpy(idle_pcb.name, "idle");
    idle_pcb.state = PROCESS_STATE_READY;
    idle_pcb.priority = PRIORITY_IDLE;
    idle_pcb.stack_base = (uint32_t)idle_stack;
    idle_pcb.stack_size = sizeof(idle_stack);
    idle_pcb.stack_pointer = (uint32_t*)(idle_pcb.stack_base + idle_pcb.stack_size - 4);
    idle_pcb.entry_point = idle_process;
    idle_pcb.time_slice = DEFAULT_TIME_SLICE_MS;
    idle_pcb.time_remaining = DEFAULT_TIME_SLICE_MS;
    idle_pcb.flags = PROCESS_FLAG_SYSTEM;
    
    /* Add idle process to list */
    add_process_to_list(&idle_pcb);
    current_process = &idle_pcb;
    
    return KERNEL_OK;
}

/**
 * @brief Start scheduler operation
 */
void scheduler_start(void)
{
    scheduler_running = true;
    
    /* Start with idle process */
    current_process = &idle_pcb;
    current_process->state = PROCESS_STATE_RUNNING;
    
    scheduler_statistics.scheduler_ticks = 0;
}

/**
 * @brief Scheduler tick handler (called from system tick)
 * 
 * Handles time slice management and preemptive scheduling.
 */
void scheduler_tick(void)
{
    if (!scheduler_running || scheduler_locked) {
        return;
    }
    
    scheduler_statistics.scheduler_ticks++;
    
    /* Decrement current process time slice */
    if (current_process && current_process->time_remaining > 0) {
        current_process->time_remaining--;
        current_process->total_runtime++;
    }
    
    /* Check if time slice expired or higher priority process is ready */
    if (current_process && current_process->time_remaining == 0) {
        scheduler_preempt();
    }
}

/**
 * @brief Preempt current process and schedule next
 */
void scheduler_preempt(void)
{
    if (!scheduler_running || scheduler_locked) {
        return;
    }
    
    process_control_block_t* next_process = get_next_ready_process();
    
    if (next_process && next_process != current_process) {
        /* Context switch */
        process_control_block_t* prev_process = current_process;
        
        if (prev_process && prev_process->state == PROCESS_STATE_RUNNING) {
            prev_process->state = PROCESS_STATE_READY;
        }
        
        current_process = next_process;
        current_process->state = PROCESS_STATE_RUNNING;
        current_process->time_remaining = current_process->time_slice;
        current_process->last_scheduled = scheduler_statistics.scheduler_ticks;
        
        scheduler_statistics.context_switches++;
        
        /* Perform context switch */
        context_switch(prev_process, current_process);
    } else if (current_process) {
        /* Reset time slice for current process */
        current_process->time_remaining = current_process->time_slice;
    }
}

/**
 * @brief Yield CPU voluntarily
 */
void scheduler_yield(void)
{
    if (!scheduler_running) {
        return;
    }
    
    if (current_process) {
        current_process->time_remaining = 0;
    }
    
    scheduler_preempt();
}

/**
 * @brief Create a new process
 * 
 * @param name Process name
 * @param entry_point Process entry function
 * @param stack_size Stack size in bytes
 * @param priority Process priority
 * @param flags Process flags
 * @return Process ID on success, 0 on failure
 */
uint32_t process_create(const char* name, void (*entry_point)(void), 
                       uint32_t stack_size, process_priority_t priority, uint32_t flags)
{
    if (!name || !entry_point || stack_size < 512) {
        return 0;
    }
    
    /* Allocate PCB */
    process_control_block_t* pcb = (process_control_block_t*)memory_alloc(sizeof(process_control_block_t), 0);
    if (!pcb) {
        return 0;
    }
    
    /* Allocate stack */
    uint8_t* stack = (uint8_t*)memory_alloc(stack_size, MEM_ALLOC_ZERO);
    if (!stack) {
        memory_free(pcb);
        return 0;
    }
    
    /* Initialize PCB */
    memset(pcb, 0, sizeof(process_control_block_t));
    pcb->process_id = next_process_id++;
    strncpy(pcb->name, name, sizeof(pcb->name) - 1);
    pcb->state = PROCESS_STATE_READY;
    pcb->priority = priority;
    pcb->stack_base = (uint32_t)stack;
    pcb->stack_size = stack_size;
    pcb->stack_pointer = (uint32_t*)(pcb->stack_base + stack_size - 4);
    pcb->entry_point = entry_point;
    pcb->time_slice = DEFAULT_TIME_SLICE_MS;
    pcb->time_remaining = DEFAULT_TIME_SLICE_MS;
    pcb->flags = flags;
    
    /* Initialize stack frame for context switching */
    uint32_t* sp = pcb->stack_pointer;
    *(--sp) = 0x01000000;  /* xPSR */
    *(--sp) = (uint32_t)entry_point;  /* PC */
    *(--sp) = 0;  /* LR */
    *(--sp) = 0;  /* R12 */
    *(--sp) = 0;  /* R3 */
    *(--sp) = 0;  /* R2 */
    *(--sp) = 0;  /* R1 */
    *(--sp) = 0;  /* R0 */
    *(--sp) = 0;  /* R11 */
    *(--sp) = 0;  /* R10 */
    *(--sp) = 0;  /* R9 */
    *(--sp) = 0;  /* R8 */
    *(--sp) = 0;  /* R7 */
    *(--sp) = 0;  /* R6 */
    *(--sp) = 0;  /* R5 */
    *(--sp) = 0;  /* R4 */
    
    pcb->stack_pointer = sp;
    
    /* Add to process list */
    add_process_to_list(pcb);
    
    scheduler_statistics.total_processes++;
    scheduler_statistics.active_processes++;
    
    return pcb->process_id;
}

/**
 * @brief Terminate a process
 * 
 * @param process_id Process ID to terminate
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t process_terminate(uint32_t process_id)
{
    process_control_block_t* pcb = process_get_by_id(process_id);
    if (!pcb || pcb->process_id == IDLE_PROCESS_ID) {
        return KERNEL_ERROR_INVALID_PARAM;
    }
    
    /* Remove from process list */
    remove_process_from_list(pcb);
    
    /* Free memory */
    if (pcb->stack_base) {
        memory_free((void*)pcb->stack_base);
    }
    
    /* If terminating current process, schedule next */
    if (pcb == current_process) {
        current_process = NULL;
        scheduler_preempt();
    }
    
    memory_free(pcb);
    
    scheduler_statistics.active_processes--;
    
    return KERNEL_OK;
}

/**
 * @brief Get next ready process for scheduling
 * 
 * @return Pointer to next ready process, NULL if none
 */
static process_control_block_t* get_next_ready_process(void)
{
    process_control_block_t* best_process = NULL;
    process_priority_t highest_priority = PRIORITY_IDLE;
    
    process_control_block_t* current = process_list_head;
    while (current) {
        if (current->state == PROCESS_STATE_READY && current->priority >= highest_priority) {
            highest_priority = current->priority;
            best_process = current;
        }
        current = current->next;
    }
    
    return best_process ? best_process : &idle_pcb;
}

/**
 * @brief Context switch between processes
 * 
 * @param from Process to switch from
 * @param to Process to switch to
 */
void context_switch(process_control_block_t* from, process_control_block_t* to)
{
    if (!to) {
        return;
    }
    
    /* Save current context if switching from a process */
    if (from) {
        save_context(from);
    }
    
    /* Restore new context */
    restore_context(to);
}

/**
 * @brief Save process context (implemented in assembly)
 * 
 * @param pcb Process control block to save context to
 */
void save_context(process_control_block_t* pcb)
{
    /* This would typically be implemented in assembly */
    /* For now, just save stack pointer */
    __asm__ volatile (
        "mrs %0, psp"
        : "=r" (pcb->stack_pointer)
    );
}

/**
 * @brief Restore process context (implemented in assembly)
 * 
 * @param pcb Process control block to restore context from
 */
void restore_context(process_control_block_t* pcb)
{
    /* This would typically be implemented in assembly */
    /* For now, just restore stack pointer */
    __asm__ volatile (
        "msr psp, %0"
        :
        : "r" (pcb->stack_pointer)
    );
}

/**
 * @brief Get current running process
 * 
 * @return Pointer to current process PCB
 */
process_control_block_t* process_get_current(void)
{
    return current_process;
}

/**
 * @brief Get process by ID
 * 
 * @param process_id Process ID to find
 * @return Pointer to process PCB, NULL if not found
 */
process_control_block_t* process_get_by_id(uint32_t process_id)
{
    process_control_block_t* current = process_list_head;
    while (current) {
        if (current->process_id == process_id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief Add process to process list
 * 
 * @param pcb Process control block to add
 */
static void add_process_to_list(process_control_block_t* pcb)
{
    pcb->next = process_list_head;
    pcb->prev = NULL;
    
    if (process_list_head) {
        process_list_head->prev = pcb;
    }
    
    process_list_head = pcb;
}

/**
 * @brief Remove process from process list
 * 
 * @param pcb Process control block to remove
 */
static void remove_process_from_list(process_control_block_t* pcb)
{
    if (pcb->prev) {
        pcb->prev->next = pcb->next;
    } else {
        process_list_head = pcb->next;
    }
    
    if (pcb->next) {
        pcb->next->prev = pcb->prev;
    }
}

/**
 * @brief Lock scheduler (disable preemption)
 */
void scheduler_lock(void)
{
    scheduler_locked = true;
}

/**
 * @brief Unlock scheduler (enable preemption)
 */
void scheduler_unlock(void)
{
    scheduler_locked = false;
}

/**
 * @brief Check if scheduler is locked
 * 
 * @return true if locked, false otherwise
 */
bool scheduler_is_locked(void)
{
    return scheduler_locked;
}

/**
 * @brief Get scheduler statistics
 * 
 * @return Pointer to scheduler statistics
 */
scheduler_stats_t* scheduler_get_stats(void)
{
    /* Update idle time percentage */
    if (scheduler_statistics.scheduler_ticks > 0) {
        uint32_t idle_ticks = (current_process == &idle_pcb) ? 
            scheduler_statistics.scheduler_ticks - idle_pcb.total_runtime : 0;
        scheduler_statistics.idle_time_percent = 
            (idle_ticks * 100) / scheduler_statistics.scheduler_ticks;
    }
    
    return &scheduler_statistics;
}

/**
 * @brief Idle process function
 * 
 * Low-power idle loop that runs when no other processes are ready.
 */
void idle_process(void)
{
    while (1) {
        /* Enter low-power mode */
        __asm__ volatile ("wfi");
    }
}