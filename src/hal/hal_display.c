/**
 * @file hal_display.c
 * @brief Display and Input Hardware Abstraction Layer Implementation
 * 
 * This file implements the display and input HAL interface for the FlipperZero
 * screen, input handling for buttons and navigation, and basic graphics
 * primitives for the TweaknGeek firmware.
 */

#include "hal_display.h"
#include "hal_internal.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Private constants */
#define DISPLAY_BUFFER_SIZE_BYTES   ((DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8)
#define INPUT_DEBOUNCE_TIME_MS      50
#define INPUT_HOLD_TIME_MS          500
#define INPUT_REPEAT_TIME_MS        100

/* Font data structures */
typedef struct {
    uint8_t width;
    uint8_t height;
    const uint8_t *data;
} font_info_t;

/* Simple 6x8 font data (basic ASCII characters) */
static const uint8_t font_6x8_data[] = {
    /* Space (32) */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* ! (33) */
    0x00, 0x00, 0x5F, 0x00, 0x00, 0x00,
    /* " (34) */
    0x00, 0x07, 0x00, 0x07, 0x00, 0x00,
    /* Add more characters as needed... */
};

/* Font definitions */
static const font_info_t fonts[] = {
    [HAL_FONT_SIZE_SMALL]  = { .width = 6,  .height = 8,  .data = font_6x8_data },
    [HAL_FONT_SIZE_MEDIUM] = { .width = 8,  .height = 12, .data = font_6x8_data }, /* Placeholder */
    [HAL_FONT_SIZE_LARGE]  = { .width = 12, .height = 16, .data = font_6x8_data }, /* Placeholder */
};

/* Private variables */
static uint8_t display_buffer[DISPLAY_BUFFER_SIZE_BYTES];
static hal_display_config_t current_config;
static bool display_initialized = false;
static bool input_initialized = false;

/* Input state tracking */
static hal_input_state_t button_states[HAL_INPUT_BUTTON_MAX];
static hal_input_state_t prev_button_states[HAL_INPUT_BUTTON_MAX];
static uint32_t button_press_times[HAL_INPUT_BUTTON_MAX];
static hal_input_event_callback_t input_callback = NULL;
static void *input_callback_user_data = NULL;

/* Private function prototypes */
static hal_result_t display_hardware_init(void);
static hal_result_t display_hardware_deinit(void);
static hal_result_t display_send_command(uint8_t cmd);
static hal_result_t display_send_data(const uint8_t *data, uint32_t size);
static void input_read_hardware_states(void);
static uint32_t get_system_time_ms(void);
static void bresenham_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, hal_graphics_mode_t mode);

/* Display HAL Implementation */

hal_result_t hal_display_init(void)
{
    if (display_initialized) {
        return HAL_OK;
    }

    /* Initialize display buffer */
    memset(display_buffer, 0, sizeof(display_buffer));

    /* Set default configuration */
    current_config.width = DISPLAY_WIDTH;
    current_config.height = DISPLAY_HEIGHT;
    current_config.format = HAL_DISPLAY_FORMAT_MONO;
    current_config.rotation = HAL_DISPLAY_ROTATION_0;
    current_config.backlight = HAL_DISPLAY_BACKLIGHT_MEDIUM;
    current_config.contrast = 128;
    current_config.invert = false;

    /* Initialize hardware */
    hal_result_t result = display_hardware_init();
    if (result != HAL_OK) {
        return result;
    }

    display_initialized = true;
    return HAL_OK;
}

hal_result_t hal_display_deinit(void)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    hal_result_t result = display_hardware_deinit();
    display_initialized = false;
    return result;
}

hal_result_t hal_display_configure(const hal_display_config_t *config)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (!config) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Validate configuration */
    if (config->format >= HAL_DISPLAY_FORMAT_MAX ||
        config->rotation >= HAL_DISPLAY_ROTATION_MAX ||
        config->backlight >= HAL_DISPLAY_BACKLIGHT_MAX) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Apply configuration */
    memcpy(&current_config, config, sizeof(hal_display_config_t));

    /* Update hardware settings */
    hal_display_set_backlight(config->backlight);
    hal_display_set_contrast(config->contrast);
    hal_display_set_invert(config->invert);

    return HAL_OK;
}

hal_result_t hal_display_get_config(hal_display_config_t *config)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (!config) {
        return HAL_ERROR_INVALID_PARAM;
    }

    memcpy(config, &current_config, sizeof(hal_display_config_t));
    return HAL_OK;
}

hal_result_t hal_display_clear(void)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    memset(display_buffer, 0, sizeof(display_buffer));
    return HAL_OK;
}

hal_result_t hal_display_update(void)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    return display_send_data(display_buffer, sizeof(display_buffer));
}

hal_result_t hal_display_set_backlight(hal_display_backlight_t level)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (level >= HAL_DISPLAY_BACKLIGHT_MAX) {
        return HAL_ERROR_INVALID_PARAM;
    }

    current_config.backlight = level;
    
    /* Hardware-specific backlight control would go here */
    /* For now, this is a stub implementation */
    
    return HAL_OK;
}

hal_result_t hal_display_set_contrast(uint8_t contrast)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    current_config.contrast = contrast;
    
    /* Hardware-specific contrast control would go here */
    /* For now, this is a stub implementation */
    
    return HAL_OK;
}

hal_result_t hal_display_set_invert(bool invert)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    current_config.invert = invert;
    
    /* Hardware-specific invert control would go here */
    /* For now, this is a stub implementation */
    
    return HAL_OK;
}

hal_result_t hal_display_get_buffer(uint8_t **buffer, uint32_t *size)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (!buffer || !size) {
        return HAL_ERROR_INVALID_PARAM;
    }

    *buffer = display_buffer;
    *size = sizeof(display_buffer);
    return HAL_OK;
}

/* Graphics Primitives Implementation */

hal_result_t hal_graphics_set_pixel(int16_t x, int16_t y, hal_graphics_mode_t mode)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (x < 0 || x >= DISPLAY_WIDTH || y < 0 || y >= DISPLAY_HEIGHT) {
        return HAL_ERROR_INVALID_PARAM;
    }

    if (mode >= HAL_GRAPHICS_MODE_MAX) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Calculate buffer position for monochrome display */
    uint32_t byte_index = (y / 8) * DISPLAY_WIDTH + x;
    uint8_t bit_mask = 1 << (y % 8);

    switch (mode) {
        case HAL_GRAPHICS_MODE_SET:
            display_buffer[byte_index] |= bit_mask;
            break;
        case HAL_GRAPHICS_MODE_CLEAR:
            display_buffer[byte_index] &= ~bit_mask;
            break;
        case HAL_GRAPHICS_MODE_INVERT:
            display_buffer[byte_index] ^= bit_mask;
            break;
        default:
            return HAL_ERROR_INVALID_PARAM;
    }

    return HAL_OK;
}

hal_result_t hal_graphics_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, hal_graphics_mode_t mode)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (mode >= HAL_GRAPHICS_MODE_MAX) {
        return HAL_ERROR_INVALID_PARAM;
    }

    bresenham_line(x0, y0, x1, y1, mode);
    return HAL_OK;
}

hal_result_t hal_graphics_draw_rect(const hal_rect_t *rect, hal_graphics_mode_t mode)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (!rect || mode >= HAL_GRAPHICS_MODE_MAX) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Draw rectangle outline */
    hal_graphics_draw_line(rect->x, rect->y, rect->x + rect->width - 1, rect->y, mode);
    hal_graphics_draw_line(rect->x + rect->width - 1, rect->y, rect->x + rect->width - 1, rect->y + rect->height - 1, mode);
    hal_graphics_draw_line(rect->x + rect->width - 1, rect->y + rect->height - 1, rect->x, rect->y + rect->height - 1, mode);
    hal_graphics_draw_line(rect->x, rect->y + rect->height - 1, rect->x, rect->y, mode);

    return HAL_OK;
}

hal_result_t hal_graphics_fill_rect(const hal_rect_t *rect, hal_graphics_mode_t mode)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (!rect || mode >= HAL_GRAPHICS_MODE_MAX) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Fill rectangle */
    for (int16_t y = rect->y; y < rect->y + rect->height; y++) {
        for (int16_t x = rect->x; x < rect->x + rect->width; x++) {
            hal_graphics_set_pixel(x, y, mode);
        }
    }

    return HAL_OK;
}

hal_result_t hal_graphics_draw_circle(const hal_point_t *center, uint16_t radius, hal_graphics_mode_t mode)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (!center || mode >= HAL_GRAPHICS_MODE_MAX) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Bresenham circle algorithm */
    int16_t x = 0;
    int16_t y = radius;
    int16_t d = 3 - 2 * radius;

    while (x <= y) {
        /* Draw 8 octants */
        hal_graphics_set_pixel(center->x + x, center->y + y, mode);
        hal_graphics_set_pixel(center->x - x, center->y + y, mode);
        hal_graphics_set_pixel(center->x + x, center->y - y, mode);
        hal_graphics_set_pixel(center->x - x, center->y - y, mode);
        hal_graphics_set_pixel(center->x + y, center->y + x, mode);
        hal_graphics_set_pixel(center->x - y, center->y + x, mode);
        hal_graphics_set_pixel(center->x + y, center->y - x, mode);
        hal_graphics_set_pixel(center->x - y, center->y - x, mode);

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }

    return HAL_OK;
}

hal_result_t hal_graphics_fill_circle(const hal_point_t *center, uint16_t radius, hal_graphics_mode_t mode)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (!center || mode >= HAL_GRAPHICS_MODE_MAX) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Fill circle by drawing horizontal lines */
    for (int16_t y = -radius; y <= radius; y++) {
        int16_t x_max = (int16_t)sqrt(radius * radius - y * y);
        for (int16_t x = -x_max; x <= x_max; x++) {
            hal_graphics_set_pixel(center->x + x, center->y + y, mode);
        }
    }

    return HAL_OK;
}

hal_result_t hal_graphics_draw_text(const char *text, const hal_point_t *position, 
                                    hal_font_size_t font_size, hal_graphics_mode_t mode)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (!text || !position || font_size >= HAL_FONT_SIZE_MAX || mode >= HAL_GRAPHICS_MODE_MAX) {
        return HAL_ERROR_INVALID_PARAM;
    }

    const font_info_t *font = &fonts[font_size];
    int16_t x = position->x;
    int16_t y = position->y;

    /* Simple text rendering - just draw rectangles for now */
    /* In a real implementation, this would use actual font data */
    while (*text) {
        if (*text >= 32 && *text <= 126) {
            hal_rect_t char_rect = {
                .x = x,
                .y = y,
                .width = font->width,
                .height = font->height
            };
            hal_graphics_draw_rect(&char_rect, mode);
        }
        
        x += font->width + 1;
        if (x >= DISPLAY_WIDTH) {
            x = position->x;
            y += font->height + 1;
        }
        
        text++;
    }

    return HAL_OK;
}

hal_result_t hal_graphics_draw_bitmap(const uint8_t *bitmap, const hal_point_t *position,
                                      uint16_t width, uint16_t height, hal_graphics_mode_t mode)
{
    if (!display_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (!bitmap || !position || mode >= HAL_GRAPHICS_MODE_MAX) {
        return HAL_ERROR_INVALID_PARAM;
    }

    /* Draw bitmap pixel by pixel */
    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            uint32_t byte_index = (y * width + x) / 8;
            uint8_t bit_index = (y * width + x) % 8;
            
            if (bitmap[byte_index] & (1 << bit_index)) {
                hal_graphics_set_pixel(position->x + x, position->y + y, mode);
            }
        }
    }

    return HAL_OK;
}

/* Input Functions Implementation */

hal_result_t hal_input_init(void)
{
    if (input_initialized) {
        return HAL_OK;
    }

    /* Initialize button states */
    for (int i = 0; i < HAL_INPUT_BUTTON_MAX; i++) {
        button_states[i] = HAL_INPUT_STATE_RELEASED;
        prev_button_states[i] = HAL_INPUT_STATE_RELEASED;
        button_press_times[i] = 0;
    }

    input_callback = NULL;
    input_callback_user_data = NULL;

    input_initialized = true;
    return HAL_OK;
}

hal_result_t hal_input_deinit(void)
{
    if (!input_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    input_callback = NULL;
    input_callback_user_data = NULL;
    input_initialized = false;
    return HAL_OK;
}

hal_result_t hal_input_get_button_state(hal_input_button_t button, hal_input_state_t *state)
{
    if (!input_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (button >= HAL_INPUT_BUTTON_MAX || !state) {
        return HAL_ERROR_INVALID_PARAM;
    }

    *state = button_states[button];
    return HAL_OK;
}

hal_result_t hal_input_get_all_states(hal_input_state_t *states)
{
    if (!input_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (!states) {
        return HAL_ERROR_INVALID_PARAM;
    }

    memcpy(states, button_states, sizeof(button_states));
    return HAL_OK;
}

hal_result_t hal_input_register_callback(hal_input_event_callback_t callback, void *user_data)
{
    if (!input_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    if (!callback) {
        return HAL_ERROR_INVALID_PARAM;
    }

    input_callback = callback;
    input_callback_user_data = user_data;
    return HAL_OK;
}

hal_result_t hal_input_unregister_callback(void)
{
    if (!input_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    input_callback = NULL;
    input_callback_user_data = NULL;
    return HAL_OK;
}

hal_result_t hal_input_process_events(void)
{
    if (!input_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }

    /* Read current hardware states */
    input_read_hardware_states();

    uint32_t current_time = get_system_time_ms();

    /* Process each button */
    for (int i = 0; i < HAL_INPUT_BUTTON_MAX; i++) {
        hal_input_state_t current_state = button_states[i];
        hal_input_state_t prev_state = prev_button_states[i];

        /* Detect state changes */
        if (current_state != prev_state) {
            if (current_state == HAL_INPUT_STATE_PRESSED && prev_state == HAL_INPUT_STATE_RELEASED) {
                /* Button press event */
                button_press_times[i] = current_time;
                
                if (input_callback) {
                    hal_input_event_data_t event = {
                        .button = (hal_input_button_t)i,
                        .event = HAL_INPUT_EVENT_PRESS,
                        .state = current_state,
                        .timestamp = current_time,
                        .duration = 0
                    };
                    input_callback(&event, input_callback_user_data);
                }
            } else if (current_state == HAL_INPUT_STATE_RELEASED && prev_state == HAL_INPUT_STATE_PRESSED) {
                /* Button release event */
                uint32_t duration = current_time - button_press_times[i];
                
                if (input_callback) {
                    hal_input_event_data_t event = {
                        .button = (hal_input_button_t)i,
                        .event = HAL_INPUT_EVENT_RELEASE,
                        .state = current_state,
                        .timestamp = current_time,
                        .duration = duration
                    };
                    input_callback(&event, input_callback_user_data);
                }
            }
        } else if (current_state == HAL_INPUT_STATE_PRESSED) {
            /* Check for hold events */
            uint32_t duration = current_time - button_press_times[i];
            
            if (duration >= INPUT_HOLD_TIME_MS && prev_state == HAL_INPUT_STATE_PRESSED) {
                button_states[i] = HAL_INPUT_STATE_HELD;
                
                if (input_callback) {
                    hal_input_event_data_t event = {
                        .button = (hal_input_button_t)i,
                        .event = HAL_INPUT_EVENT_HOLD,
                        .state = HAL_INPUT_STATE_HELD,
                        .timestamp = current_time,
                        .duration = duration
                    };
                    input_callback(&event, input_callback_user_data);
                }
            }
        }

        prev_button_states[i] = button_states[i];
    }

    return HAL_OK;
}

/* Utility Functions Implementation */

uint8_t hal_graphics_get_char_width(hal_font_size_t font_size)
{
    if (font_size >= HAL_FONT_SIZE_MAX) {
        return 0;
    }
    return fonts[font_size].width;
}

uint8_t hal_graphics_get_char_height(hal_font_size_t font_size)
{
    if (font_size >= HAL_FONT_SIZE_MAX) {
        return 0;
    }
    return fonts[font_size].height;
}

uint16_t hal_graphics_get_text_width(const char *text, hal_font_size_t font_size)
{
    if (!text || font_size >= HAL_FONT_SIZE_MAX) {
        return 0;
    }

    uint16_t width = 0;
    uint8_t char_width = fonts[font_size].width;
    
    while (*text) {
        if (width > 0) {
            width += 1; /* Character spacing */
        }
        width += char_width;
        text++;
    }

    return width;
}

const char *hal_input_button_to_string(hal_input_button_t button)
{
    static const char *button_names[] = {
        [HAL_INPUT_BUTTON_UP] = "UP",
        [HAL_INPUT_BUTTON_DOWN] = "DOWN",
        [HAL_INPUT_BUTTON_LEFT] = "LEFT",
        [HAL_INPUT_BUTTON_RIGHT] = "RIGHT",
        [HAL_INPUT_BUTTON_OK] = "OK",
        [HAL_INPUT_BUTTON_BACK] = "BACK"
    };

    if (button >= HAL_INPUT_BUTTON_MAX) {
        return "UNKNOWN";
    }

    return button_names[button];
}

const char *hal_input_event_to_string(hal_input_event_t event)
{
    static const char *event_names[] = {
        [HAL_INPUT_EVENT_PRESS] = "PRESS",
        [HAL_INPUT_EVENT_RELEASE] = "RELEASE",
        [HAL_INPUT_EVENT_HOLD] = "HOLD",
        [HAL_INPUT_EVENT_REPEAT] = "REPEAT"
    };

    if (event >= HAL_INPUT_EVENT_MAX) {
        return "UNKNOWN";
    }

    return event_names[event];
}

/* Private Functions Implementation */

static hal_result_t display_hardware_init(void)
{
    /* Hardware-specific initialization would go here */
    /* This is a stub implementation for the FlipperZero display */
    
    /* Initialize SPI interface for display */
    /* Configure GPIO pins for CS, DC, RST */
    /* Send initialization sequence to display controller */
    
    return HAL_OK;
}

static hal_result_t display_hardware_deinit(void)
{
    /* Hardware-specific deinitialization would go here */
    /* Turn off display, release GPIO pins, etc. */
    
    return HAL_OK;
}

static hal_result_t display_send_command(uint8_t cmd)
{
    /* Hardware-specific command sending would go here */
    /* Set DC pin low, send command via SPI */
    
    (void)cmd; /* Suppress unused parameter warning */
    return HAL_OK;
}

static hal_result_t display_send_data(const uint8_t *data, uint32_t size)
{
    /* Hardware-specific data sending would go here */
    /* Set DC pin high, send data via SPI */
    
    (void)data;
    (void)size;
    return HAL_OK;
}

static void input_read_hardware_states(void)
{
    /* Hardware-specific button reading would go here */
    /* Read GPIO pins for each button and update button_states array */
    
    /* For now, this is a stub implementation */
    /* In a real implementation, this would read the actual hardware pins */
    
    /* Example stub code - buttons are always released */
    for (int i = 0; i < HAL_INPUT_BUTTON_MAX; i++) {
        button_states[i] = HAL_INPUT_STATE_RELEASED;
    }
}

static uint32_t get_system_time_ms(void)
{
    /* Hardware-specific system time reading would go here */
    /* This should return the current system time in milliseconds */
    
    /* For now, return a stub value */
    static uint32_t stub_time = 0;
    return stub_time++;
}

static void bresenham_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, hal_graphics_mode_t mode)
{
    /* Bresenham's line algorithm implementation */
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    while (true) {
        hal_graphics_set_pixel(x0, y0, mode);

        if (x0 == x1 && y0 == y1) {
            break;
        }

        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}