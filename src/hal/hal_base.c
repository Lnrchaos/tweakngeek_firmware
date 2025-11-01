/**
 * @file hal_base.c
 * @brief Hardware Abstraction Layer Base Implementation
 * 
 * This file implements the base HAL framework including device and driver
 * registration, hardware resource management, and core HAL functionality.
 */

#include "hal.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

/* HAL framework state */
static bool hal_initialized = false;

/* Driver and device lists */
static hal_driver_t *driver_list_head = NULL;
static hal_device_t *device_list_head = NULL;
static hal_resource_t *resource_list_head = NULL;

/* Resource ID counter */
static uint32_t next_resource_id = 1;
static uint32_t next_device_id = 1;

/**
 * @brief HAL framework initialization
 */
hal_result_t hal_init(void)
{
    if (hal_initialized) {
        return HAL_OK;
    }

    /* Initialize lists */
    driver_list_head = NULL;
    device_list_head = NULL;
    resource_list_head = NULL;
    
    /* Reset ID counters */
    next_resource_id = 1;
    next_device_id = 1;

    hal_initialized = true;
    return HAL_OK;
}

/**
 * @brief HAL framework deinitialization
 */
hal_result_t hal_deinit(void)
{
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    /* Cleanup all devices */
    hal_device_t *device = device_list_head;
    while (device != NULL) {
        hal_device_t *next = device->next;
        if (device->driver && device->driver->ops && device->driver->ops->deinit) {
            device->driver->ops->deinit(device);
        }
        device = next;
    }

    /* Clear lists */
    driver_list_head = NULL;
    device_list_head = NULL;
    resource_list_head = NULL;

    hal_initialized = false;
    return HAL_OK;
}

/**
 * @brief Register a HAL driver
 */
hal_result_t hal_driver_register(hal_driver_t *driver)
{
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (driver == NULL || driver->name == NULL || driver->ops == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Check if driver already registered */
    if (hal_driver_find(driver->name) != NULL) {
        return HAL_ERROR_RESOURCE_BUSY;
    }

    /* Add to driver list */
    driver->next = driver_list_head;
    driver_list_head = driver;

    return HAL_OK;
}

/**
 * @brief Unregister a HAL driver
 */
hal_result_t hal_driver_unregister(hal_driver_t *driver)
{
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (driver == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Find and remove from driver list */
    hal_driver_t **current = &driver_list_head;
    while (*current != NULL) {
        if (*current == driver) {
            *current = driver->next;
            driver->next = NULL;
            return HAL_OK;
        }
        current = &((*current)->next);
    }

    return HAL_ERROR_RESOURCE_NOT_FOUND;
}

/**
 * @brief Find a driver by name
 */
hal_driver_t *hal_driver_find(const char *name)
{
    if (!hal_initialized || name == NULL) {
        return NULL;
    }

    hal_driver_t *driver = driver_list_head;
    while (driver != NULL) {
        if (strcmp(driver->name, name) == 0) {
            return driver;
        }
        driver = driver->next;
    }

    return NULL;
}

/**
 * @brief Register a HAL device
 */
hal_result_t hal_device_register(hal_device_t *device)
{
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (device == NULL || device->name == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Assign device ID if not set */
    if (device->device_id == 0) {
        device->device_id = next_device_id++;
    }

    /* Check if device already registered */
    if (hal_device_find_by_id(device->device_id) != NULL) {
        return HAL_ERROR_RESOURCE_BUSY;
    }

    /* Initialize device state */
    device->state = HAL_DEVICE_STATE_UNINITIALIZED;
    device->ref_count = 0;

    /* Add to device list */
    device->next = device_list_head;
    device_list_head = device;

    /* Initialize device if driver is available */
    if (device->driver && device->driver->ops && device->driver->ops->init) {
        hal_result_t result = device->driver->ops->init(device);
        if (result == HAL_OK) {
            device->state = HAL_DEVICE_STATE_INITIALIZED;
        } else {
            device->state = HAL_DEVICE_STATE_ERROR;
        }
    }

    return HAL_OK;
}

/**
 * @brief Unregister a HAL device
 */
hal_result_t hal_device_unregister(hal_device_t *device)
{
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (device == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Check if device is in use */
    if (device->ref_count > 0) {
        return HAL_ERROR_RESOURCE_BUSY;
    }

    /* Deinitialize device */
    if (device->driver && device->driver->ops && device->driver->ops->deinit) {
        device->driver->ops->deinit(device);
    }

    /* Find and remove from device list */
    hal_device_t **current = &device_list_head;
    while (*current != NULL) {
        if (*current == device) {
            *current = device->next;
            device->next = NULL;
            device->state = HAL_DEVICE_STATE_UNINITIALIZED;
            return HAL_OK;
        }
        current = &((*current)->next);
    }

    return HAL_ERROR_RESOURCE_NOT_FOUND;
}

/**
 * @brief Find a device by ID
 */
hal_device_t *hal_device_find_by_id(uint32_t device_id)
{
    if (!hal_initialized) {
        return NULL;
    }

    hal_device_t *device = device_list_head;
    while (device != NULL) {
        if (device->device_id == device_id) {
            return device;
        }
        device = device->next;
    }

    return NULL;
}

/**
 * @brief Find a device by name
 */
hal_device_t *hal_device_find_by_name(const char *name)
{
    if (!hal_initialized || name == NULL) {
        return NULL;
    }

    hal_device_t *device = device_list_head;
    while (device != NULL) {
        if (strcmp(device->name, name) == 0) {
            return device;
        }
        device = device->next;
    }

    return NULL;
}

/**
 * @brief Open a device
 */
hal_result_t hal_device_open(uint32_t device_id, uint32_t flags)
{
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    hal_device_t *device = hal_device_find_by_id(device_id);
    if (device == NULL) {
        return HAL_ERROR_RESOURCE_NOT_FOUND;
    }

    if (device->state != HAL_DEVICE_STATE_INITIALIZED && 
        device->state != HAL_DEVICE_STATE_ACTIVE) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    /* Call driver open function */
    if (device->driver && device->driver->ops && device->driver->ops->open) {
        hal_result_t result = device->driver->ops->open(device, flags);
        if (result != HAL_OK) {
            return result;
        }
    }

    /* Increment reference count and update state */
    device->ref_count++;
    device->state = HAL_DEVICE_STATE_ACTIVE;

    return HAL_OK;
}

/**
 * @brief Close a device
 */
hal_result_t hal_device_close(uint32_t device_id)
{
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    hal_device_t *device = hal_device_find_by_id(device_id);
    if (device == NULL) {
        return HAL_ERROR_RESOURCE_NOT_FOUND;
    }

    if (device->ref_count == 0) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Decrement reference count */
    device->ref_count--;

    /* Call driver close function if no more references */
    if (device->ref_count == 0) {
        if (device->driver && device->driver->ops && device->driver->ops->close) {
            device->driver->ops->close(device);
        }
        device->state = HAL_DEVICE_STATE_INITIALIZED;
    }

    return HAL_OK;
}

/**
 * @brief Allocate a hardware resource
 */
hal_result_t hal_resource_allocate(hal_resource_type_t type, uint32_t size, 
                                   uint32_t flags, uint32_t *resource_id)
{
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (resource_id == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Create new resource */
    hal_resource_t *resource = (hal_resource_t *)malloc(sizeof(hal_resource_t));
    if (resource == NULL) {
        return HAL_ERROR_NO_MEMORY;
    }

    /* Initialize resource */
    resource->resource_id = next_resource_id++;
    resource->type = type;
    resource->size = size;
    resource->access_flags = flags;
    resource->in_use = true;
    resource->owner_device_id = 0;  /* Will be set when assigned to device */
    
    /* Add to resource list */
    resource->next = resource_list_head;
    resource_list_head = resource;

    *resource_id = resource->resource_id;
    return HAL_OK;
}

/**
 * @brief Free a hardware resource
 */
hal_result_t hal_resource_free(uint32_t resource_id)
{
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    /* Find and remove from resource list */
    hal_resource_t **current = &resource_list_head;
    while (*current != NULL) {
        if ((*current)->resource_id == resource_id) {
            hal_resource_t *resource = *current;
            *current = resource->next;
            free(resource);
            return HAL_OK;
        }
        current = &((*current)->next);
    }

    return HAL_ERROR_RESOURCE_NOT_FOUND;
}

/**
 * @brief Get resource information
 */
hal_result_t hal_resource_get_info(uint32_t resource_id, hal_resource_t *resource)
{
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (resource == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    hal_resource_t *res = resource_list_head;
    while (res != NULL) {
        if (res->resource_id == resource_id) {
            *resource = *res;
            return HAL_OK;
        }
        res = res->next;
    }

    return HAL_ERROR_RESOURCE_NOT_FOUND;
}

/**
 * @brief Check if resource is available
 */
bool hal_resource_is_available(hal_resource_type_t type, uint32_t base_address, uint32_t size)
{
    if (!hal_initialized) {
        return false;
    }

    hal_resource_t *resource = resource_list_head;
    while (resource != NULL) {
        if (resource->type == type && resource->in_use) {
            /* Check for address range overlap */
            uint32_t res_end = resource->base_address + resource->size;
            uint32_t req_end = base_address + size;
            
            if (!(base_address >= res_end || req_end <= resource->base_address)) {
                return false;  /* Overlap detected */
            }
        }
        resource = resource->next;
    }

    return true;  /* No conflicts found */
}