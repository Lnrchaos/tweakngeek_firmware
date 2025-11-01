/**
 * @file hal_display.h
 * @brief Display and Input Hardware Abstraction Layer Interface
 * 
 * This file defines the display and input HAL interface for the FlipperZero
 * screen, input handling for buttons and navigation, and basic graphics
 * primitives for the TweaknGeek firmware.
 */

#ifndef HAL_DISPLAY_H
#define HAL_DISPLAY_H

#include "hal.h"

/* Display configuration from tweakngeek_config.h */
#define DISPLAY_WIDTH       HAL_DISPLAY_WIDTH
#define DISPLAY_HEIGHT      HAL_DISPLAY_HEIGHT
#define DISPLAY_BUFFER_SIZE ((DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8)

/**
 * @brief Display pixel formats
 */
typedef enum {
    HAL_DISPLAY_FORMAT_MONO = 0,    /**< Monochrome 1-bit per pixel */
    HAL_DISPLAY_FORMAT_GRAY2,       /**< 2-bit grayscale */
    HAL_DISPLAY_FORMAT_GRAY4,       /**< 4-bit grayscale */
    HAL_DISPLAY_FORMAT_RGB565,      /**< 16-bit RGB565 */
    HAL_DISPLAY_FORMAT_MAX
} hal_display_format_t;

/**
 * @brief Display rotation modes
 */
typedef enum {
    HAL_DISPLAY_ROTATION_0 = 0,     /**< No rotation */
    HAL_DISPLAY_ROTATION_90,        /**< 90 degrees clockwise */
    HAL_DISPLAY_ROTATION_180,       /**< 180 degrees */
    HAL_DISPLAY_ROTATION_270,       /**< 270 degrees clockwise */
    HAL_DISPLAY_ROTATION_MAX
} hal_display_rotation_t;

/**
 * @brief Display backlight levels
 */
typedef enum {
    HAL_DISPLAY_BACKLIGHT_OFF = 0,  /**< Backlight off */
    HAL_DISPLAY_BACKLIGHT_LOW,      /**< Low brightness */
    HAL_DISPLAY_BACKLIGHT_MEDIUM,   /**< Medium brightness */
    HAL_DISPLAY_BACKLIGHT_HIGH,     /**< High brightness */
    HAL_DISPLAY_BACKLIGHT_MAX
} hal_display_backlight_t;

/**
 * @brief Input button types
 */
typedef enum {
    HAL_INPUT_BUTTON_UP = 0,        /**< Up button */
    HAL_INPUT_BUTTON_DOWN,          /**< Down button */
    HAL_INPUT_BUTTON_LEFT,          /**< Left button */
    HAL_INPUT_BUTTON_RIGHT,         /**< Right button */
    HAL_INPUT_BUTTON_OK,            /**< OK/Select button */
    HAL_INPUT_BUTTON_BACK,          /**< Back button */
    HAL_INPUT_BUTTON_MAX
} hal_input_button_t;

/**
 * @brief Input button states
 */
typedef enum {
    HAL_INPUT_STATE_RELEASED = 0,   /**< Button released */
    HAL_INPUT_STATE_PRESSED,        /**< Button pressed */
    HAL_INPUT_STATE_HELD,           /**< Button held down */
    HAL_INPUT_STATE_MAX
} hal_input_state_t;

/**
 * @brief Input event types
 */
typedef enum {
    HAL_INPUT_EVENT_PRESS = 0,      /**< Button press event */
    HAL_INPUT_EVENT_RELEASE,        /**< Button release event */
    HAL_INPUT_EVENT_HOLD,           /**< Button hold event */
    HAL_INPUT_EVENT_REPEAT,         /**< Button repeat event */
    HAL_INPUT_EVENT_MAX
} hal_input_event_t;

/**
 * @brief Graphics drawing modes
 */
typedef enum {
    HAL_GRAPHICS_MODE_SET = 0,      /**< Set pixels (OR operation) */
    HAL_GRAPHICS_MODE_CLEAR,        /**< Clear pixels (AND NOT operation) */
    HAL_GRAPHICS_MODE_INVERT,       /**< Invert pixels (XOR operation) */
    HAL_GRAPHICS_MODE_MAX
} hal_graphics_mode_t;

/**
 * @brief Font sizes
 */
typedef enum {
    HAL_FONT_SIZE_SMALL = 0,        /**< Small font (6x8) */
    HAL_FONT_SIZE_MEDIUM,           /**< Medium font (8x12) */
    HAL_FONT_SIZE_LARGE,            /**< Large font (12x16) */
    HAL_FONT_SIZE_MAX
} hal_font_size_t;

/**
 * @brief Display configuration structure
 */
typedef struct {
    uint16_t width;                     /**< Display width in pixels */
    uint16_t height;                    /**< Display height in pixels */
    hal_display_format_t format;        /**< Pixel format */
    hal_display_rotation_t rotation;    /**< Display rotation */
    hal_display_backlight_t backlight;  /**< Backlight level */
    uint8_t contrast;                   /**< Display contrast (0-255) */
    bool invert;                        /**< Invert display colors */
} hal_display_config_t;

/**
 * @brief Point structure
 */
typedef struct {
    int16_t x;                          /**< X coordinate */
    int16_t y;                          /**< Y coordinate */
} hal_point_t;

/**
 * @brief Rectangle structure
 */
typedef struct {
    int16_t x;                          /**< X coordinate */
    int16_t y;                          /**< Y coordinate */
    uint16_t width;                     /**< Width */
    uint16_t height;                    /**< Height */
} hal_rect_t;

/**
 * @brief Input event structure
 */
typedef struct {
    hal_input_button_t button;          /**< Button that generated the event */
    hal_input_event_t event;            /**< Event type */
    hal_input_state_t state;            /**< Current button state */
    uint32_t timestamp;                 /**< Event timestamp */
    uint32_t duration;                  /**< Duration for hold events */
} hal_input_event_data_t;

/**
 * @brief Input event callback function type
 * @param event Input event data
 * @param user_data User-provided data pointer
 */
typedef void (*hal_input_event_callback_t)(const hal_input_event_data_t *event, void *user_data);

/**
 * @brief Initialize Display HAL
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_display_init(void);

/**
 * @brief Deinitialize Display HAL
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_display_deinit(void);

/**
 * @brief Configure display parameters
 * @param config Display configuration
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_display_configure(const hal_display_config_t *config);

/**
 * @brief Get display configuration
 * @param config Pointer to store display configuration
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_display_get_config(hal_display_config_t *config);

/**
 * @brief Clear display buffer
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_display_clear(void);

/**
 * @brief Update display from buffer
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_display_update(void);

/**
 * @brief Set display backlight level
 * @param level Backlight level
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_display_set_backlight(hal_display_backlight_t level);

/**
 * @brief Set display contrast
 * @param contrast Contrast level (0-255)
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_display_set_contrast(uint8_t contrast);

/**
 * @brief Set display inversion
 * @param invert true to invert colors, false for normal
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_display_set_invert(bool invert);

/**
 * @brief Get display buffer pointer
 * @param buffer Pointer to store buffer address
 * @param size Pointer to store buffer size
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_display_get_buffer(uint8_t **buffer, uint32_t *size);

/* Graphics Primitives */

/**
 * @brief Set pixel in display buffer
 * @param x X coordinate
 * @param y Y coordinate
 * @param mode Drawing mode
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_graphics_set_pixel(int16_t x, int16_t y, hal_graphics_mode_t mode);

/**
 * @brief Draw line in display buffer
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 * @param mode Drawing mode
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_graphics_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, hal_graphics_mode_t mode);

/**
 * @brief Draw rectangle in display buffer
 * @param rect Rectangle coordinates and size
 * @param mode Drawing mode
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_graphics_draw_rect(const hal_rect_t *rect, hal_graphics_mode_t mode);

/**
 * @brief Fill rectangle in display buffer
 * @param rect Rectangle coordinates and size
 * @param mode Drawing mode
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_graphics_fill_rect(const hal_rect_t *rect, hal_graphics_mode_t mode);

/**
 * @brief Draw circle in display buffer
 * @param center Circle center point
 * @param radius Circle radius
 * @param mode Drawing mode
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_graphics_draw_circle(const hal_point_t *center, uint16_t radius, hal_graphics_mode_t mode);

/**
 * @brief Fill circle in display buffer
 * @param center Circle center point
 * @param radius Circle radius
 * @param mode Drawing mode
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_graphics_fill_circle(const hal_point_t *center, uint16_t radius, hal_graphics_mode_t mode);

/**
 * @brief Draw text in display buffer
 * @param text Text string to draw
 * @param position Text position
 * @param font_size Font size
 * @param mode Drawing mode
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_graphics_draw_text(const char *text, const hal_point_t *position, 
                                    hal_font_size_t font_size, hal_graphics_mode_t mode);

/**
 * @brief Draw bitmap in display buffer
 * @param bitmap Bitmap data
 * @param position Bitmap position
 * @param width Bitmap width
 * @param height Bitmap height
 * @param mode Drawing mode
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_graphics_draw_bitmap(const uint8_t *bitmap, const hal_point_t *position,
                                      uint16_t width, uint16_t height, hal_graphics_mode_t mode);

/* Input Functions */

/**
 * @brief Initialize Input HAL
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_input_init(void);

/**
 * @brief Deinitialize Input HAL
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_input_deinit(void);

/**
 * @brief Get current button state
 * @param button Button to check
 * @param state Pointer to store button state
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_input_get_button_state(hal_input_button_t button, hal_input_state_t *state);

/**
 * @brief Get all button states at once
 * @param states Array to store button states (size HAL_INPUT_BUTTON_MAX)
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_input_get_all_states(hal_input_state_t *states);

/**
 * @brief Register input event callback
 * @param callback Callback function
 * @param user_data User data to pass to callback
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_input_register_callback(hal_input_event_callback_t callback, void *user_data);

/**
 * @brief Unregister input event callback
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_input_unregister_callback(void);

/**
 * @brief Process input events (should be called periodically)
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_input_process_events(void);

/* Utility Functions */

/**
 * @brief Get font character width
 * @param font_size Font size
 * @return Character width in pixels
 */
uint8_t hal_graphics_get_char_width(hal_font_size_t font_size);

/**
 * @brief Get font character height
 * @param font_size Font size
 * @return Character height in pixels
 */
uint8_t hal_graphics_get_char_height(hal_font_size_t font_size);

/**
 * @brief Calculate text width
 * @param text Text string
 * @param font_size Font size
 * @return Text width in pixels
 */
uint16_t hal_graphics_get_text_width(const char *text, hal_font_size_t font_size);

/**
 * @brief Get button name string
 * @param button Button type
 * @return String representation of button
 */
const char *hal_input_button_to_string(hal_input_button_t button);

/**
 * @brief Get input event name string
 * @param event Event type
 * @return String representation of event
 */
const char *hal_input_event_to_string(hal_input_event_t event);

#endif /* HAL_DISPLAY_H */