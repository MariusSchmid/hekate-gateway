#include "gateway_task.h"
#include "concentrator.h"
#include "concentrator_types.h"
#include "packet_forwarder_task.h"


#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

#define LORA_EN_PIN 18
#define SPI_LORA_CLK 2
#define SPI_LORA_MOSI 3
#define SPI_LORA_MISO 4
#define SPI_LORA_CS 5
#define SPI_INSTANCE spi0

static concentrator_spi_if_t spi_if;

struct SPI_handle_s
{
    uint8_t fd;
} spi_handle_internal;

static bool received_msg_cb(lora_rx_packet_t *rx_packet)
{
    printf("  count_us: %u\n", rx_packet->count_us);
    printf("  size:     %u\n", rx_packet->size);
    printf("  chan:     %u\n", rx_packet->if_chain);
    printf("  status:   0x%02X\n", rx_packet->status);
    printf("  datr:     %u\n", rx_packet->datarate);
    printf("  codr:     %u\n", rx_packet->coderate);
    printf("  rf_chain  %u\n", rx_packet->rf_chain);
    printf("  freq_hz   %u\n", rx_packet->freq_hz);
    printf("  snr_avg:  %.1f\n", rx_packet->snr);
    printf("  rssi_chan:%.1f\n", rx_packet->rssic);
    printf("  rssi_sig :%.1f\n", rx_packet->rssis);
    printf("  crc:      0x%04X\n", rx_packet->crc);
    for (int j = 0; j < rx_packet->size; j++)
    {
        printf("%02X ", rx_packet->payload[j]);
    }
    printf("\n");

    packet_forwarder_task_send_upstream(rx_packet);
    return true;
}

static bool spi_set_chip_select(SPI_handle_t spi_handle, bool enable)
{
    return true;
}
static bool spi_write_read(SPI_handle_t spi_handle, uint8_t *tx_buffer, uint8_t *rx_buffer, uint32_t buffersize)
{
    uint8_t num_bytes = 0;
    gpio_put(SPI_LORA_CS, false);
    sleep_ms(2);
    num_bytes = spi_write_read_blocking(SPI_INSTANCE, tx_buffer, rx_buffer, buffersize);
    sleep_ms(2);
    gpio_put(SPI_LORA_CS, true);
    return true;
}

static bool spi_write_read_burst(SPI_handle_t spi_handle, uint8_t *command, uint32_t command_size, uint8_t *rx_buffer, uint32_t rx_buffersize)
{
    uint8_t num_bytes = 0;
    gpio_put(SPI_LORA_CS, false);
    sleep_ms(2);
    num_bytes = spi_write_blocking(SPI_INSTANCE, command, command_size);
    num_bytes = spi_read_blocking(SPI_INSTANCE, 0x00, rx_buffer, rx_buffersize);
    sleep_ms(2);
    gpio_put(SPI_LORA_CS, true);
    return true;
}
static bool spi_write_burst(SPI_handle_t spi_handle, uint8_t *command, uint32_t command_size, const uint8_t *tx_buffer, uint32_t tx_buffersize)
{
    uint8_t num_bytes = 0;
    gpio_put(SPI_LORA_CS, false);
    sleep_ms(2);
    num_bytes = spi_write_blocking(SPI_INSTANCE, command, command_size);
    num_bytes = spi_write_blocking(SPI_INSTANCE, tx_buffer, tx_buffersize);
    sleep_ms(2);
    gpio_put(SPI_LORA_CS, true);
}

static bool spi_enable(SPI_handle_t spi_handle)
{
    return true;
}
static bool spi_disable(SPI_handle_t spi_handle)
{
    return true;
}
static bool spi_get_handle(SPI_handle_t *spi_handle)
{
    static uint8_t fd = 0;
    spi_handle_internal.fd = fd++;
    *spi_handle = &spi_handle_internal;
    return true;
}

static bool init_spi()
{
#if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_RX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
#warning spi/bme280_spi example requires a board with SPI pins
#endif

    spi_init(SPI_INSTANCE, 100 * 1000);
    // spi_init(SPI_INSTANCE, 500 * 1000);
    gpio_set_function(SPI_LORA_CLK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_LORA_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI_LORA_MISO, GPIO_FUNC_SPI);
    gpio_init(SPI_LORA_CS);
    gpio_put(SPI_LORA_CS, true);
    gpio_set_dir(SPI_LORA_CS, GPIO_OUT);
    bi_decl(bi_3pins_with_func(SPI_LORA_MISO, SPI_LORA_MOSI, SPI_LORA_CLK, GPIO_FUNC_SPI));
}

static void gateway_task(void *pvParameters)
{
    const uint lora_en_pin = LORA_EN_PIN;

    // Initialize LoRa enable pin
    gpio_init(lora_en_pin);
    gpio_set_dir(lora_en_pin, GPIO_OUT);

    gpio_put(lora_en_pin, false);
    sleep_ms(100);
    gpio_put(lora_en_pin, true);
    sleep_ms(100);

    init_spi();

    // concentrator
    spi_if.spi_disable = spi_disable;
    spi_if.spi_enable = spi_enable;
    spi_if.spi_set_chip_select = spi_set_chip_select;
    spi_if.spi_get_handle = spi_get_handle;
    spi_if.spi_write_read = spi_write_read;
    spi_if.spi_write_read_burst = spi_write_read_burst;
    spi_if.spi_write_burst = spi_write_burst;

    if (!concentrator_init(spi_if))
    {
        printf("Failed: concentrator_init \r\n");
    }

    if (!concentrator_start())
    {
        printf("Failed: concentrator_start \r\n");
    }

    uint32_t packets_received = 0;
    while (true)
    {
        if (!concentrator_receive(received_msg_cb, &packets_received))
        {
            printf("Failed: concentrator_receive \r\n");
        }

        if (packets_received == 0)
        {
            vTaskDelay(10);
        }
    }
}

void gateway_task_init(void)
{

    BaseType_t ret = xTaskCreate(gateway_task,
                                 "GW_TASK",
                                 1024 * 8,
                                 NULL,
                                 1,
                                 NULL);
}