/**
 * @file hal_gpio.h
 * @brief GPIO Hardware Abstraction Layer Interface
 * 
 * This file defines the GPIO HAL interface for pin configuration,
 * control functions, interrupt-driven GPIO handling, and dynamic
 * pin function assignment for the TweaknGeek firmware.
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "hal.h"

/**
 * @brief GPIO pin modes
 */
typedef enum {
    HAL_GPIO_MODE_INPUT = 0,        /**< Input mode */
    HAL_GPIO_MODE_OUTPUT,           /**< Output mode */
    HAL_GPIO_MODE_ALTERNATE,        /**< Alternate function mode */
    HAL_GPIO_MODE_ANALOG,           /**< Analog mode */
    HAL_GPIO_MODE_MAX
} hal_gpio_mode_t;

/**
 * @brief GPIO pin pull configurations
 */
typedef enum {
    HAL_GPIO_PULL_NONE = 0,         /**< No pull resistor */
    HAL_GPIO_PULL_UP,               /**< Pull-up resistor */
    HAL_GPIO_PULL_DOWN,             /**< Pull-down resistor */
    HAL_GPIO_PULL_MAX
} hal_gpio_pull_t;

/**
 * @brief GPIO pin output types
 */
typedef enum {
    HAL_GPIO_OUTPUT_PUSH_PULL = 0,  /**< Push-pull output */
    HAL_GPIO_OUTPUT_OPEN_DRAIN,     /**< Open-drain output */
    HAL_GPIO_OUTPUT_MAX
} hal_gpio_output_type_t;

/**
 * @brief GPIO pin speeds
 */
typedef enum {
    HAL_GPIO_SPEED_LOW = 0,         /**< Low speed */
    HAL_GPIO_SPEED_MEDIUM,          /**< Medium speed */
    HAL_GPIO_SPEED_HIGH,            /**< High speed */
    HAL_GPIO_SPEED_VERY_HIGH,       /**< Very high speed */
    HAL_GPIO_SPEED_MAX
} hal_gpio_speed_t;

/**
 * @brief GPIO interrupt trigger types
 */
typedef enum {
    HAL_GPIO_TRIGGER_NONE = 0,      /**< No interrupt */
    HAL_GPIO_TRIGGER_RISING,        /**< Rising edge trigger */
    HAL_GPIO_TRIGGER_FALLING,       /**< Falling edge trigger */
    HAL_GPIO_TRIGGER_BOTH,          /**< Both edges trigger */
    HAL_GPIO_TRIGGER_LOW,           /**< Low level trigger */
    HAL_GPIO_TRIGGER_HIGH,          /**< High level trigger */
    HAL_GPIO_TRIGGER_MAX
} hal_gpio_trigger_t;

/**
 * @brief GPIO pin states
 */
typedef enum {
    HAL_GPIO_STATE_LOW = 0,         /**< Low state (0V) */
    HAL_GPIO_STATE_HIGH = 1,        /**< High state (VDD) */
    HAL_GPIO_STATE_UNKNOWN = 2      /**< Unknown/invalid state */
} hal_gpio_state_t;

/**
 * @brief GPIO alternate functions (STM32WB55 specific)
 */
typedef enum {
    HAL_GPIO_AF_SYSTEM = 0,         /**< System function */
    HAL_GPIO_AF_TIM1,               /**< Timer 1 */
    HAL_GPIO_AF_TIM2,               /**< Timer 2 */
    HAL_GPIO_AF_TIM16,              /**< Timer 16 */
    HAL_GPIO_AF_TIM17,              /**< Timer 17 */
    HAL_GPIO_AF_I2C1,               /**< I2C 1 */
    HAL_GPIO_AF_I2C3,               /**< I2C 3 */
    HAL_GPIO_AF_SPI1,               /**< SPI 1 */
    HAL_GPIO_AF_SPI2,               /**< SPI 2 */
    HAL_GPIO_AF_USART1,             /**< USART 1 */
    HAL_GPIO_AF_LPUART1,            /**< Low Power UART 1 */
    HAL_GPIO_AF_RF,                 /**< Radio frequency */
    HAL_GPIO_AF_USB,                /**< USB */
    HAL_GPIO_AF_LCD,                /**< LCD */
    HAL_GPIO_AF_QUADSPI,            /**< Quad SPI */
    HAL_GPIO_AF_EVENTOUT,           /**< Event output */
    HAL_GPIO_AF_MAX
} hal_gpio_alternate_function_t;

/**
 * @brief GPIO pin configuration structure
 */
typedef struct {
    uint32_t pin;                           /**< Pin number (0-63) */
    hal_gpio_mode_t mode;                   /**< Pin mode */
    hal_gpio_pull_t pull;                   /**< Pull resistor configuration */
    hal_gpio_output_type_t output_type;     /**< Output type (for output mode) */
    hal_gpio_speed_t speed;                 /**< Pin speed */
    hal_gpio_alternate_function_t alt_func; /**< Alternate function (for AF mode) */
    hal_gpio_trigger_t trigger;             /**< Interrupt trigger type */
} hal_gpio_config_t;

/**
 * @brief GPIO interrupt callback function type
 * @param pin Pin number that triggered the interrupt
 * @param user_data User-provided data pointer
 */
typedef void (*hal_gpio_interrupt_callback_t)(uint32_t pin, void *user_data);

/**
 * @brief GPIO interrupt context structure
 */
typedef struct {
    uint32_t pin;                           /**< Pin number */
    hal_gpio_interrupt_callback_t callback; /**< Callback function */
    void *user_data;                        /**< User data pointer */
    bool enabled;                           /**< Interrupt enabled flag */
} hal_gpio_interrupt_context_t;

/**
 * @brief Initialize GPIO HAL
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_init(void);

/**
 * @brief Deinitialize GPIO HAL
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_deinit(void);

/**
 * @brief Configure a GPIO pin
 * @param config Pin configuration
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_configure_pin(const hal_gpio_config_t *config);

/**
 * @brief Set GPIO pin state
 * @param pin Pin number (0-63)
 * @param state Pin state (HIGH/LOW)
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_set_pin(uint32_t pin, hal_gpio_state_t state);

/**
 * @brief Get GPIO pin state
 * @param pin Pin number (0-63)
 * @param state Pointer to store pin state
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_get_pin(uint32_t pin, hal_gpio_state_t *state);

/**
 * @brief Toggle GPIO pin state
 * @param pin Pin number (0-63)
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_toggle_pin(uint32_t pin);

/**
 * @brief Set multiple GPIO pins at once
 * @param pin_mask Bitmask of pins to set (bit position = pin number)
 * @param state_mask Bitmask of desired states (1=HIGH, 0=LOW)
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_set_pins(uint64_t pin_mask, uint64_t state_mask);

/**
 * @brief Get multiple GPIO pin states at once
 * @param pin_mask Bitmask of pins to read (bit position = pin number)
 * @param state_mask Pointer to store pin states (1=HIGH, 0=LOW)
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_get_pins(uint64_t pin_mask, uint64_t *state_mask);

/**
 * @brief Enable GPIO pin interrupt
 * @param pin Pin number (0-63)
 * @param trigger Interrupt trigger type
 * @param callback Callback function to call on interrupt
 * @param user_data User data to pass to callback
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_enable_interrupt(uint32_t pin, hal_gpio_trigger_t trigger,
                                       hal_gpio_interrupt_callback_t callback, void *user_data);

/**
 * @brief Disable GPIO pin interrupt
 * @param pin Pin number (0-63)
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_disable_interrupt(uint32_t pin);

/**
 * @brief Set GPIO pin alternate function
 * @param pin Pin number (0-63)
 * @param alt_func Alternate function to assign
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_set_alternate_function(uint32_t pin, hal_gpio_alternate_function_t alt_func);

/**
 * @brief Get GPIO pin configuration
 * @param pin Pin number (0-63)
 * @param config Pointer to store pin configuration
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_get_pin_config(uint32_t pin, hal_gpio_config_t *config);

/**
 * @brief Check if GPIO pin is available for use
 * @param pin Pin number (0-63)
 * @return true if available, false if in use or invalid
 */
bool hal_gpio_is_pin_available(uint32_t pin);

/**
 * @brief Reserve GPIO pin for exclusive use
 * @param pin Pin number (0-63)
 * @param owner_name Name of the owner (for debugging)
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_reserve_pin(uint32_t pin, const char *owner_name);

/**
 * @brief Release GPIO pin reservation
 * @param pin Pin number (0-63)
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_gpio_release_pin(uint32_t pin);

/**
 * @brief Get GPIO pin owner name
 * @param pin Pin number (0-63)
 * @return Owner name string or NULL if not reserved
 */
const char *hal_gpio_get_pin_owner(uint32_t pin);

#endif /* HAL_GPIO_H */