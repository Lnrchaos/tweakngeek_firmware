/**
 * @file scheduler.h
 * @brief TweaknGeek Process Scheduler Definitions
 * 
 * This file contains process scheduler data structures, constants,
 * and function declarations for the TweaknGeek kernel.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "kernel.h"

/* Process States */
typedef enum {
    PROCESS_STATE_READY = 0,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_SUSPENDED,
    PROCESS_STATE_TERMINATED
} process_state_t;

/* Process Priority Levels */
typedef enum {
    PRIORITY_IDLE = 0,
    PRIORITY_LOW = 1,
    PRIORITY_NORMAL = 2,
    PRIORITY_HIGH = 3,
    PRIORITY_CRITICAL = 4
} process_priority_t;

/* Process Control Block (PCB) */
typedef struct process_control_block {
    uint32_t process_id;
    char name[32];
    process_state_t state;
    process_priority_t priority;
    
    /* CPU Context */
    uint32_t* stack_pointer;
    uint32_t stack_base;
    uint32_t stack_size;
    uint32_t registers[16];  /* ARM Cortex-M4 registers */
    
    /* Memory Management */
    uint32_t memory_base;
    uint32_t memory_size;
    uint32_t heap_pointer;
    
    /* Timing */
    uint32_t time_slice;
    uint32_t time_remaining;
    uint32_t total_runtime;
    uint32_t last_scheduled;
    
    /* Linked List */
    struct process_control_block* next;
    struct process_control_block* prev;
    
    /* Process Entry Point */
    void (*entry_point)(void);
    void* entry_param;
    
    /* Process Flags */
    uint32_t flags;
} process_control_block_t;

/* Task Control Block (TCB) - Lightweight tasks */
typedef struct task_control_block {
    uint32_t task_id;
    char name[16];
    process_state_t state;
    process_priority_t priority;
    
    /* Task Function */
    void (*task_function)(void* param);
    void* task_param;
    
    /* Stack */
    uint32_t* stack_pointer;
    uint32_t stack_size;
    
    /* Timing */
    uint32_t time_slice;
    uint32_t time_remaining;
    
    /* Linked List */
    struct task_control_block* next;
    
    /* Task Flags */
    uint32_t flags;
} task_control_block_t;

/* Scheduler Statistics */
typedef struct {
    uint32_t total_processes;
    uint32_t active_processes;
    uint32_t total_tasks;
    uint32_t active_tasks;
    uint32_t context_switches;
    uint32_t scheduler_ticks;
    uint32_t idle_time_percent;
} scheduler_stats_t;

/* Process/Task Creation Flags */
#define PROCESS_FLAG_SYSTEM     (1 << 0)
#define PROCESS_FLAG_USER       (1 << 1)
#define PROCESS_FLAG_REALTIME   (1 << 2)
#define PROCESS_FLAG_SUSPENDED  (1 << 3)

/* Scheduler Functions */
kernel_status_t scheduler_init(void);
void scheduler_start(void);
void scheduler_tick(void);
void scheduler_yield(void);
void scheduler_preempt(void);

/* Process Management */
uint32_t process_create(const char* name, void (*entry_point)(void), 
                       uint32_t stack_size, process_priority_t priority, uint32_t flags);
kernel_status_t process_terminate(uint32_t process_id);
kernel_status_t process_suspend(uint32_t process_id);
kernel_status_t process_resume(uint32_t process_id);
process_control_block_t* process_get_current(void);
process_control_block_t* process_get_by_id(uint32_t process_id);

/* Task Management */
uint32_t task_create(const char* name, void (*task_function)(void* param), 
                    void* param, uint32_t stack_size, process_priority_t priority);
kernel_status_t task_delete(uint32_t task_id);
task_control_block_t* task_get_current(void);

/* Context Switching */
void context_switch(process_control_block_t* from, process_control_block_t* to);
void save_context(process_control_block_t* pcb);
void restore_context(process_control_block_t* pcb);

/* Scheduler Control */
void scheduler_lock(void);
void scheduler_unlock(void);
bool scheduler_is_locked(void);

/* Statistics */
scheduler_stats_t* scheduler_get_stats(void);

/* Idle Process */
void idle_process(void);

#endif /* SCHEDULER_H */