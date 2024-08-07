#ifndef B1CA4511_BE0C_4D43_95EC_5FC3EBED0100
#define B1CA4511_BE0C_4D43_95EC_5FC3EBED0100

#include "concentrator_types.h"

bool concentrator_spi_init(concentrator_spi_if_t spi_if);
bool concentrator_spi_get_handle(SPI_handle_t *spi_handle);
// bool spi_init(SPI_handle_t *spi_handle, SPI_settings_t setting);
// bool spi_deinit(SPI_handle_t *spi_handle);
bool concentrator_spi_enable(SPI_handle_t spi_handle);
bool concentrator_spi_disable(SPI_handle_t spi_handle);
bool concentrator_spi_write_read(SPI_handle_t spi_handle, uint8_t *tx_buffer, uint8_t *rx_buffer, uint32_t buffersize);
// bool spi_set_chip_select(SPI_handle_t spi_handle, bool enable);
bool concentrator_spi_write_read_burst(SPI_handle_t spi_handle, uint8_t *command, uint32_t command_size, uint8_t *rx_buffer, uint32_t rx_buffersize);
bool concentrator_spi_write_burst(SPI_handle_t spi_handle, uint8_t *command, uint32_t command_size, const uint8_t *tx_data, uint32_t tx_data_size);

#endif /* B1CA4511_BE0C_4D43_95EC_5FC3EBED0100 */
