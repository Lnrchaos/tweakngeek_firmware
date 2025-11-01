/**
 * @file hal_gpio.c
 * @brief GPIO Hardware Abstraction Layer Implementation
 * 
 * This file implements GPIO pin configuration, control functions,
 * interrupt-driven GPIO handling, and dynamic pin function assignment
 * for the STM32WB55 microcontroller.
 */

#include "hal_gpio.h"
#include "hal_internal.h"
#include <string.h>
#include <stdlib.h>

/* STM32WB55 GPIO register definitions */
#define GPIOA_BASE      0x48000000UL
#define GPIOB_BASE      0x48000400UL
#define GPIOC_BASE      0x48000800UL
#define GPIOD_BASE      0x48000C00UL
#define GPIOE_BASE      0x48001000UL
#define GPIOH_BASE      0x48001C00UL

/* GPIO register offsets */
#define GPIO_MODER_OFFSET   0x00    /**< Mode register */
#define GPIO_OTYPER_OFFSET  0x04    /**< Output type register */
#define GPIO_OSPEEDR_OFFSET 0x08    /**< Output speed register */
#define GPIO_PUPDR_OFFSET   0x0C    /**< Pull-up/pull-down register */
#define GPIO_IDR_OFFSET     0x10    /**< Input data register */
#define GPIO_ODR_OFFSET     0x14    /**< Output data register */
#define GPIO_BSRR_OFFSET    0x18    /**< Bit set/reset register */
#define GPIO_LCKR_OFFSET    0x1C    /**< Configuration lock register */
#define GPIO_AFRL_OFFSET    0x20    /**< Alternate function low register */
#define GPIO_AFRH_OFFSET    0x24    /**< Alternate function high register */

/* Maximum number of GPIO pins */
#define MAX_GPIO_PINS       64
#define PINS_PER_PORT       16

/* GPIO port bases */
static const uint32_t gpio_port_bases[] = {
    GPIOA_BASE, GPIOB_BASE, GPIOC_BASE, GPIOD_BASE,
    GPIOE_BASE, 0, 0, GPIOH_BASE  /* Ports F and G not available on STM32WB55 */
};

/**
 * @brief GPIO pin state structure
 */
typedef struct {
    hal_gpio_config_t config;               /**< Pin configuration */
    bool reserved;                          /**< Pin reservation status */
    char owner_name[32];                    /**< Owner name for debugging */
    hal_gpio_interrupt_context_t interrupt; /**< Interrupt context */
} hal_gpio_pin_state_t;

/* GPIO HAL state */
static bool gpio_hal_initialized = false;
static hal_gpio_pin_state_t pin_states[MAX_GPIO_PINS];
static hal_device_t gpio_device;
static hal_driver_t gpio_driver;

/* Forward declarations */
static hal_result_t gpio_driver_init(hal_device_t *device);
static hal_result_t gpio_driver_deinit(hal_device_t *device);
static uint32_t gpio_get_port_base(uint32_t pin);
static uint32_t gpio_get_pin_mask(uint32_t pin);
static void gpio_interrupt_handler(uint32_t pin);

/* GPIO driver operations */
static const hal_driver_ops_t gpio_driver_ops = {
    .init = gpio_driver_init,
    .deinit = gpio_driver_deinit,
    .open = NULL,
    .close = NULL,
    .read = NULL,
    .write = NULL,
    .ioctl = NULL,
    .suspend = NULL,
    .resume = NULL
};

/**
 * @brief Initialize GPIO HAL
 */
hal_result_t hal_gpio_init(void)
{
    if (gpio_hal_initialized) {
        return HAL_OK;
    }

    /* Initialize pin states */
    memset(pin_states, 0, sizeof(pin_states));
    for (uint32_t i = 0; i < MAX_GPIO_PINS; i++) {
        pin_states[i].config.pin = i;
        pin_states[i].config.mode = HAL_GPIO_MODE_INPUT;
        pin_states[i].config.pull = HAL_GPIO_PULL_NONE;
        pin_states[i].reserved = false;
        pin_states[i].interrupt.enabled = false;
    }

    /* Initialize GPIO driver */
    gpio_driver.name = "gpio";
    gpio_driver.type = HAL_DEVICE_TYPE_GPIO;
    gpio_driver.version = 0x010000;  /* Version 1.0.0 */
    gpio_driver.ops = &gpio_driver_ops;
    gpio_driver.next = NULL;

    /* Register GPIO driver */
    hal_result_t result = hal_driver_register(&gpio_driver);
    if (result != HAL_OK) {
        return result;
    }

    /* Initialize GPIO device */
    gpio_device.device_id = 0;  /* Will be assigned by hal_device_register */
    gpio_device.name = "gpio0";
    gpio_device.type = HAL_DEVICE_TYPE_GPIO;
    gpio_device.state = HAL_DEVICE_STATE_UNINITIALIZED;
    gpio_device.config.base_address = GPIOA_BASE;
    gpio_device.config.size = 0x2000;  /* Covers all GPIO ports */
    gpio_device.config.irq_number = 0;
    gpio_device.config.clock_frequency = 0;
    gpio_device.config.flags = 0;
    gpio_device.driver = &gpio_driver;
    gpio_device.private_data = NULL;
    gpio_device.ref_count = 0;
    gpio_device.next = NULL;

    /* Register GPIO device */
    result = hal_device_register(&gpio_device);
    if (result != HAL_OK) {
        hal_driver_unregister(&gpio_driver);
        return result;
    }

    gpio_hal_initialized = true;
    return HAL_OK;
}

/**
 * @brief Deinitialize GPIO HAL
 */
hal_result_t hal_gpio_deinit(void)
{
    if (!gpio_hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    /* Disable all interrupts */
    for (uint32_t i = 0; i < MAX_GPIO_PINS; i++) {
        if (pin_states[i].interrupt.enabled) {
            hal_gpio_disable_interrupt(i);
        }
    }

    /* Unregister device and driver */
    hal_device_unregister(&gpio_device);
    hal_driver_unregister(&gpio_driver);

    gpio_hal_initialized = false;
    return HAL_OK;
}

/**
 * @brief Configure a GPIO pin
 */
hal_result_t hal_gpio_configure_pin(const hal_gpio_config_t *config)
{
    if (!gpio_hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (config == NULL || config->pin >= MAX_GPIO_PINS) {
        return HAL_ERROR_INVALID_PARAM;
    }

    uint32_t pin = config->pin;
    uint32_t port_base = gpio_get_port_base(pin);
    if (port_base == 0) {
        return HAL_ERROR_INVALID_PARAM;
    }

    uint32_t pin_pos = pin % PINS_PER_PORT;
    uint32_t pin_mask = 1UL << pin_pos;

    /* Configure pin mode */
    volatile uint32_t *moder = (volatile uint32_t *)(port_base + GPIO_MODER_OFFSET);
    *moder = (*moder & ~(3UL << (pin_pos * 2))) | ((uint32_t)config->mode << (pin_pos * 2));

    /* Configure output type (only for output mode) */
    if (config->mode == HAL_GPIO_MODE_OUTPUT) {
        volatile uint32_t *otyper = (volatile uint32_t *)(port_base + GPIO_OTYPER_OFFSET);
        if (config->output_type == HAL_GPIO_OUTPUT_OPEN_DRAIN) {
            *otyper |= pin_mask;
        } else {
            *otyper &= ~pin_mask;
        }
    }

    /* Configure pin speed */
    volatile uint32_t *ospeedr = (volatile uint32_t *)(port_base + GPIO_OSPEEDR_OFFSET);
    *ospeedr = (*ospeedr & ~(3UL << (pin_pos * 2))) | ((uint32_t)config->speed << (pin_pos * 2));

    /* Configure pull-up/pull-down */
    volatile uint32_t *pupdr = (volatile uint32_t *)(port_base + GPIO_PUPDR_OFFSET);
    *pupdr = (*pupdr & ~(3UL << (pin_pos * 2))) | ((uint32_t)config->pull << (pin_pos * 2));

    /* Configure alternate function (only for AF mode) */
    if (config->mode == HAL_GPIO_MODE_ALTERNATE) {
        volatile uint32_t *afr = (volatile uint32_t *)(port_base + 
            (pin_pos < 8 ? GPIO_AFRL_OFFSET : GPIO_AFRH_OFFSET));
        uint32_t afr_pos = (pin_pos % 8) * 4;
        *afr = (*afr & ~(0xFUL << afr_pos)) | ((uint32_t)config->alt_func << afr_pos);
    }

    /* Store configuration */
    pin_states[pin].config = *config;

    return HAL_OK;
}

/**
 * @brief Set GPIO pin state
 */
hal_result_t hal_gpio_set_pin(uint32_t pin, hal_gpio_state_t state)
{
    if (!gpio_hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (pin >= MAX_GPIO_PINS || state >= HAL_GPIO_STATE_UNKNOWN) {
        return HAL_ERROR_INVALID_PARAM;
    }

    uint32_t port_base = gpio_get_port_base(pin);
    if (port_base == 0) {
        return HAL_ERROR_INVALID_PARAM;
    }

    uint32_t pin_pos = pin % PINS_PER_PORT;
    volatile uint32_t *bsrr = (volatile uint32_t *)(port_base + GPIO_BSRR_OFFSET);

    if (state == HAL_GPIO_STATE_HIGH) {
        *bsrr = 1UL << pin_pos;  /* Set bit */
    } else {
        *bsrr = 1UL << (pin_pos + 16);  /* Reset bit */
    }

    return HAL_OK;
}

/**
 * @brief Get GPIO pin state
 */
hal_result_t hal_gpio_get_pin(uint32_t pin, hal_gpio_state_t *state)
{
    if (!gpio_hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (pin >= MAX_GPIO_PINS || state == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    uint32_t port_base = gpio_get_port_base(pin);
    if (port_base == 0) {
        return HAL_ERROR_INVALID_PARAM;
    }

    uint32_t pin_pos = pin % PINS_PER_PORT;
    volatile uint32_t *idr = (volatile uint32_t *)(port_base + GPIO_IDR_OFFSET);

    *state = ((*idr >> pin_pos) & 1) ? HAL_GPIO_STATE_HIGH : HAL_GPIO_STATE_LOW;

    return HAL_OK;
}

/**
 * @brief Toggle GPIO pin state
 */
hal_result_t hal_gpio_toggle_pin(uint32_t pin)
{
    hal_gpio_state_t current_state;
    hal_result_t result = hal_gpio_get_pin(pin, &current_state);
    if (result != HAL_OK) {
        return result;
    }

    hal_gpio_state_t new_state = (current_state == HAL_GPIO_STATE_HIGH) ? 
                                 HAL_GPIO_STATE_LOW : HAL_GPIO_STATE_HIGH;
    
    return hal_gpio_set_pin(pin, new_state);
}

/**
 * @brief Enable GPIO pin interrupt
 */
hal_result_t hal_gpio_enable_interrupt(uint32_t pin, hal_gpio_trigger_t trigger,
                                       hal_gpio_interrupt_callback_t callback, void *user_data)
{
    if (!gpio_hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (pin >= MAX_GPIO_PINS || callback == NULL || trigger >= HAL_GPIO_TRIGGER_MAX) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Store interrupt configuration */
    pin_states[pin].interrupt.pin = pin;
    pin_states[pin].interrupt.callback = callback;
    pin_states[pin].interrupt.user_data = user_data;
    pin_states[pin].interrupt.enabled = true;

    /* TODO: Configure EXTI (External Interrupt) hardware registers
     * This would involve:
     * 1. Configure EXTI line for the pin
     * 2. Set trigger type (rising/falling/both)
     * 3. Enable EXTI interrupt in NVIC
     * 4. Configure SYSCFG to route GPIO to EXTI
     */

    return HAL_OK;
}

/**
 * @brief Disable GPIO pin interrupt
 */
hal_result_t hal_gpio_disable_interrupt(uint32_t pin)
{
    if (!gpio_hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (pin >= MAX_GPIO_PINS) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Clear interrupt configuration */
    pin_states[pin].interrupt.enabled = false;
    pin_states[pin].interrupt.callback = NULL;
    pin_states[pin].interrupt.user_data = NULL;

    /* TODO: Disable EXTI hardware configuration */

    return HAL_OK;
}

/**
 * @brief Reserve GPIO pin for exclusive use
 */
hal_result_t hal_gpio_reserve_pin(uint32_t pin, const char *owner_name)
{
    if (!gpio_hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (pin >= MAX_GPIO_PINS || owner_name == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    if (pin_states[pin].reserved) {
        return HAL_ERROR_RESOURCE_BUSY;
    }

    pin_states[pin].reserved = true;
    strncpy(pin_states[pin].owner_name, owner_name, sizeof(pin_states[pin].owner_name) - 1);
    pin_states[pin].owner_name[sizeof(pin_states[pin].owner_name) - 1] = '\0';

    return HAL_OK;
}

/**
 * @brief Release GPIO pin reservation
 */
hal_result_t hal_gpio_release_pin(uint32_t pin)
{
    if (!gpio_hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (pin >= MAX_GPIO_PINS) {
        return HAL_ERROR_INVALID_PARAM;
    }

    pin_states[pin].reserved = false;
    pin_states[pin].owner_name[0] = '\0';

    return HAL_OK;
}

/**
 * @brief Check if GPIO pin is available for use
 */
bool hal_gpio_is_pin_available(uint32_t pin)
{
    if (!gpio_hal_initialized || pin >= MAX_GPIO_PINS) {
        return false;
    }

    return !pin_states[pin].reserved;
}

/**
 * @brief Get GPIO pin owner name
 */
const char *hal_gpio_get_pin_owner(uint32_t pin)
{
    if (!gpio_hal_initialized || pin >= MAX_GPIO_PINS || !pin_states[pin].reserved) {
        return NULL;
    }

    return pin_states[pin].owner_name;
}

/* Helper functions */

/**
 * @brief Get GPIO port base address for a pin
 */
static uint32_t gpio_get_port_base(uint32_t pin)
{
    uint32_t port = pin / PINS_PER_PORT;
    if (port >= sizeof(gpio_port_bases) / sizeof(gpio_port_bases[0])) {
        return 0;
    }
    return gpio_port_bases[port];
}

/**
 * @brief Get pin mask for a pin within its port
 */
static uint32_t gpio_get_pin_mask(uint32_t pin)
{
    return 1UL << (pin % PINS_PER_PORT);
}

/**
 * @brief GPIO interrupt handler (called by system interrupt handler)
 */
static void gpio_interrupt_handler(uint32_t pin)
{
    if (pin < MAX_GPIO_PINS && pin_states[pin].interrupt.enabled) {
        if (pin_states[pin].interrupt.callback) {
            pin_states[pin].interrupt.callback(pin, pin_states[pin].interrupt.user_data);
        }
    }
}

/* Driver implementation */

/**
 * @brief GPIO driver initialization
 */
static hal_result_t gpio_driver_init(hal_device_t *device)
{
    if (device == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* TODO: Enable GPIO clocks in RCC
     * This would involve setting the appropriate bits in RCC_AHB2ENR
     * to enable clocks for GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOH
     */

    return HAL_OK;
}

/**
 * @brief GPIO driver deinitialization
 */
static hal_result_t gpio_driver_deinit(hal_device_t *device)
{
    if (device == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* TODO: Disable GPIO clocks in RCC */

    return HAL_OK;
}
/**

 * @brief Set multiple GPIO pins at once
 */
hal_result_t hal_gpio_set_pins(uint64_t pin_mask, uint64_t state_mask)
{
    if (!gpio_hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    /* Process each port separately */
    for (uint32_t port = 0; port < 8; port++) {
        uint32_t port_base = gpio_port_bases[port];
        if (port_base == 0) {
            continue;  /* Skip unavailable ports */
        }

        uint32_t port_pin_mask = (pin_mask >> (port * PINS_PER_PORT)) & 0xFFFF;
        uint32_t port_state_mask = (state_mask >> (port * PINS_PER_PORT)) & 0xFFFF;

        if (port_pin_mask == 0) {
            continue;  /* No pins to set in this port */
        }

        volatile uint32_t *bsrr = (volatile uint32_t *)(port_base + GPIO_BSRR_OFFSET);
        
        /* Set bits for HIGH pins */
        uint32_t set_mask = port_pin_mask & port_state_mask;
        /* Reset bits for LOW pins */
        uint32_t reset_mask = port_pin_mask & (~port_state_mask);

        *bsrr = set_mask | (reset_mask << 16);
    }

    return HAL_OK;
}

/**
 * @brief Get multiple GPIO pin states at once
 */
hal_result_t hal_gpio_get_pins(uint64_t pin_mask, uint64_t *state_mask)
{
    if (!gpio_hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (state_mask == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    *state_mask = 0;

    /* Process each port separately */
    for (uint32_t port = 0; port < 8; port++) {
        uint32_t port_base = gpio_port_bases[port];
        if (port_base == 0) {
            continue;  /* Skip unavailable ports */
        }

        uint32_t port_pin_mask = (pin_mask >> (port * PINS_PER_PORT)) & 0xFFFF;
        if (port_pin_mask == 0) {
            continue;  /* No pins to read in this port */
        }

        volatile uint32_t *idr = (volatile uint32_t *)(port_base + GPIO_IDR_OFFSET);
        uint32_t port_state = *idr & port_pin_mask;

        *state_mask |= ((uint64_t)port_state) << (port * PINS_PER_PORT);
    }

    return HAL_OK;
}

/**
 * @brief Set GPIO pin alternate function
 */
hal_result_t hal_gpio_set_alternate_function(uint32_t pin, hal_gpio_alternate_function_t alt_func)
{
    if (!gpio_hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (pin >= MAX_GPIO_PINS || alt_func >= HAL_GPIO_AF_MAX) {
        return HAL_ERROR_INVALID_PARAM;
    }

    uint32_t port_base = gpio_get_port_base(pin);
    if (port_base == 0) {
        return HAL_ERROR_INVALID_PARAM;
    }

    uint32_t pin_pos = pin % PINS_PER_PORT;
    
    /* Configure alternate function register */
    volatile uint32_t *afr = (volatile uint32_t *)(port_base + 
        (pin_pos < 8 ? GPIO_AFRL_OFFSET : GPIO_AFRH_OFFSET));
    uint32_t afr_pos = (pin_pos % 8) * 4;
    *afr = (*afr & ~(0xFUL << afr_pos)) | ((uint32_t)alt_func << afr_pos);

    /* Update stored configuration */
    pin_states[pin].config.alt_func = alt_func;

    return HAL_OK;
}

/**
 * @brief Get GPIO pin configuration
 */
hal_result_t hal_gpio_get_pin_config(uint32_t pin, hal_gpio_config_t *config)
{
    if (!gpio_hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (pin >= MAX_GPIO_PINS || config == NULL) {
        return HAL_ERROR_INVALID_PARAM;
    }

    *config = pin_states[pin].config;
    return HAL_OK;
}