#ifndef EXTERNAL_SX1302_HAL_V2_1_0_CUSTOM_LORAGW_CUSTOM_INIT
#define EXTERNAL_SX1302_HAL_V2_1_0_CUSTOM_LORAGW_CUSTOM_INIT

struct SPI_handle_s;
int loragw_spi_init(struct SPI_handle_s *spi_handle);
int sx1250_spi_init(struct SPI_handle_s *spi_handle);
#endif /* EXTERNAL_LORA_GATEWAY_5_0_1_LORA_GATEWAY_LIBLORAGW_INC_CUSTOM_INIT */
