#ifndef B1CA4511_BE0C_4D43_95EC_5FC3EBED0100
#define B1CA4511_BE0C_4D43_95EC_5FC3EBED0100

#include "concentrator_types.h"

bool spi_init(concentrator_spi_if_t spi_if);
bool spi_get_handle(SPI_handle_t *spi_handle);
// bool spi_init(SPI_handle_t *spi_handle, SPI_settings_t setting);
// bool spi_deinit(SPI_handle_t *spi_handle);
bool spi_enable(SPI_handle_t spi_handle);
bool spi_disable(SPI_handle_t spi_handle);
bool spi_write_read(SPI_handle_t spi_handle, uint8_t *tx_buffer, uint8_t *rx_buffer, uint32_t buffersize);
// bool spi_set_chip_select(SPI_handle_t spi_handle, bool enable);
bool spi_write_read_burst(SPI_handle_t spi_handle, uint8_t *tx_buffer, uint32_t tx_buffer_size, uint8_t *rx_buffer, uint32_t rx_buffersize);
bool spi_write_burst(SPI_handle_t spi_handle, uint8_t *tx_buffer, uint32_t tx_buffer_size,const uint8_t *rx_buffer, uint32_t rx_buffersize);

#endif /* B1CA4511_BE0C_4D43_95EC_5FC3EBED0100 */
