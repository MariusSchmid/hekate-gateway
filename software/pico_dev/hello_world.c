#include <stdio.h>
#include "pico/stdlib.h"
#include "concentrator.h"
#include "concentrator_types.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#include "pico/cyw43_arch.h"

// #include "lwip/pbuf.h"
// #include "lwip/udp.h"

#define UDP_PORT 4444
#define BEACON_MSG_LEN_MAX 127
#define BEACON_TARGET "255.255.255.255"
#define BEACON_INTERVAL_MS 5000

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
void run_udp_beacon()
{
    struct udp_pcb *pcb = udp_new();

    ip_addr_t addr;
    ipaddr_aton(BEACON_TARGET, &addr);

    int counter = 0;
    while (true)
    {
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX + 1, PBUF_RAM);
        char *req = (char *)p->payload;
        memset(req, 0, BEACON_MSG_LEN_MAX + 1);
        snprintf(req, BEACON_MSG_LEN_MAX, "%d\n", counter);
        err_t er = udp_sendto(pcb, p, &addr, UDP_PORT);
        pbuf_free(p);
        if (er != ERR_OK)
        {
            printf("Failed to send UDP packet! error=%d", er);
        }
        else
        {
            printf("Sent packet %d\n", counter);
            counter++;
        }

        // Note in practice for this simple UDP transmitter,
        // the end result for both background and poll is the same

#if PICO_CYW43_ARCH_POLL
        // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
        // main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        sleep_ms(BEACON_INTERVAL_MS);
#else
        // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
        sleep_ms(BEACON_INTERVAL_MS);
#endif
    }
}

int main()
{

    if (cyw43_arch_init())
    {
        printf("failed to initialise\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("failed to connect.\n");
        return 1;
    }
    else
    {
        printf("Connected.\n");
    }
    run_udp_beacon();
    cyw43_arch_deinit();
    while (1)
    {
        sleep_ms(10);
    }

#if 0
    const uint lora_en_pin = LORA_EN_PIN;

    // Initialize LED pin
    gpio_init(lora_en_pin);
    gpio_set_dir(lora_en_pin, GPIO_OUT);

    gpio_put(lora_en_pin, false);
    sleep_ms(100);
    gpio_put(lora_en_pin, true);
    sleep_ms(100);

    // Initialize chosen serial port
    stdio_init_all();

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
        printf("Failed: concentrator_init\r\n");
    }

    if (!concentrator_start())
    {
        printf("Failed: concentrator_init\r\n");
    }

    uint32_t packets_received = 0;
    while (true)
    {
        if (!concentrator_receive(received_msg_cb, &packets_received))
        {
            printf("Failed: concentrator_init\r\n");
        }

        if (packets_received == 0)
        {
            sleep_ms(10);
        }
        
        // Blink LED
        // printf("Blinking!\r\n");
        // gpio_put(lora_en_pin, true);
        // sleep_ms(1000);
        // gpio_put(lora_en_pin, false);
        // sleep_ms(1000);
    }
#endif
}