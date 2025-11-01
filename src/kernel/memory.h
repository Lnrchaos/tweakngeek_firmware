/**
 * @file memory.h
 * @brief TweaknGeek Memory Management Definitions
 * 
 * This file contains memory management data structures, constants,
 * and function declarations for the TweaknGeek kernel.
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include "kernel.h"

/* Memory Protection Flags */
#define MEM_PROT_READ       (1 << 0)
#define MEM_PROT_WRITE      (1 << 1)
#define MEM_PROT_EXEC       (1 << 2)
#define MEM_PROT_USER       (1 << 3)
#define MEM_PROT_KERNEL     (1 << 4)

/* Memory Allocation Flags */
#define MEM_ALLOC_ZERO      (1 << 0)
#define MEM_ALLOC_ALIGN     (1 << 1)
#define MEM_ALLOC_DMA       (1 << 2)

/* Memory Block Header */
typedef struct memory_block {
    uint32_t size;
    uint32_t flags;
    bool is_free;
    struct memory_block* next;
    struct memory_block* prev;
    uint32_t magic;
} memory_block_t;

/* Memory Region Descriptor */
typedef struct {
    uint32_t start_addr;
    uint32_t size;
    uint32_t protection;
    bool is_allocated;
    uint32_t owner_process;
} memory_region_t;

/* Memory Statistics */
typedef struct {
    uint32_t total_memory;
    uint32_t free_memory;
    uint32_t used_memory;
    uint32_t largest_free_block;
    uint32_t num_allocations;
    uint32_t num_free_blocks;
    uint32_t fragmentation_percent;
} memory_stats_t;

/* Memory Management Functions */
kernel_status_t memory_init(void);
void* memory_alloc(uint32_t size, uint32_t flags);
void memory_free(void* ptr);
void* memory_realloc(void* ptr, uint32_t new_size);
kernel_status_t memory_protect(void* addr, uint32_t size, uint32_t protection);
memory_stats_t* memory_get_stats(void);

/* Stack Management Functions */
kernel_status_t stack_init(void);
bool stack_check_overflow(void* stack_ptr, uint32_t stack_size);
void stack_set_guard(void* stack_base, uint32_t stack_size);

/* Heap Management Functions */
kernel_status_t heap_init(void* heap_start, uint32_t heap_size);
void heap_defragment(void);
bool heap_validate(void);

/* Memory Mapping Functions */
void* memory_map_physical(uint32_t physical_addr, uint32_t size, uint32_t flags);
kernel_status_t memory_unmap(void* virtual_addr, uint32_t size);

/* Debug Functions */
void memory_dump_blocks(void);
void memory_dump_regions(void);

#endif /* MEMORY_H */