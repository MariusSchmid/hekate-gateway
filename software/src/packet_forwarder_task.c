#include "packet_forwarder_task.h"
#include "application_config.h"
#include "semtech_packet.h"
#include "internet_task_if.h"
#include "free_rtos_memory.h"
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "log.h"

#include <sys/time.h>

#define HEADER_SIZE 12

static SemaphoreHandle_t send_status_sem;
static SemaphoreHandle_t time_mutex;
static uint64_t mac = 0xA84041FDFEEFBE63;

static gateway_config_t gateway_config;
static gateway_stats_t gateway_stats;

static TaskHandle_t forwarding_task_handle;
#define FORWARDING_TASK_STACK_SIZE_WORDS 1024
static StackType_t forwarding_task_stack[FORWARDING_TASK_STACK_SIZE_WORDS];
static StaticTask_t forwarding_task_buffer;

static TaskHandle_t status_task_handle;
#define STATUS_TASK_STACK_SIZE_WORDS 1024
static StackType_t status_task_stack[STATUS_TASK_STACK_SIZE_WORDS];
static StaticTask_t status_task_buffer;

static TaskHandle_t set_time_task_handle;
#define SET_TIME_TASK_STACK_SIZE_WORDS 1024
static StackType_t set_time_task_stack[SET_TIME_TASK_STACK_SIZE_WORDS];
static StaticTask_t set_time_task_buffer;

static bool try_to_connect(uint32_t nr_retries)
{
    for (size_t i = 0; i < nr_retries; i++)
    {
        if (!internet_task_connect(LORA_LNS_IP, LORA_LNS_PORT))
        {
            log_warn("internet_task_connect failed, retry....");
        }
        else
        {
            return true;
        }
    }
    return false;
}

static void send_status_packet()
{
    uint32_t packet_size = 0;
    static char stat_packet[512];
    if (xSemaphoreTake(time_mutex, 100) == pdTRUE)
    {
        gateway_stats.time = time(NULL);
        xSemaphoreGive(time_mutex);
    }
    else
    {
        log_error("cant get time_mutex");
    }
    semtech_packet_create_stat(stat_packet, sizeof(stat_packet), &packet_size, &gateway_stats);

    if (!try_to_connect(3))
    {
        log_error("internet_task_connect failed");
        return;
    }

    if (!internet_task_send_udp(stat_packet, packet_size))
    {
        log_error("internet_task_send_udp failed");
        return;
    }
    if (!internet_task_disconnect())
    {
        log_error("internet_task_disconnect failed");
        return;
    }
    log_info("sending: %s", &stat_packet[12]);
}

static void send_lora_package(lora_rx_packet_t *packet)
{
    uint32_t packet_size = 0;
    char rx_packet[1024];
    semtech_packet_create_rxpk(rx_packet, sizeof(rx_packet), &packet_size, packet);
    log_info("sending: %s", &rx_packet[12]);
    if (!internet_task_send_udp(rx_packet, packet_size))
    {
        log_error("internet_task_send_udp failed");
        return;
    }
}

#define QUEUE_LENGTH 64
static StaticQueue_t lora_rx_packet_static_queue;
static uint8_t lora_rx_packet_queue_storage_area[QUEUE_LENGTH * sizeof(lora_rx_packet_t)];
static QueueHandle_t lora_rx_packet_queue;
void packet_forwarder_task_send_upstream(lora_rx_packet_t *packet)
{
    static int once = true;
    once = false;
    if (xQueueSend(lora_rx_packet_queue,
                   (void *)packet,
                   (TickType_t)0) != pdPASS)
    {
        log_error("fail to add something to queue :lora_rx_packet_queue");
    }
    else
    {
        log_info("Lora Packet recieved from conentrator");
    }
}

static void forwarding_task(void *pvParameters)
{

    semtech_packet_init(gateway_config);

    log_info("forwarding_task started");
    lora_rx_packet_t lora_rx_packet;

    while (1)
    {
        if (uxSemaphoreGetCount(send_status_sem) > 0)
        {
            xSemaphoreTake(send_status_sem, 0);
            send_status_packet();
        }
        UBaseType_t number_of_packets = uxQueueMessagesWaiting(lora_rx_packet_queue);
        if (number_of_packets > 0)
        {
            if (!try_to_connect(3))
            {
                log_error("internet_task_connect failed");
            }
            else
            {
                while (true)
                {
                    if (xQueueReceive(lora_rx_packet_queue, &(lora_rx_packet), 2000) == pdPASS)
                    {
                        send_lora_package(&lora_rx_packet);
                    }
                    else
                    {
                        break;
                    }
                }
            }
            if (!internet_task_disconnect())
            {
                log_error("internet_task_disconnect failed");
            }
        }

        vTaskDelay(pdTICKS_TO_MS(1));
    }
}

static bool time_set = false;

static bool set_time(struct tm *time)
{
    struct timeval now = {0};
    if (xSemaphoreTake(time_mutex, 100) == pdTRUE)
    {
        log_info("set time: %02d/%02d/%04d %02d:%02d:%02d", time->tm_mday, time->tm_mon + 1, time->tm_year + 1900,
                 time->tm_hour, time->tm_min, time->tm_sec);
        now.tv_sec = mktime(time);
        settimeofday(&now, NULL);
        time_set = true;
        xSemaphoreGive(time_mutex);
    }
    else
    {
        log_error("Can not get time_mutex");
        return false;
    }
    return true;
}

static void status_task(void *pvParameters)
{
    while (1)
    {
#if 1
        if (time_set)
        {
            if (pdTRUE != xSemaphoreGive(send_status_sem))
            {
                log_error("xSemaphoreGive failed: send_status_sem");
            }
        }
#endif
        vTaskDelay(pdTICKS_TO_MS(LORA_STATUS_INTERVAL_MINUTES * 60 * 1000));
    }
}

static void set_time_task(void *pvParameters)
{
    struct tm time;
    // internet_task_register_time_callback(set_time_callback);
    while (1)
    {
#if 1
        if (internet_task_get_time(&time))
        {
            set_time(&time);
            vTaskDelay(pdTICKS_TO_MS(SNTP_INTERVAL_MINUTES * 60 * 1000));
        }
        else
        {
            log_error("failed to get time!");
            vTaskDelay(pdTICKS_TO_MS(10 * 1000));
        }
#endif
    }
}

static void init_gateway_config()
{
    gateway_config.mac_address = mac;
}

void packet_forwarder_print_task_stats(void)
{
    free_rtos_memory_print_usage(forwarding_task_handle, "forwarding_task", FORWARDING_TASK_STACK_SIZE_WORDS * sizeof(UBaseType_t));
    free_rtos_memory_print_usage(status_task_handle, "status_task", STATUS_TASK_STACK_SIZE_WORDS * sizeof(UBaseType_t));
    free_rtos_memory_print_usage(set_time_task_handle, "set_time_task", SET_TIME_TASK_STACK_SIZE_WORDS * sizeof(UBaseType_t));
}

void packet_forwarder_task_init(void)
{

#if 1
    init_gateway_config();

    send_status_sem = xSemaphoreCreateBinary();
    if (!send_status_sem)
    {
        log_error("xSemaphoreCreateBinary failed: send_status_sem");
    }

    time_mutex = xSemaphoreCreateMutex();
    if (!time_mutex)
    {
        log_error("xSemaphoreCreateMutex failed: time_mutex");
    }

    lora_rx_packet_queue = xQueueCreateStatic(QUEUE_LENGTH, sizeof(lora_rx_packet_t), lora_rx_packet_queue_storage_area, &lora_rx_packet_static_queue);
    if (!lora_rx_packet_queue)
    {
        log_error("create lora_rx_packet_queue failed");
    }

    forwarding_task_handle = xTaskCreateStatic(forwarding_task,
                                               "PFW_TASK",
                                               FORWARDING_TASK_STACK_SIZE_WORDS,
                                               NULL,
                                               1,
                                               forwarding_task_stack,
                                               &forwarding_task_buffer);

    if (forwarding_task_handle == NULL)
    {
        log_error("xTaskCreate failed: forwarding_task");
    }

    TaskHandle_t status_task_handle = xTaskCreateStatic(status_task,
                                                        "STATUS_TASK",
                                                        STATUS_TASK_STACK_SIZE_WORDS,
                                                        NULL,
                                                        1,
                                                        status_task_stack,
                                                        &status_task_buffer);
    if (status_task_handle == NULL)
    {
        log_error("xTaskCreate failed: status_task");
    }
    TaskHandle_t set_time_task_handle = xTaskCreateStatic(set_time_task,
                                                          "SET_TIME_TASK",
                                                          SET_TIME_TASK_STACK_SIZE_WORDS,
                                                          NULL,
                                                          1,
                                                          set_time_task_stack,
                                                          &set_time_task_buffer);
    if (set_time_task_handle == NULL)
    {
        log_error("xTaskCreate failed: status_task");
    }
#endif
}