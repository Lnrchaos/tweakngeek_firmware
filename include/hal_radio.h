/**
 * @file hal_radio.h
 * @brief Radio Hardware Abstraction Layer Interface
 * 
 * This file defines the unified radio HAL interface for multiple hardware types
 * including CC1101 and Bluetooth, with frequency and power management functions,
 * and protocol-agnostic transmission and reception capabilities.
 */

#ifndef HAL_RADIO_H
#define HAL_RADIO_H

#include "hal.h"

/**
 * @brief Radio hardware types
 */
typedef enum {
    HAL_RADIO_TYPE_CC1101 = 0,      /**< CC1101 Sub-GHz radio */
    HAL_RADIO_TYPE_BLUETOOTH,       /**< Bluetooth Low Energy */
    HAL_RADIO_TYPE_MAX
} hal_radio_type_t;

/**
 * @brief Radio modulation types
 */
typedef enum {
    HAL_RADIO_MODULATION_ASK = 0,   /**< Amplitude Shift Keying */
    HAL_RADIO_MODULATION_FSK,       /**< Frequency Shift Keying */
    HAL_RADIO_MODULATION_GFSK,      /**< Gaussian FSK */
    HAL_RADIO_MODULATION_MSK,       /**< Minimum Shift Keying */
    HAL_RADIO_MODULATION_OOK,       /**< On-Off Keying */
    HAL_RADIO_MODULATION_MAX
} hal_radio_modulation_t;

/**
 * @brief Radio power levels
 */
typedef enum {
    HAL_RADIO_POWER_MIN = 0,        /**< Minimum power (-30 dBm) */
    HAL_RADIO_POWER_LOW,            /**< Low power (-20 dBm) */
    HAL_RADIO_POWER_MEDIUM,         /**< Medium power (-10 dBm) */
    HAL_RADIO_POWER_HIGH,           /**< High power (0 dBm) */
    HAL_RADIO_POWER_MAX,            /**< Maximum power (+10 dBm) */
    HAL_RADIO_POWER_LEVELS
} hal_radio_power_t;

/**
 * @brief Radio states
 */
typedef enum {
    HAL_RADIO_STATE_IDLE = 0,       /**< Idle state */
    HAL_RADIO_STATE_RX,             /**< Receive mode */
    HAL_RADIO_STATE_TX,             /**< Transmit mode */
    HAL_RADIO_STATE_SLEEP,          /**< Sleep mode */
    HAL_RADIO_STATE_CALIBRATE,      /**< Calibration mode */
    HAL_RADIO_STATE_ERROR,          /**< Error state */
    HAL_RADIO_STATE_MAX
} hal_radio_state_t;

/**
 * @brief Radio packet formats
 */
typedef enum {
    HAL_RADIO_PACKET_RAW = 0,       /**< Raw data packet */
    HAL_RADIO_PACKET_FIXED_LENGTH,  /**< Fixed length packet */
    HAL_RADIO_PACKET_VARIABLE_LENGTH, /**< Variable length packet */
    HAL_RADIO_PACKET_INFINITE,      /**< Infinite length packet */
    HAL_RADIO_PACKET_MAX
} hal_radio_packet_format_t;

/**
 * @brief Radio configuration structure
 */
typedef struct {
    hal_radio_type_t type;              /**< Radio hardware type */
    uint32_t frequency_hz;              /**< Operating frequency in Hz */
    uint32_t data_rate_bps;             /**< Data rate in bits per second */
    hal_radio_modulation_t modulation;  /**< Modulation type */
    hal_radio_power_t power_level;      /**< Transmit power level */
    uint32_t deviation_hz;              /**< Frequency deviation (FSK/GFSK) */
    uint32_t bandwidth_hz;              /**< Channel bandwidth in Hz */
    hal_radio_packet_format_t packet_format; /**< Packet format */
    uint8_t sync_word[4];               /**< Synchronization word */
    uint8_t sync_word_length;           /**< Sync word length (1-4 bytes) */
    bool crc_enabled;                   /**< CRC enable flag */
    bool whitening_enabled;             /**< Data whitening enable flag */
} hal_radio_config_t;

/**
 * @brief Radio packet structure
 */
typedef struct {
    uint8_t *data;                      /**< Packet data buffer */
    uint16_t length;                    /**< Packet length in bytes */
    int8_t rssi;                        /**< Received signal strength (dBm) */
    uint8_t lqi;                        /**< Link quality indicator */
    uint32_t timestamp;                 /**< Packet timestamp */
    bool crc_ok;                        /**< CRC validation result */
} hal_radio_packet_t;

/**
 * @brief Radio statistics structure
 */
typedef struct {
    uint32_t packets_transmitted;       /**< Total packets transmitted */
    uint32_t packets_received;          /**< Total packets received */
    uint32_t packets_dropped;           /**< Packets dropped due to errors */
    uint32_t crc_errors;                /**< CRC error count */
    uint32_t sync_errors;               /**< Sync word error count */
    int8_t last_rssi;                   /**< Last measured RSSI */
    uint8_t last_lqi;                   /**< Last measured LQI */
} hal_radio_stats_t;

/**
 * @brief Radio event types
 */
typedef enum {
    HAL_RADIO_EVENT_TX_COMPLETE = 0,    /**< Transmission completed */
    HAL_RADIO_EVENT_RX_COMPLETE,        /**< Reception completed */
    HAL_RADIO_EVENT_RX_TIMEOUT,         /**< Reception timeout */
    HAL_RADIO_EVENT_CRC_ERROR,          /**< CRC error detected */
    HAL_RADIO_EVENT_SYNC_DETECTED,      /**< Sync word detected */
    HAL_RADIO_EVENT_FIFO_OVERFLOW,      /**< FIFO overflow */
    HAL_RADIO_EVENT_FIFO_UNDERFLOW,     /**< FIFO underflow */
    HAL_RADIO_EVENT_MAX
} hal_radio_event_t;

/**
 * @brief Radio event callback function type
 * @param radio_id Radio device ID
 * @param event Event type
 * @param data Event-specific data
 * @param user_data User-provided data pointer
 */
typedef void (*hal_radio_event_callback_t)(uint32_t radio_id, hal_radio_event_t event, 
                                           void *data, void *user_data);

/**
 * @brief Initialize Radio HAL
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_init(void);

/**
 * @brief Deinitialize Radio HAL
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_deinit(void);

/**
 * @brief Open a radio device
 * @param type Radio hardware type
 * @param radio_id Pointer to store assigned radio ID
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_open(hal_radio_type_t type, uint32_t *radio_id);

/**
 * @brief Close a radio device
 * @param radio_id Radio device ID
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_close(uint32_t radio_id);

/**
 * @brief Configure radio parameters
 * @param radio_id Radio device ID
 * @param config Radio configuration
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_configure(uint32_t radio_id, const hal_radio_config_t *config);

/**
 * @brief Get radio configuration
 * @param radio_id Radio device ID
 * @param config Pointer to store radio configuration
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_get_config(uint32_t radio_id, hal_radio_config_t *config);

/**
 * @brief Set radio frequency
 * @param radio_id Radio device ID
 * @param frequency_hz Frequency in Hz
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_set_frequency(uint32_t radio_id, uint32_t frequency_hz);

/**
 * @brief Set radio power level
 * @param radio_id Radio device ID
 * @param power_level Power level
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_set_power(uint32_t radio_id, hal_radio_power_t power_level);

/**
 * @brief Set radio modulation
 * @param radio_id Radio device ID
 * @param modulation Modulation type
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_set_modulation(uint32_t radio_id, hal_radio_modulation_t modulation);

/**
 * @brief Transmit data packet
 * @param radio_id Radio device ID
 * @param packet Packet to transmit
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_transmit(uint32_t radio_id, const hal_radio_packet_t *packet);

/**
 * @brief Receive data packet
 * @param radio_id Radio device ID
 * @param packet Buffer to store received packet
 * @param timeout_ms Timeout in milliseconds (0 = no timeout)
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_receive(uint32_t radio_id, hal_radio_packet_t *packet, uint32_t timeout_ms);

/**
 * @brief Start continuous transmission
 * @param radio_id Radio device ID
 * @param data Data buffer to transmit continuously
 * @param length Data length in bytes
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_start_tx_continuous(uint32_t radio_id, const uint8_t *data, uint16_t length);

/**
 * @brief Start continuous reception
 * @param radio_id Radio device ID
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_start_rx_continuous(uint32_t radio_id);

/**
 * @brief Stop continuous operation
 * @param radio_id Radio device ID
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_stop_continuous(uint32_t radio_id);

/**
 * @brief Get radio state
 * @param radio_id Radio device ID
 * @param state Pointer to store radio state
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_get_state(uint32_t radio_id, hal_radio_state_t *state);

/**
 * @brief Set radio to idle state
 * @param radio_id Radio device ID
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_set_idle(uint32_t radio_id);

/**
 * @brief Set radio to sleep state
 * @param radio_id Radio device ID
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_set_sleep(uint32_t radio_id);

/**
 * @brief Calibrate radio
 * @param radio_id Radio device ID
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_calibrate(uint32_t radio_id);

/**
 * @brief Get radio statistics
 * @param radio_id Radio device ID
 * @param stats Pointer to store statistics
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_get_stats(uint32_t radio_id, hal_radio_stats_t *stats);

/**
 * @brief Reset radio statistics
 * @param radio_id Radio device ID
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_reset_stats(uint32_t radio_id);

/**
 * @brief Register event callback
 * @param radio_id Radio device ID
 * @param callback Callback function
 * @param user_data User data to pass to callback
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_register_callback(uint32_t radio_id, hal_radio_event_callback_t callback, 
                                         void *user_data);

/**
 * @brief Unregister event callback
 * @param radio_id Radio device ID
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_unregister_callback(uint32_t radio_id);

/**
 * @brief Perform raw register read (for advanced users)
 * @param radio_id Radio device ID
 * @param reg_addr Register address
 * @param value Pointer to store register value
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_read_register(uint32_t radio_id, uint8_t reg_addr, uint8_t *value);

/**
 * @brief Perform raw register write (for advanced users)
 * @param radio_id Radio device ID
 * @param reg_addr Register address
 * @param value Register value to write
 * @return HAL_OK on success, error code otherwise
 */
hal_result_t hal_radio_write_register(uint32_t radio_id, uint8_t reg_addr, uint8_t value);

/**
 * @brief Get radio type name string
 * @param type Radio type
 * @return String representation of radio type
 */
const char *hal_radio_type_to_string(hal_radio_type_t type);

/**
 * @brief Get radio state name string
 * @param state Radio state
 * @return String representation of radio state
 */
const char *hal_radio_state_to_string(hal_radio_state_t state);

#endif /* HAL_RADIO_H */