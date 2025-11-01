/**
 * @file boot.h
 * @brief TweaknGeek Kernel Boot Sequence Definitions
 * 
 * This file contains boot sequence specific definitions and functions.
 */

#ifndef BOOT_H
#define BOOT_H

#include "kernel.h"

/* Boot sequence functions */
bool boot_has_errors(void);
uint32_t boot_get_elapsed_time(void);
void boot_init_timing(void);

#endif /* BOOT_H */