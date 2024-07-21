#include "packet_forwarder_task.h"
#include "semtech_packet.h"
#include "time_ntp.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "log.h"

#define HEADER_SIZE 12

static SemaphoreHandle_t send_status_sem;
static uint64_t mac = 0xA84041FDFEEFBE63;

static gateway_config_t gateway_config;
static gateway_stats_t gateway_stats;

static char stat_packet[512];
char rx_packet[1024];

static struct udp_pcb *pcb_rxpt;
static struct udp_pcb *pcb_status;
static ip_addr_t addr;

static volatile bool time_set = false;

static void send_status_packet()
{
    uint32_t packet_size = 0;

    gateway_stats.time = time(NULL);
    semtech_packet_create_stat(stat_packet, sizeof(stat_packet), &packet_size, &gateway_stats);

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, packet_size, PBUF_RAM);
    memcpy(p->payload, stat_packet, packet_size);
    err_t er = udp_sendto(pcb_status, p, &addr, UDP_PORT);
    pbuf_free(p);
    if (er != ERR_OK)
    {
        log_error("Failed to send send_status_packet packet! error=%d", er);
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

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, packet_size, PBUF_RAM);
    memcpy(p->payload, rx_packet, packet_size);
    err_t er = udp_sendto(pcb_rxpt, p, &addr, UDP_PORT);
    pbuf_free(p);
    if (er != ERR_OK)
    {
        log_error("Failed to send_lora_package! lora_package error=%d", er);
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

static void time_set_cb()
{
    time_set = true;
}

static void sending_task(void *pvParameters)
{

    semtech_packet_init(gateway_config);

    log_info("Initialize  cyw43_arch");
    if (cyw43_arch_init())
    {
        log_error("fail: cyw43_arch_init");
    }

    cyw43_arch_enable_sta_mode();

    log_info("Connecting to Wi-Fi %s", WIFI_SSID);
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        log_error("fail cyw43_arch_wifi_connect_timeout_ms");
    }
    else
    {
        log_info("WiFi connected");
    }


    log_info("start to get NTP Time");
    time_npt_set_time(time_set_cb);

    static int wait_ms = 0;
    pcb_status = udp_new();
    if (pcb_status == NULL)
    {
        log_error("udp_new failed: pcb_status");
    }
    else
    {
        log_info("udp_new for pcb_status successful");
    }
    pcb_rxpt = udp_new();
    if (pcb_rxpt == NULL)
    {
        log_error("udp_new failed: pcb_rxpt");
    }
    else
    {
        log_info("udp_new for pcb_rxpt successful");
    }

    if (!ipaddr_aton(LORA_LNS_IP, &addr))
    {
        log_error("can not convert %s into ip address", LORA_LNS_IP);
    }
    else
    {
        log_info("LORA_LNS_IP: %s", LORA_LNS_IP);
    }

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
        cyw43_arch_poll();
        vTaskDelay(pdTICKS_TO_MS(1));
    }
    cyw43_arch_deinit();
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

static void init_gateway_config()
{
    gateway_config.mac_address = mac;
}
void packet_forwarder_task_init(void)
{
    init_gateway_config();

    send_status_sem = xSemaphoreCreateBinary();
    if (!send_status_sem)
    {
        log_error("xSemaphoreCreateBinary failed");
    }

    lora_rx_packet_queue = xQueueCreate(QUEUE_LENGTH, sizeof(lora_rx_packet_t));
    if (!lora_rx_packet_queue)
    {
        log_error("create lora_rx_packet_queue failed");
    }

    TaskHandle_t sending_task_handle;
    TaskHandle_t status_task_handle;

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
}