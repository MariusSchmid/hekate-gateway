#ifndef F1CAD709_63B5_43D2_9191_D94B8FDF2D24
#define F1CAD709_63B5_43D2_9191_D94B8FDF2D24

#include <stdint.h>
#include <stdbool.h>

typedef struct SPI_handle_s* SPI_handle_t;

typedef bool (*spi_set_chip_select_fptr_t)(SPI_handle_t spi_handle, bool enable);
typedef bool (*spi_write_read_fptr_t)(SPI_handle_t spi_handle, uint8_t *tx_buffer, uint8_t *rx_buffer, uint32_t buffersize);
typedef bool (*spi_enable_fptr_t)(SPI_handle_t spi_handle);
typedef bool (*spi_disable_fptr_t)(SPI_handle_t spi_handle);
typedef bool (*spi_get_handle_t)(SPI_handle_t *spi_handle);

typedef  struct concentrator_spi_if {
    spi_write_read_fptr_t spi_write_read;
	spi_enable_fptr_t spi_enable;
	spi_disable_fptr_t spi_disable;
	spi_set_chip_select_fptr_t spi_set_chip_select;
	spi_get_handle_t spi_get_handle;
} concentrator_spi_if_t;

#endif /* F1CAD709_63B5_43D2_9191_D94B8FDF2D24 */
