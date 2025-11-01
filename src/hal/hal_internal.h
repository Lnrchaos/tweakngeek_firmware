/**
 * @file hal_internal.h
 * @brief HAL Internal Definitions
 * 
 * This file contains internal HAL definitions and declarations
 * that are shared between HAL implementation files but not
 * exposed to external users.
 */

#ifndef HAL_INTERNAL_H
#define HAL_INTERNAL_H

#include "hal.h"

/* Internal HAL state variables */
extern hal_driver_t *driver_list_head;
extern hal_device_t *device_list_head;
extern hal_resource_t *resource_list_head;
extern bool hal_initialized;

/* Internal helper functions */

/**
 * @brief Get pointer to device list head
 * @return Pointer to first device in list
 */
hal_device_t *hal_internal_get_device_list(void);

/**
 * @brief Get pointer to resource list head
 * @return Pointer to first resource in list
 */
hal_resource_t *hal_internal_get_resource_list(void);

/**
 * @brief Check if HAL is initialized
 * @return true if initialized, false otherwise
 */
bool hal_internal_is_initialized(void);

#endif /* HAL_INTERNAL_H */