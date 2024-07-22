#include "packet_forwarder_task.h"
#include "semtech_packet.h"
#include "internet_task_if.h"
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

static char stat_packet[512];
char rx_packet[1024];

static void send_status_packet()
{
    uint32_t packet_size = 0;

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
    if (!internet_task_send_udp(stat_packet, packet_size, LORA_LNS_IP, UDP_PORT))
    {
        log_error("internet_task_send_udp");
    }
    else
    {
        log_info("sending: %s", &stat_packet[12]);
    }
}

static void send_lora_package(lora_rx_packet_t *packet)
{
    uint32_t packet_size = 0;

    semtech_packet_create_rxpk(rx_packet, sizeof(rx_packet), &packet_size, packet);

    if (!internet_task_send_udp(stat_packet, packet_size, LORA_LNS_IP, UDP_PORT))
    {
        log_error("internet_task_send_udp");
    }
    else
    {
        log_info("sending: %s", &rx_packet[12]);
    }
}

#define QUEUE_LENGTH 16
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

static void sending_task(void *pvParameters)
{

    semtech_packet_init(gateway_config);

    log_info("sending_task started");
    lora_rx_packet_t lora_rx_packet;

    while (1)
    {
        if (uxSemaphoreGetCount(send_status_sem) > 0)
        {
            xSemaphoreTake(send_status_sem, 0);
            send_status_packet();
        }
        if (xQueueReceive(lora_rx_packet_queue, &(lora_rx_packet), 0) == pdPASS)
        {
            send_lora_package(&lora_rx_packet);
        }
        vTaskDelay(pdTICKS_TO_MS(1));
    }
}

static bool time_set = false;
static bool set_time_callback(struct tm time)
{
    struct timeval now = {0};
    if (xSemaphoreTake(time_mutex, 100) == pdTRUE)
    {
        log_info("set time: %02d/%02d/%04d %02d:%02d:%02d", time.tm_mday, time.tm_mon + 1, time.tm_year + 1900,
                 time.tm_hour, time.tm_min, time.tm_sec);
        now.tv_sec = mktime(&time);
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
        if (time_set)
        {
            if (pdTRUE != xSemaphoreGive(send_status_sem))
            {
                log_error("xSemaphoreGive failed: send_status_sem");
            }
        }
        vTaskDelay(pdTICKS_TO_MS(STATUS_INTERVAL_MS));
    }
}

static void set_time_task(void *pvParameters)
{
    internet_task_register_time_callback(set_time_callback);
    while (1)
    {
        internet_task_trigger_get_time();
        vTaskDelay(pdTICKS_TO_MS(TIME_INTERVAL_S * 1000));
    }
}

static void init_gateway_config()
{
    gateway_config.mac_address = mac;
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
    if (!send_status_sem)
    {
        log_error("xSemaphoreCreateMutex failed: time_mutex");
    }

    lora_rx_packet_queue = xQueueCreate(QUEUE_LENGTH, sizeof(lora_rx_packet_t));
    if (!lora_rx_packet_queue)
    {
        log_error("create lora_rx_packet_queue failed");
    }

    TaskHandle_t sending_task_handle;
    TaskHandle_t status_task_handle;
    TaskHandle_t set_time_task_handle;

    BaseType_t ret = xTaskCreate(sending_task,
                                 "PFW_TASK",
                                 1024 * 8,
                                 NULL,
                                 1,
                                 &sending_task_handle);

    if (ret != pdPASS)
    {
        log_error("xTaskCreate failed: sending_task");
    }

    ret = xTaskCreate(status_task,
                      "STATUS_TASK",
                      128,
                      NULL,
                      1,
                      &status_task_handle);
    if (ret != pdPASS)
    {
        log_error("xTaskCreate failed: status_task");
    }
    ret = xTaskCreate(set_time_task,
                      "STATUS_TASK",
                      1024 * 8,
                      NULL,
                      1,
                      &set_time_task_handle);
    if (ret != pdPASS)
    {
        log_error("xTaskCreate failed: status_task");
    }
#endif
}