#include "internet_task_if.h"
#include "hekate_utils.h"
#include "free_rtos_memory.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "log.h"
#include "semphr.h"

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"
// #include "pico/stdlib.h"

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

static set_time_callback_t this_time_callback;




#define MAX_UART_RESPONSE 128
typedef struct uart_response_s
{
    char response[MAX_UART_RESPONSE];
    uint32_t size;
} uart_response_t;

static uart_response_t current_uart_response = {0};

#define UART_RX_QUEUE_LENGTH 32
#define PKT_SIZE MAX_UART_RESPONSE
static QueueHandle_t uart_rx_packet_queue;

static SemaphoreHandle_t sim_init_done_sem;
static SemaphoreHandle_t sim_mutex;

TaskHandle_t sim7020_task_handle;
#define SIM7020_TASK_STACK_SIZE_WORDS 1024
StackType_t sim7020_task_stack[SIM7020_TASK_STACK_SIZE_WORDS];
StaticTask_t sim7020_task_buffer;

static void put_response_to_queue()
{

    if (xQueueSendFromISR(uart_rx_packet_queue,
                          (void *)&current_uart_response,
                          (TickType_t)0) != pdPASS)
    {
        log_error("fail to add something to queue :lora_rx_packet_queue");
    }
}

bool wait_for_resp(uint32_t timeout_ms, char *expected_result, uart_response_t *uart_response)
{

    uint32_t timeout_cnt = 0;
    while (true)
    {
        if (timeout_cnt > timeout_ms)
        {
            return false;
        }
        if (xQueueReceive(uart_rx_packet_queue, uart_response, pdTICKS_TO_MS(100)) == pdPASS)
        {

            if (strstr(uart_response->response, expected_result) != NULL)
            {
                return true;
            }
        }
        timeout_cnt += 100;
    }
    log_error("wait_for_resp timeout");
    return false;
}

void on_uart_rx()
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

static bool sim_uart_init()
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

static void sim_gpio_init()
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

static void enable_sim_module()
{
    uart_response_t uart_response;
    gpio_put(sim_pwr_key, true); // press power key
    sleep_ms(2000);
    gpio_put(sim_pwr_key, false); // release power key
    // wait_for_resp(10000, "OK", &uart_response);
    sleep_ms(5000);
}

static bool send_cmd_get_recv(char *cmd, char *expected_result, uint32_t timeout, uart_response_t *uart_response)
{
    BaseType_t res = xQueueReset(uart_rx_packet_queue);
    if (res != pdPASS)
    {
        log_error("fail to reset queue: uart_rx_packet_queue");
    }
    printf("S: %s \n", cmd);
    uart_puts(UART_ID, cmd);
    if (!wait_for_resp(timeout, expected_result, uart_response))
    {
        log_error("%s not found in response for %s", expected_result, cmd);
        return false;
    }
    log_info("%s%s", cmd, uart_response->response);
    return true;
}
static bool send_cmd_check_recv(char *cmd, char *expected_result, uint32_t timeout)
{
    uart_response_t uart_response;
    send_cmd_get_recv(cmd, expected_result, timeout, &uart_response);
}

static void set_apn()
{
    send_cmd_check_recv("AT+CFUN=0\r\n", "READY", 10000);
    send_cmd_check_recv("AT*MCGDEFCONT=\"IP\",\"iot.1nce.net\"\r\n", "OK", 10000); // set apn settings
    send_cmd_check_recv("AT+CFUN=1\r\n", "READY", 10000);
}

static bool parse_ntp_string(char *ntp_string, struct tm *time)
{
    char search_string[] = "+CCLK: ";
    ntp_string = strstr(ntp_string, search_string);
    if (!ntp_string)
    {
        return false;
    }
    ntp_string = ntp_string + sizeof(search_string) - 1;
    log_info("parse %s", ntp_string);

    if (strlen(ntp_string) <= 16)
    {
        return false;
    }
    char year_str[2];
    year_str[0] = ntp_string[0];
    year_str[1] = ntp_string[1];

    char month_str[2];
    month_str[0] = ntp_string[3];
    month_str[1] = ntp_string[4];

    char day_str[2];
    day_str[0] = ntp_string[6];
    day_str[1] = ntp_string[7];

    char hour_str[2];
    hour_str[0] = ntp_string[9];
    hour_str[1] = ntp_string[10];

    char minute_str[2];
    minute_str[0] = ntp_string[12];
    minute_str[1] = ntp_string[13];

    char second_str[2];
    second_str[0] = ntp_string[15];
    second_str[1] = ntp_string[16];

    time->tm_year = strtol(year_str, NULL, 0) + 100;
    time->tm_mon = strtol(month_str, NULL, 0) - 1;
    time->tm_mday = strtol(day_str, NULL, 0);
    time->tm_hour = strtol(hour_str, NULL, 0);
    time->tm_min = strtol(minute_str, NULL, 0);
    time->tm_sec = strtol(second_str, NULL, 0);

    return true;
}

static void get_time_ntp()
{

#if CHINESE_NTP == 1
    send_cmd_check_recv("AT+CSNTPSTART=\"jp.ntp.org.cn\",\"+32\"\r\n", "OK", 20000);
#else
    send_cmd_check_recv("AT+CSNTPSTART=\"pool.ntp.org\",\"+48\"\r\n", "+CSNTP:", 20000);
#endif
    // send_cmd_check_recv("AT+CCLK?\r\n", "+CCLK:", 5000);
    uart_response_t uart_response;
    if (!send_cmd_get_recv("AT+CCLK?\r\n", "+CCLK:", 5000, &uart_response))
    {
        log_error("can not get ntp time");
    }
    else
    {
        struct tm time;
        if (!parse_ntp_string(uart_response.response, &time))
        {
            log_error("can parse ntp time: %s", uart_response.response);
        }
        else
        {
            if (this_time_callback != NULL)
            {
                this_time_callback(time);
            }
        }
    }

    send_cmd_check_recv("AT+CSNTPSTOP\r\n", "OK", 5000);
}

static bool initialize_sim_module()
{
    send_cmd_check_recv("AT\r\n", "OK", 5000);
    send_cmd_check_recv("AT+CMEE=2\r\n", "OK", 5000); // extended error report
    send_cmd_check_recv("AT+CPIN?\r\n", "READY", 1000);

    set_apn();
    // uart_puts(UART_ID, "AT+CGCONTRDP\r\n");
    send_cmd_check_recv("AT+CGCONTRDP\r\n", "OK", 5000); // get APN settings

    send_cmd_check_recv("AT+CIPMUX=0\r\n", "OK", 5000);  // enable single connection
    send_cmd_check_recv("AT+CIPMODE=1\r\n", "OK", 5000); // enable transparent mode

    return true;
}

static void sim7020_task(void *pvParameters)
{

    memset(&current_uart_response, 0, sizeof(current_uart_response));
    log_info("sim7020_task started");
    sim_uart_init();
    log_info("sim uart initialized");
    sim_gpio_init();
    log_info("sim gpio initialized");
    enable_sim_module();
    log_info("sim enabled");
    initialize_sim_module();
    xSemaphoreGive(sim_init_done_sem);

    while (true)
    {
        // log_info("Wait");
        vTaskDelay(pdTICKS_TO_MS(1000));
    }
}

bool internet_task_send_udp(uint8_t *message, uint32_t size, const char *dst_ip, uint16_t port)
{
    char connection_string[256] = {0};
    sprintf(connection_string, "AT+CIPSTART=\"UDP\",\"%s\",\"%d\"\r\n", dst_ip, port);
    if (xSemaphoreTake(sim_mutex, 10000) == pdTRUE)
    {

        send_cmd_check_recv("AT+CSOC=1,2,1\r\n", "+CSOC:", 10000);
        send_cmd_check_recv(connection_string, "CONNECT OK", 60000);
        send_cmd_check_recv("AT+CIPCHAN\r\n", "CONNECT", 10000); // enable transparent mode

        uart_write_blocking(UART_ID, message, size);
        sleep_ms(1500);
        uart_puts(UART_ID, "+++"); // disable transparent mode
        sleep_ms(1500);
        send_cmd_check_recv("AT+CIPCLOSE=0\r\n", "OK", 5000);
        xSemaphoreGive(sim_mutex);
    }
    return true;
}

bool internet_task_register_time_callback(set_time_callback_t time_callback)
{
    ENSURE_RET(time_callback, false);
    this_time_callback = time_callback;
}

bool internet_task_trigger_get_time(void)
{
    static bool sim_initialized = false;

    if (!sim_initialized)
    {
        if (xSemaphoreTake(sim_init_done_sem, pdTICKS_TO_MS(60000)) == pdTRUE)
        {
            sim_initialized = true;
        }
        else
        {
            log_error("failed to take sim_init_done_sem");
            return false;
        }
    }

    if (xSemaphoreTake(sim_mutex, 1000) == pdTRUE)
    {
        get_time_ntp();
        xSemaphoreGive(sim_mutex);
    }

    return true;
}

void internet_task_print_task_stats(void)
{
    free_rtos_memory_print_usage(sim7020_task_handle, "sim7020_task", SIM7020_TASK_STACK_SIZE_WORDS * sizeof(UBaseType_t));
}

bool internet_task_init(void)
{

    sim_mutex = xSemaphoreCreateMutex();
    if (!sim_mutex)
    {
        log_error("xSemaphoreCreateMutex failed: sim_mutex");
    }

    uart_rx_packet_queue = xQueueCreate(UART_RX_QUEUE_LENGTH, sizeof(uart_response_t));
    if (!uart_rx_packet_queue)
    {
        log_error("create uart_rx_packet_queue  failed");
    }

    sim7020_task_handle = xTaskCreateStatic(sim7020_task,
                                            "SIM7020_TASK",
                                            SIM7020_TASK_STACK_SIZE_WORDS,
                                            NULL,
                                            1,
                                            sim7020_task_stack,
                                            &sim7020_task_buffer);
    if (sim7020_task_handle == NULL)
    {
        log_error("xTaskCreate failed: sending_task");
        return false;
    }
    sim_init_done_sem = xSemaphoreCreateBinary();
    if (!sim_init_done_sem)
    {
        log_error("xSemaphoreCreateBinary failed: sim_init_done_sem");
    }

    return true;
}