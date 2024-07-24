#include "internet_task_if.h"
#include "hekate_utils.h"
#include "free_rtos_memory.h"
#include "sim7020_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "log.h"
#include "semphr.h"

#include "pico/stdlib.h"

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

static set_time_callback_t this_time_callback;



static SemaphoreHandle_t sim_init_done_sem;
static SemaphoreHandle_t sim_mutex;

TaskHandle_t sim7020_task_handle;
#define SIM7020_TASK_STACK_SIZE_WORDS 1024
StackType_t sim7020_task_stack[SIM7020_TASK_STACK_SIZE_WORDS];
StaticTask_t sim7020_task_buffer;

static void set_apn()
{
    sim7020_hal_send_cmd_check_recv("AT+CFUN=0\r\n", "READY", 10000);
    sim7020_hal_send_cmd_check_recv("AT*MCGDEFCONT=\"IP\",\"iot.1nce.net\"\r\n", "OK", 10000); // set apn settings
    sim7020_hal_send_cmd_check_recv("AT+CFUN=1\r\n", "READY", 10000);
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
    char response[128];
#if CHINESE_NTP == 1
    sim7020_hal_send_cmd_check_recv("AT+CSNTPSTART=\"jp.ntp.org.cn\",\"+32\"\r\n", "OK", 20000);
#else
    sim7020_hal_send_cmd_check_recv("AT+CSNTPSTART=\"pool.ntp.org\",\"+48\"\r\n", "+CSNTP:", 20000);
#endif

    if (!sim7020_hal_send_cmd_get_recv("AT+CCLK?\r\n", "+CCLK:", 5000, response, sizeof(response)))
    {
        log_error("can not get ntp time");
    }
    else
    {
        struct tm time;
        if (!parse_ntp_string(response, &time))
        {
            log_error("can parse ntp time: %s", response);
        }
        else
        {
            if (this_time_callback != NULL)
            {
                this_time_callback(time);
            }
        }
    }

    sim7020_hal_send_cmd_check_recv("AT+CSNTPSTOP\r\n", "OK", 5000);
}

static bool initialize_sim_module()
{
    sim7020_hal_send_cmd_check_recv("AT\r\n", "OK", 5000);
    sim7020_hal_send_cmd_check_recv("AT+CMEE=2\r\n", "OK", 5000); // extended error report
    sim7020_hal_send_cmd_check_recv("AT+CPIN?\r\n", "READY", 1000);

    set_apn();
    sim7020_hal_send_cmd_check_recv("AT+CGCONTRDP\r\n", "OK", 5000); // get APN settings

    sim7020_hal_send_cmd_check_recv("AT+CIPMUX=0\r\n", "OK", 5000);  // enable single connection
    sim7020_hal_send_cmd_check_recv("AT+CIPMODE=1\r\n", "OK", 5000); // enable transparent mode

    return true;
}

static void sim7020_task(void *pvParameters)
{

    // memset(&current_uart_response, 0, sizeof(current_uart_response));
    log_info("sim7020_task started");
    sim7020_hal_uart_init();
    log_info("sim uart initialized");
    sim7020_hal_sim_gpio_init();
    log_info("sim gpio initialized");
    sim7020_hal_enable_sim_module();
    log_info("sim enabled");
    initialize_sim_module();
    xSemaphoreGive(sim_init_done_sem);

    while (true)
    {
        // log_info("Wait");
        vTaskDelay(pdTICKS_TO_MS(1000));
    }
}
bool internet_task_connect(const char *dst_ip, uint16_t port)
{
    ENSURE_RET(dst_ip, false);
    char connection_string[256] = {0};
    sprintf(connection_string, "AT+CIPSTART=\"UDP\",\"%s\",\"%d\"\r\n", dst_ip, port);
    if (xSemaphoreTake(sim_mutex, 10000) == pdTRUE)
    {
        if (!sim7020_hal_send_cmd_check_recv("AT+CSOC=1,2,1\r\n", "+CSOC:", 10000))
        {
            sim7020_hal_send_cmd_check_recv("AT+CIPCLOSE=0\r\n", "OK", 5000);
            xSemaphoreGive(sim_mutex);
            return false;
        }

        sim7020_hal_send_cmd_check_recv(connection_string, "CONNECT OK", 60000);
        sim7020_hal_send_cmd_check_recv("AT+CIPCHAN\r\n", "CONNECT", 10000); // enable transparent mode
    }
    else
    {
        log_error("cant get sim_mutex");
        return false;
    }

    return true;
}
bool internet_task_disconnect(void)
{
    char exit_transparent_mode_sequence[] = "+++";
    sleep_ms(1500);
    sim7020_hal_send(exit_transparent_mode_sequence, 3);
    sleep_ms(1500);
    sim7020_hal_send_cmd_check_recv("AT+CIPCLOSE=0\r\n", "OK", 5000);
    xSemaphoreGive(sim_mutex);
}
bool internet_task_send_udp(uint8_t *message, uint32_t size)
{
    return sim7020_hal_send(message, size);
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

    sim7020_hal_init();

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