#include "concentrator_spi.h"

static concentrator_spi_if_t this_spi_if;
bool concentrator_spi_init(concentrator_spi_if_t spi_if)
{
    this_spi_if = spi_if;
    return true;
}

bool concentrator_spi_write_read_burst(SPI_handle_t spi_handle, uint8_t *command, uint32_t command_size, uint8_t *rx_buffer, uint32_t rx_buffersize)
{
    return this_spi_if.spi_write_read_burst(spi_handle, command, command_size, rx_buffer, rx_buffersize);
}

bool concentrator_spi_write_burst(SPI_handle_t spi_handle, uint8_t *command, uint32_t command_size, const uint8_t *tx_data, uint32_t tx_data_size)
{
    return this_spi_if.spi_write_burst(spi_handle, command, command_size, tx_data, tx_data_size);
}

bool concentrator_spi_enable(SPI_handle_t spi_handle)
{

    return this_spi_if.spi_enable(spi_handle);
}

bool concentrator_spi_disable(SPI_handle_t spi_handle)
{
    return this_spi_if.spi_disable(spi_handle);
}

bool concentrator_spi_write_read(SPI_handle_t spi_handle, uint8_t *tx_buffer, uint8_t *rx_buffer, uint32_t buffersize)
{
    return this_spi_if.spi_write_read(spi_handle, tx_buffer, rx_buffer, buffersize);
}

bool concentrator_spi_get_handle(SPI_handle_t *spi_handle)
{
    return this_spi_if.spi_get_handle(spi_handle);
}