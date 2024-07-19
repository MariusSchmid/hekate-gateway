#ifndef F1CAD709_63B5_43D2_9191_D94B8FDF2D24
#define F1CAD709_63B5_43D2_9191_D94B8FDF2D24

#include <stdint.h>
#include <stdbool.h>

typedef struct SPI_handle_s *SPI_handle_t;

typedef bool (*spi_set_chip_select_fptr_t)(SPI_handle_t spi_handle, bool enable);
typedef bool (*spi_write_read_fptr_t)(SPI_handle_t spi_handle, uint8_t *tx_buffer, uint8_t *rx_buffer, uint32_t buffersize);
typedef bool (*spi_write_read_burst_fptr_t)(SPI_handle_t spi_handle, uint8_t *command, uint32_t command_size, uint8_t *rx_buffer, uint32_t rx_buffersize);
typedef bool (*spi_write_burst_fptr_t)(SPI_handle_t spi_handle, uint8_t *command, uint32_t command_size, const uint8_t *tx_buffer, uint32_t rx_buffersize);
typedef bool (*spi_enable_fptr_t)(SPI_handle_t spi_handle);
typedef bool (*spi_disable_fptr_t)(SPI_handle_t spi_handle);
typedef bool (*spi_get_handle_t)(SPI_handle_t *spi_handle);

typedef struct concentrator_spi_if
{
    spi_write_read_fptr_t spi_write_read;
    spi_enable_fptr_t spi_enable;
    spi_disable_fptr_t spi_disable;
    spi_set_chip_select_fptr_t spi_set_chip_select;
    spi_get_handle_t spi_get_handle;
    spi_write_read_burst_fptr_t spi_write_read_burst;
    spi_write_burst_fptr_t spi_write_burst;
} concentrator_spi_if_t;

typedef struct lora_rx_packet_s
{
    uint32_t freq_hz; /*!> central frequency of the IF chain */
    int32_t freq_offset;
    uint8_t if_chain;  /*!> by which IF chain was packet received */
    uint8_t status;    /*!> status of the received packet */
    uint32_t count_us; /*!> internal concentrator counter for timestamping, 1 microsecond resolution */
    uint8_t rf_chain;  /*!> through which RF chain the packet was received */
    uint8_t modem_id;
    uint8_t modulation;   /*!> modulation used by the packet */
    uint8_t bandwidth;    /*!> modulation bandwidth (LoRa only) */
    uint32_t datarate;    /*!> RX datarate of the packet (SF for LoRa) */
    uint8_t coderate;     /*!> error-correcting code of the packet (LoRa only) */
    float rssic;          /*!> average RSSI of the channel in dB */
    float rssis;          /*!> average RSSI of the signal in dB */
    float snr;            /*!> average packet SNR, in dB (LoRa only) */
    float snr_min;        /*!> minimum packet SNR, in dB (LoRa only) */
    float snr_max;        /*!> maximum packet SNR, in dB (LoRa only) */
    uint16_t crc;         /*!> CRC that was received in the payload */
    uint16_t size;        /*!> payload size in bytes */
    uint8_t payload[256]; /*!> buffer containing the payload */
    bool ftime_received;  /*!> a fine timestamp has been received */
    uint32_t ftime;       /*!> packet fine timestamp (nanoseconds since last PPS) */
} lora_rx_packet_t;

typedef struct gateway_config_s
{
    uint64_t mac_address;
} gateway_config_t;

typedef struct gateway_stats_s
{

} gateway_stats_t;

#define ENSURE(val) \
    if (!val)       \
        return;

#define ENSURE_RET(val, ret) \
    if (!val)                \
        return ret;


#endif /* F1CAD709_63B5_43D2_9191_D94B8FDF2D24 */
