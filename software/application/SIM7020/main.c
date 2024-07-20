

#include "pico/stdlib.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "hardware/uart.h"
#include "hardware/irq.h"

#include "tusb.h"

#include "log.h"

#define UART_ID uart1
#define UART_TX_PIN 8
#define UART_RX_PIN 9

#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

#define SIM_PWR_PIN 28
#define SIM_EN_PIN 19
#define SIM_RESET_PIN 26

#define WAIT_FOR_CDC 0

static bool uart_received = false;
static bool uart_cnt = 0;
// static char uart_response[1024];

// RX interrupt handler
void on_uart_rx()
{

    while (uart_is_readable(UART_ID))
    {
        uint8_t ch = uart_getc(UART_ID);
        printf("%c", ch);
    }
}

static void my_uart_init()
{

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);
}

static void set_apn()
{
    uart_puts(UART_ID, "AT+CFUN=0\r\n");
    sleep_ms(10000);
    uart_puts(UART_ID, "AT*MCGDEFCONT=\"IP\",\"iot.1nce.net\"\r\n");
    sleep_ms(1000);
    uart_puts(UART_ID, "AT+CFUN=1\r\n");
    sleep_ms(10000);
}

static void ntp_example()
{
    uart_puts(UART_ID, "AT\r\n");
    sleep_ms(1000);
    uart_puts(UART_ID, "AT+CMEE=2\r\n"); // extebded error report
    sleep_ms(1000);
    uart_puts(UART_ID, "AT+CPIN?\r\n");
    sleep_ms(1000);
    set_apn();
    uart_puts(UART_ID, "AT+CGCONTRDP\r\n");
    sleep_ms(5000);

#if CHINESE_NTP == 1
    uart_puts(UART_ID, "AT+CSNTPSTART=\"jp.ntp.org.cn\",\"+32\"\r\n");
#else
    uart_puts(UART_ID, "AT+CSNTPSTART=\"pool.ntp.org\",\"+48\"\r\n");
#endif
    sleep_ms(20000);
    uart_puts(UART_ID, "AT+CCLK?\r\n");
    sleep_ms(5000);
    uart_puts(UART_ID, "AT+CSNTPSTOP\r\n");
    sleep_ms(5000);
}

static void send_udp(){

}

int main()
{

#if 1
    if (!stdio_init_all())
    {
        log_error("failed: stdio_usb_init()");
    }
#endif
#if (WAIT_FOR_CDC == 1)
    while (!tud_cdc_connected())
    {
        printf(".");
        sleep_ms(500);
    }
#endif


    my_uart_init();
    log_info("staart");
    const uint sim_pwr_key = SIM_PWR_PIN;
    const uint sim_en_key = SIM_EN_PIN;
    const uint sim_reset_key = SIM_RESET_PIN;

    // Initialize LoRa enable pin
    gpio_init(sim_pwr_key);
    gpio_init(sim_en_key);
    gpio_init(sim_reset_key);

    gpio_set_dir(sim_pwr_key, GPIO_OUT);
    gpio_set_dir(sim_en_key, GPIO_OUT);
    gpio_set_dir(sim_reset_key, GPIO_OUT);

    gpio_put(sim_reset_key, false); // no reset
    gpio_put(sim_pwr_key, true);    // power key down

    sleep_ms(100);
    gpio_put(sim_en_key, true);
    sleep_ms(100);

    gpio_put(sim_pwr_key, false); // release power key
    sleep_ms(100);

    // gpio_put(sim_reset_key, false);
    // sleep_ms(100);
    // gpio_put(sim_reset_key, false);
    // sleep_ms(100);
    gpio_put(sim_pwr_key, true); // press power key
    sleep_ms(2000);
    gpio_put(sim_pwr_key, false); // release power key
    sleep_ms(100);

    ntp_example();
    while (true)
    {
        // uart_puts(UART_ID, "AT\r\n");
        sleep_ms(1000);
        // log_info("ok");
        // sleep_ms(500);
    }
}