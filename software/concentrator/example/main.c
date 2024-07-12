#include <stdio.h>
#include "concentrator.h"

struct SPI_handle_s
{
    uint8_t fd;
} spi_handle_internal;

static bool spi_set_chip_select(SPI_handle_t spi_handle, bool enable)
{
    printf("chip select \n");
    return true;
}
static bool spi_write_read(SPI_handle_t spi_handle, uint8_t *tx_buffer, uint8_t *rx_buffer, uint32_t buffersize)
{
    printf("spi_write_read \n");
    return true;
}
static bool spi_enable(SPI_handle_t spi_handle)
{

    printf("spi_enable \n");
    return true;
}
static bool spi_disable(SPI_handle_t spi_handle)
{
    printf("spi_disable \n");
    return true;
}
static bool spi_get_handle(SPI_handle_t *spi_handle)
{
    spi_handle_internal.fd = 1;
    *spi_handle = &spi_handle_internal;
    return true;
}

int main(int argc, char const *argv[])
{
    concentrator_spi_if_t spi_if;
    spi_if.spi_enable = spi_enable;
    spi_if.spi_disable = spi_disable;
    spi_if.spi_write_read = spi_write_read;
    spi_if.spi_set_chip_select = spi_set_chip_select;
    spi_if.spi_get_handle = spi_get_handle;
    concentrator_init(spi_if);
    concentrator_start(NULL);
    return 0;
}
