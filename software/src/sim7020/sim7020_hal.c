#include "sim7020_hal.h"

#include "log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"

#include <string.h>

#define PRINT_RAW_RCV_UART 0

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

const uint sim_pwr_key = SIM_PWR_PIN;
const uint sim_en_key = SIM_EN_PIN;
const uint sim_reset_key = SIM_RESET_PIN;

#define UART_RX_QUEUE_LENGTH 32
#define PKT_SIZE MAX_UART_RESPONSE

static QueueHandle_t uart_rx_packet_queue;

#define MAX_UART_RESPONSE 128

typedef struct uart_response_s
{
    char response[MAX_UART_RESPONSE];
    uint32_t size;
} uart_response_t;

static uart_response_t current_uart_response = {0};

static void put_response_to_queue()
{

    if (xQueueSendFromISR(uart_rx_packet_queue,
                          (void *)&current_uart_response,
                          (TickType_t)0) != pdPASS)
    {
        log_error("fail to add something to queue :lora_rx_packet_queue");
    }
}

static void on_uart_rx()
{
    static uint32_t uart_cnt = 0;
    while (uart_is_readable(UART_ID))
    {
        uint8_t ch = uart_getc(UART_ID);
#if PRINT_RAW_RCV_UART == 0
        printf("%c", ch);
#endif
        if (uart_cnt >= MAX_UART_RESPONSE)
        {
            log_error("uart recv overflow");
            uart_cnt = 0;
        }
        current_uart_response.response[uart_cnt] = ch;
        if (ch == '\n')
        {
            current_uart_response.response[uart_cnt] = '\0';
            current_uart_response.size = uart_cnt;
            put_response_to_queue();
            memset(&current_uart_response, 0, sizeof(current_uart_response));
            uart_cnt = 0;
        }
        else
        {
            uart_cnt++;
        }
    }
}

bool sim7020_hal_uart_init(void)
{
    if (!uart_init(UART_ID, BAUD_RATE))
    {
        log_error("uart_init SIM7020 failed");
        return false;
    }
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_ID, false);
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);
    return true;
}

bool wait_for_resp(uint32_t timeout_ms, char *expected_result, char *uart_response, uint16_t uart_response_buffer_size)
{
    uart_response_t response_queue_entry;
    uint32_t timeout_cnt = 0;
    while (true)
    {
        if (timeout_cnt > timeout_ms)
        {
            return false;
        }
        if (xQueueReceive(uart_rx_packet_queue, &response_queue_entry, pdTICKS_TO_MS(100)) == pdPASS)
        {

            if (strstr(response_queue_entry.response, expected_result) != NULL)
            {
                strncat(uart_response, response_queue_entry.response, uart_response_buffer_size - 1);
                return true;
            }
        }
        timeout_cnt += 100;
    }
    log_error("wait_for_resp timeout");
    return false;
}

bool sim7020_hal_send_cmd_get_recv(char *cmd, char *expected_result, uint32_t timeout, char *uart_response, uint16_t uart_response_buffer_size)
{
    BaseType_t res = xQueueReset(uart_rx_packet_queue);
    if (res != pdPASS)
    {
        log_error("fail to reset queue: uart_rx_packet_queue");
    }
    printf("S: %s \n", cmd);
    uart_puts(UART_ID, cmd);
    if (!wait_for_resp(timeout, expected_result, uart_response, uart_response_buffer_size))
    {
        log_error("%s not found in response for %s", expected_result, cmd);
        return false;
    }
    // log_info("%s%s", cmd, uart_response->response);
    return true;
}

bool sim7020_hal_send_cmd_check_recv(char *cmd, char *expected_result, uint32_t timeout)
{
    uart_response_t uart_response;
    sim7020_hal_send_cmd_get_recv(cmd, expected_result, timeout, uart_response.response,sizeof(uart_response.response));
    return true;
}

bool sim7020_hal_send(uint8_t *message, uint32_t size)
{
    uart_write_blocking(UART_ID, message, size);
    return true;
}

bool sim7020_hal_enable_sim_module(void)
{
    gpio_put(sim_pwr_key, true); // press power key
    sleep_ms(2000);
    gpio_put(sim_pwr_key, false); // release power key
    sleep_ms(5000);
    return true;
}

bool sim7020_hal_sim_gpio_init()
{
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
}

bool sim7020_hal_init(void)
{
    uart_rx_packet_queue = xQueueCreate(UART_RX_QUEUE_LENGTH, sizeof(uart_response_t));
    if (!uart_rx_packet_queue)
    {
        log_error("create uart_rx_packet_queue  failed");
    }
    return false;
}
