/**
 * @file hal_utils.c
 * @brief Hardware Abstraction Layer Utilities
 * 
 * This file implements utility functions for the HAL framework including
 * device enumeration, resource validation, and debugging helpers.
 */

#include "hal.h"
#include "hal_internal.h"
#include <string.h>

/**
 * @brief Get device count by type
 * @param type Device type
 * @return Number of devices of specified type
 */
uint32_t hal_device_get_count_by_type(hal_device_type_t type)
{
    uint32_t count = 0;
    hal_device_t *device = device_list_head;
    
    while (device != NULL) {
        if (device->type == type) {
            count++;
        }
        device = device->next;
    }
    
    return count;
}

/**
 * @brief Get all devices of a specific type
 * @param type Device type
 * @param devices Array to store device pointers
 * @param max_devices Maximum number of devices to return
 * @return Number of devices found
 */
uint32_t hal_device_get_by_type(hal_device_type_t type, hal_device_t **devices, uint32_t max_devices)
{
    if (devices == NULL || max_devices == 0) {
        return 0;
    }

    uint32_t count = 0;
    hal_device_t *device = device_list_head;
    
    while (device != NULL && count < max_devices) {
        if (device->type == type) {
            devices[count++] = device;
        }
        device = device->next;
    }
    
    return count;
}

/**
 * @brief Validate device configuration
 * @param config Device configuration
 * @return HAL_OK if valid, error code otherwise
 */
hal_result_t hal_device_validate_config(const hal_device_config_t *config)
{
    if (config == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Validate base address alignment */
    if (config->base_address & 0x3) {
        return HAL_ERROR_INVALID_PARAM;  /* Must be 4-byte aligned */
    }

    /* Validate size */
    if (config->size == 0) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Validate clock frequency */
    if (config->clock_frequency > CPU_FREQUENCY_HZ) {
        return HAL_ERROR_INVALID_PARAM;
    }

    return HAL_OK;
}

/**
 * @brief Get device type name string
 * @param type Device type
 * @return String representation of device type
 */
const char *hal_device_type_to_string(hal_device_type_t type)
{
    switch (type) {
        case HAL_DEVICE_TYPE_GPIO:    return "GPIO";
        case HAL_DEVICE_TYPE_RADIO:   return "RADIO";
        case HAL_DEVICE_TYPE_DISPLAY: return "DISPLAY";
        case HAL_DEVICE_TYPE_STORAGE: return "STORAGE";
        case HAL_DEVICE_TYPE_TIMER:   return "TIMER";
        case HAL_DEVICE_TYPE_UART:    return "UART";
        case HAL_DEVICE_TYPE_SPI:     return "SPI";
        case HAL_DEVICE_TYPE_I2C:     return "I2C";
        default:                      return "UNKNOWN";
    }
}

/**
 * @brief Get device state name string
 * @param state Device state
 * @return String representation of device state
 */
const char *hal_device_state_to_string(hal_device_state_t state)
{
    switch (state) {
        case HAL_DEVICE_STATE_UNINITIALIZED: return "UNINITIALIZED";
        case HAL_DEVICE_STATE_INITIALIZED:   return "INITIALIZED";
        case HAL_DEVICE_STATE_ACTIVE:        return "ACTIVE";
        case HAL_DEVICE_STATE_SUSPENDED:     return "SUSPENDED";
        case HAL_DEVICE_STATE_ERROR:         return "ERROR";
        default:                             return "UNKNOWN";
    }
}

/**
 * @brief Get resource type name string
 * @param type Resource type
 * @return String representation of resource type
 */
const char *hal_resource_type_to_string(hal_resource_type_t type)
{
    switch (type) {
        case HAL_RESOURCE_TYPE_MEMORY:    return "MEMORY";
        case HAL_RESOURCE_TYPE_INTERRUPT: return "INTERRUPT";
        case HAL_RESOURCE_TYPE_DMA:       return "DMA";
        case HAL_RESOURCE_TYPE_CLOCK:     return "CLOCK";
        case HAL_RESOURCE_TYPE_PIN:       return "PIN";
        default:                          return "UNKNOWN";
    }
}

/**
 * @brief Calculate resource usage statistics
 * @param type Resource type (HAL_RESOURCE_TYPE_MAX for all types)
 * @param total_count Pointer to store total resource count
 * @param used_count Pointer to store used resource count
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_resource_get_usage_stats(hal_resource_type_t type, uint32_t *total_count, uint32_t *used_count)
{
    if (total_count == NULL || used_count == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    *total_count = 0;
    *used_count = 0;

    hal_resource_t *resource = resource_list_head;
    
    while (resource != NULL) {
        if (type == HAL_RESOURCE_TYPE_MAX || resource->type == type) {
            (*total_count)++;
            if (resource->in_use) {
                (*used_count)++;
            }
        }
        resource = resource->next;
    }
    
    return HAL_OK;
}