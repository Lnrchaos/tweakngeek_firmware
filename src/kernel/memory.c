/**
 * @file memory.c
 * @brief TweaknGeek Memory Management Implementation
 * 
 * This file implements the memory management system including heap
 * allocation, memory protection, and stack overflow protection.
 */

#include "memory.h"
#include <string.h>

/* Memory block magic number for corruption detection */
#define MEMORY_BLOCK_MAGIC      0xDEADBEEF
#define MEMORY_FREE_MAGIC       0xFEEDFACE

/* Memory alignment */
#define MEMORY_ALIGNMENT        8
#define ALIGN_SIZE(size)        (((size) + MEMORY_ALIGNMENT - 1) & ~(MEMORY_ALIGNMENT - 1))

/* Heap configuration */
static uint8_t heap_memory[HEAP_SIZE] __attribute__((aligned(8)));
static memory_block_t* heap_head = NULL;
static memory_stats_t memory_statistics = {0};
static bool memory_initialized = false;

/* Stack guard configuration */
#define STACK_GUARD_PATTERN     0xDEADC0DE
static uint32_t* stack_guard_base = NULL;
static uint32_t stack_guard_size = 0;

/* Memory region tracking */
#define MAX_MEMORY_REGIONS      32
static memory_region_t memory_regions[MAX_MEMORY_REGIONS];
static uint32_t num_memory_regions = 0;

/**
 * @brief Initialize memory management system
 * 
 * Sets up heap, stack protection, and memory tracking structures.
 * 
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t memory_init(void)
{
    if (memory_initialized) {
        return KERNEL_OK;
    }
    
    /* Initialize heap */
    kernel_status_t status = heap_init(heap_memory, HEAP_SIZE);
    if (status != KERNEL_OK) {
        return status;
    }
    
    /* Initialize stack protection */
    status = stack_init();
    if (status != KERNEL_OK) {
        return status;
    }
    
    /* Initialize memory statistics */
    memory_statistics.total_memory = HEAP_SIZE;
    memory_statistics.free_memory = HEAP_SIZE - sizeof(memory_block_t);
    memory_statistics.used_memory = sizeof(memory_block_t);
    memory_statistics.largest_free_block = HEAP_SIZE - sizeof(memory_block_t);
    memory_statistics.num_allocations = 0;
    memory_statistics.num_free_blocks = 1;
    memory_statistics.fragmentation_percent = 0;
    
    /* Initialize memory regions */
    memset(memory_regions, 0, sizeof(memory_regions));
    num_memory_regions = 0;
    
    memory_initialized = true;
    return KERNEL_OK;
}

/**
 * @brief Initialize heap allocator
 * 
 * @param heap_start Pointer to heap memory start
 * @param heap_size Size of heap in bytes
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t heap_init(void* heap_start, uint32_t heap_size)
{
    if (!heap_start || heap_size < sizeof(memory_block_t)) {
        return KERNEL_ERROR_INVALID_PARAM;
    }
    
    /* Initialize first free block */
    heap_head = (memory_block_t*)heap_start;
    heap_head->size = heap_size - sizeof(memory_block_t);
    heap_head->flags = 0;
    heap_head->is_free = true;
    heap_head->next = NULL;
    heap_head->prev = NULL;
    heap_head->magic = MEMORY_FREE_MAGIC;
    
    return KERNEL_OK;
}

/**
 * @brief Allocate memory from heap
 * 
 * @param size Size in bytes to allocate
 * @param flags Allocation flags
 * @return Pointer to allocated memory, NULL on failure
 */
void* memory_alloc(uint32_t size, uint32_t flags)
{
    if (!memory_initialized || size == 0) {
        return NULL;
    }
    
    /* Align size to memory boundary */
    size = ALIGN_SIZE(size);
    
    /* Find suitable free block */
    memory_block_t* current = heap_head;
    while (current) {
        if (current->is_free && current->size >= size) {
            break;
        }
        current = current->next;
    }
    
    if (!current) {
        return NULL; /* No suitable block found */
    }
    
    /* Split block if it's significantly larger */
    if (current->size > size + sizeof(memory_block_t) + MEMORY_ALIGNMENT) {
        memory_block_t* new_block = (memory_block_t*)((uint8_t*)current + sizeof(memory_block_t) + size);
        new_block->size = current->size - size - sizeof(memory_block_t);
        new_block->flags = 0;
        new_block->is_free = true;
        new_block->next = current->next;
        new_block->prev = current;
        new_block->magic = MEMORY_FREE_MAGIC;
        
        if (current->next) {
            current->next->prev = new_block;
        }
        current->next = new_block;
        current->size = size;
    }
    
    /* Mark block as allocated */
    current->is_free = false;
    current->flags = flags;
    current->magic = MEMORY_BLOCK_MAGIC;
    
    /* Update statistics */
    memory_statistics.num_allocations++;
    memory_statistics.used_memory += current->size + sizeof(memory_block_t);
    memory_statistics.free_memory -= current->size + sizeof(memory_block_t);
    
    /* Get pointer to user data */
    void* user_ptr = (uint8_t*)current + sizeof(memory_block_t);
    
    /* Zero memory if requested */
    if (flags & MEM_ALLOC_ZERO) {
        memset(user_ptr, 0, size);
    }
    
    return user_ptr;
}

/**
 * @brief Free allocated memory
 * 
 * @param ptr Pointer to memory to free
 */
void memory_free(void* ptr)
{
    if (!ptr || !memory_initialized) {
        return;
    }
    
    /* Get block header */
    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    
    /* Validate block */
    if (block->magic != MEMORY_BLOCK_MAGIC || block->is_free) {
        return; /* Invalid or already freed block */
    }
    
    /* Mark as free */
    block->is_free = true;
    block->magic = MEMORY_FREE_MAGIC;
    
    /* Update statistics */
    memory_statistics.used_memory -= block->size + sizeof(memory_block_t);
    memory_statistics.free_memory += block->size + sizeof(memory_block_t);
    
    /* Coalesce with next block if free */
    if (block->next && block->next->is_free) {
        block->size += block->next->size + sizeof(memory_block_t);
        if (block->next->next) {
            block->next->next->prev = block;
        }
        block->next = block->next->next;
    }
    
    /* Coalesce with previous block if free */
    if (block->prev && block->prev->is_free) {
        block->prev->size += block->size + sizeof(memory_block_t);
        if (block->next) {
            block->next->prev = block->prev;
        }
        block->prev->next = block->next;
    }
}

/**
 * @brief Reallocate memory block
 * 
 * @param ptr Pointer to existing memory block
 * @param new_size New size in bytes
 * @return Pointer to reallocated memory, NULL on failure
 */
void* memory_realloc(void* ptr, uint32_t new_size)
{
    if (!ptr) {
        return memory_alloc(new_size, 0);
    }
    
    if (new_size == 0) {
        memory_free(ptr);
        return NULL;
    }
    
    /* Get current block */
    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    
    if (block->magic != MEMORY_BLOCK_MAGIC || block->is_free) {
        return NULL; /* Invalid block */
    }
    
    /* If new size fits in current block, just return */
    if (new_size <= block->size) {
        return ptr;
    }
    
    /* Allocate new block and copy data */
    void* new_ptr = memory_alloc(new_size, block->flags);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        memory_free(ptr);
    }
    
    return new_ptr;
}

/**
 * @brief Set memory protection for a region
 * 
 * @param addr Start address of region
 * @param size Size of region in bytes
 * @param protection Protection flags
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t memory_protect(void* addr, uint32_t size, uint32_t protection)
{
    if (!addr || size == 0 || num_memory_regions >= MAX_MEMORY_REGIONS) {
        return KERNEL_ERROR_INVALID_PARAM;
    }
    
    /* Add memory region */
    memory_region_t* region = &memory_regions[num_memory_regions];
    region->start_addr = (uint32_t)addr;
    region->size = size;
    region->protection = protection;
    region->is_allocated = true;
    region->owner_process = 0; /* Kernel for now */
    
    num_memory_regions++;
    
    /* TODO: Configure MPU if available */
    
    return KERNEL_OK;
}

/**
 * @brief Initialize stack protection
 * 
 * @return KERNEL_OK on success, error code on failure
 */
kernel_status_t stack_init(void)
{
    /* Set up stack guard at the end of stack */
    extern uint32_t _estack;
    stack_guard_base = &_estack - (STACK_SIZE / sizeof(uint32_t));
    stack_guard_size = 16; /* 16 words guard */
    
    /* Fill guard area with pattern */
    for (uint32_t i = 0; i < stack_guard_size; i++) {
        stack_guard_base[i] = STACK_GUARD_PATTERN;
    }
    
    return KERNEL_OK;
}

/**
 * @brief Check for stack overflow
 * 
 * @param stack_ptr Current stack pointer
 * @param stack_size Stack size in bytes
 * @return true if overflow detected, false otherwise
 */
bool stack_check_overflow(void* stack_ptr, uint32_t stack_size)
{
    if (!stack_guard_base) {
        return false;
    }
    
    /* Check if stack pointer is in guard area */
    uint32_t* sp = (uint32_t*)stack_ptr;
    if (sp <= stack_guard_base + stack_guard_size) {
        return true;
    }
    
    /* Check guard pattern integrity */
    for (uint32_t i = 0; i < stack_guard_size; i++) {
        if (stack_guard_base[i] != STACK_GUARD_PATTERN) {
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Get memory statistics
 * 
 * @return Pointer to memory statistics structure
 */
memory_stats_t* memory_get_stats(void)
{
    if (!memory_initialized) {
        return NULL;
    }
    
    /* Update dynamic statistics */
    memory_block_t* current = heap_head;
    uint32_t free_blocks = 0;
    uint32_t largest_free = 0;
    
    while (current) {
        if (current->is_free) {
            free_blocks++;
            if (current->size > largest_free) {
                largest_free = current->size;
            }
        }
        current = current->next;
    }
    
    memory_statistics.num_free_blocks = free_blocks;
    memory_statistics.largest_free_block = largest_free;
    
    /* Calculate fragmentation percentage */
    if (memory_statistics.free_memory > 0) {
        memory_statistics.fragmentation_percent = 
            ((memory_statistics.free_memory - largest_free) * 100) / memory_statistics.free_memory;
    }
    
    return &memory_statistics;
}

/**
 * @brief Validate heap integrity
 * 
 * @return true if heap is valid, false if corrupted
 */
bool heap_validate(void)
{
    if (!memory_initialized) {
        return false;
    }
    
    memory_block_t* current = heap_head;
    while (current) {
        /* Check magic number */
        uint32_t expected_magic = current->is_free ? MEMORY_FREE_MAGIC : MEMORY_BLOCK_MAGIC;
        if (current->magic != expected_magic) {
            return false;
        }
        
        /* Check size alignment */
        if (current->size % MEMORY_ALIGNMENT != 0) {
            return false;
        }
        
        current = current->next;
    }
    
    return true;
}