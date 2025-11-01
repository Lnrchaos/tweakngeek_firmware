/**
 * @file hal.h
 * @brief Hardware Abstraction Layer Interface
 * 
 * This file defines the base HAL framework including interface structures,
 * function pointers, hardware resource management, and device driver
 * registration system for the TweaknGeek firmware.
 */

#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stdbool.h>
#include "tweakngeek_config.h"

/* Forward declarations */
typedef struct hal_device hal_device_t;
typedef struct hal_driver hal_driver_t;
typedef struct hal_resource hal_resource_t;

/**
 * @brief HAL device types
 */
typedef enum {
    HAL_DEVICE_TYPE_GPIO = 0,
    HAL_DEVICE_TYPE_RADIO,
    HAL_DEVICE_TYPE_DISPLAY,
    HAL_DEVICE_TYPE_STORAGE,
    HAL_DEVICE_TYPE_TIMER,
    HAL_DEVICE_TYPE_UART,
    HAL_DEVICE_TYPE_SPI,
    HAL_DEVICE_TYPE_I2C,
    HAL_DEVICE_TYPE_MAX
} hal_device_type_t;

/**
 * @brief HAL resource types
 */
typedef enum {
    HAL_RESOURCE_TYPE_MEMORY = 0,
    HAL_RESOURCE_TYPE_INTERRUPT,
    HAL_RESOURCE_TYPE_DMA,
    HAL_RESOURCE_TYPE_CLOCK,
    HAL_RESOURCE_TYPE_PIN,
    HAL_RESOURCE_TYPE_MAX
} hal_resource_type_t;

/**
 * @brief HAL device states
 */
typedef enum {
    HAL_DEVICE_STATE_UNINITIALIZED = 0,
    HAL_DEVICE_STATE_INITIALIZED,
    HAL_DEVICE_STATE_ACTIVE,
    HAL_DEVICE_STATE_SUSPENDED,
    HAL_DEVICE_STATE_ERROR,
    HAL_DEVICE_STATE_MAX
} hal_device_state_t;

/**
 * @brief HAL operation result codes
 */
typedef enum {
    HAL_OK = 0,
    HAL_ERROR = -1,
    HAL_ERROR_INVALID_PARAM = -2,
    HAL_ERROR_NOT_INITIALIZED = -3,
    HAL_ERROR_RESOURCE_BUSY = -4,
    HAL_ERROR_RESOURCE_NOT_FOUND = -5,
    HAL_ERROR_TIMEOUT = -6,
    HAL_ERROR_NO_MEMORY = -7,
    HAL_ERROR_NOT_SUPPORTED = -8
} hal_result_t;

/**
 * @brief HAL device configuration structure
 */
typedef struct {
    uint32_t base_address;      /**< Device base address */
    uint32_t size;              /**< Device memory size */
    uint32_t irq_number;        /**< Interrupt number */
    uint32_t clock_frequency;   /**< Operating frequency */
    uint32_t flags;             /**< Configuration flags */
    void *private_data;         /**< Driver-specific data */
} hal_device_config_t;

/**
 * @brief HAL driver operations structure
 */
typedef struct {
    hal_result_t (*init)(hal_device_t *device);
    hal_result_t (*deinit)(hal_device_t *device);
    hal_result_t (*open)(hal_device_t *device, uint32_t flags);
    hal_result_t (*close)(hal_device_t *device);
    hal_result_t (*read)(hal_device_t *device, void *buffer, uint32_t size, uint32_t *bytes_read);
    hal_result_t (*write)(hal_device_t *device, const void *buffer, uint32_t size, uint32_t *bytes_written);
    hal_result_t (*ioctl)(hal_device_t *device, uint32_t cmd, void *arg);
    hal_result_t (*suspend)(hal_device_t *device);
    hal_result_t (*resume)(hal_device_t *device);
} hal_driver_ops_t;

/**
 * @brief HAL driver structure
 */
struct hal_driver {
    const char *name;                   /**< Driver name */
    hal_device_type_t type;             /**< Device type */
    uint32_t version;                   /**< Driver version */
    const hal_driver_ops_t *ops;        /**< Driver operations */
    struct hal_driver *next;            /**< Next driver in list */
};

/**
 * @brief HAL device structure
 */
struct hal_device {
    uint32_t device_id;                 /**< Unique device ID */
    const char *name;                   /**< Device name */
    hal_device_type_t type;             /**< Device type */
    hal_device_state_t state;           /**< Current state */
    hal_device_config_t config;         /**< Device configuration */
    const hal_driver_t *driver;         /**< Associated driver */
    void *private_data;                 /**< Device-specific data */
    uint32_t ref_count;                 /**< Reference count */
    struct hal_device *next;            /**< Next device in list */
};

/**
 * @brief HAL resource structure
 */
struct hal_resource {
    uint32_t resource_id;               /**< Unique resource ID */
    hal_resource_type_t type;           /**< Resource type */
    uint32_t base_address;              /**< Base address */
    uint32_t size;                      /**< Resource size */
    uint32_t access_flags;              /**< Access permissions */
    bool in_use;                        /**< Usage status */
    uint32_t owner_device_id;           /**< Owner device ID */
    struct hal_resource *next;          /**< Next resource in list */
};

/**
 * @brief HAL framework initialization
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_init(void);

/**
 * @brief HAL framework deinitialization
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_deinit(void);

/**
 * @brief Register a HAL driver
 * @param driver Pointer to driver structure
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_driver_register(hal_driver_t *driver);

/**
 * @brief Unregister a HAL driver
 * @param driver Pointer to driver structure
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_driver_unregister(hal_driver_t *driver);

/**
 * @brief Find a driver by name
 * @param name Driver name
 * @return Pointer to driver or NULL if not found
 */
hal_driver_t *hal_driver_find(const char *name);

/**
 * @brief Register a HAL device
 * @param device Pointer to device structure
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_device_register(hal_device_t *device);

/**
 * @brief Unregister a HAL device
 * @param device Pointer to device structure
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_device_unregister(hal_device_t *device);

/**
 * @brief Find a device by ID
 * @param device_id Device ID
 * @return Pointer to device or NULL if not found
 */
hal_device_t *hal_device_find_by_id(uint32_t device_id);

/**
 * @brief Find a device by name
 * @param name Device name
 * @return Pointer to device or NULL if not found
 */
hal_device_t *hal_device_find_by_name(const char *name);

/**
 * @brief Open a device
 * @param device_id Device ID
 * @param flags Open flags
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_device_open(uint32_t device_id, uint32_t flags);

/**
 * @brief Close a device
 * @param device_id Device ID
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_device_close(uint32_t device_id);

/**
 * @brief Allocate a hardware resource
 * @param type Resource type
 * @param size Resource size
 * @param flags Allocation flags
 * @param resource_id Pointer to store allocated resource ID
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_resource_allocate(hal_resource_type_t type, uint32_t size, 
                                   uint32_t flags, uint32_t *resource_id);

/**
 * @brief Free a hardware resource
 * @param resource_id Resource ID
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_resource_free(uint32_t resource_id);

/**
 * @brief Get resource information
 * @param resource_id Resource ID
 * @param resource Pointer to store resource information
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_resource_get_info(uint32_t resource_id, hal_resource_t *resource);

/**
 * @brief Check if resource is available
 * @param type Resource type
 * @param base_address Base address
 * @param size Resource size
 * @return true if available, false otherwise
 */
bool hal_resource_is_available(hal_resource_type_t type, uint32_t base_address, uint32_t size);

#endif /* HAL_H */