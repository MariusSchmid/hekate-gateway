#include "packet_forwarder_task.h"
#include "semtech_packet.h"
#include "time_ntp.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

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
        printf("Failed to send send_status_packet packet! error=%d", er);
    }
    else
    {
        printf("status: %s \n", &stat_packet[12]);
    }
}
// #define PKT_SIZE 128
// #define PKT_SIZE 256 + 128
// uint8_t tmp_pkg[PKT_SIZE] = {0};

static void send_lora_package(lora_rx_packet_t *packet)
{
    uint32_t packet_size = 0;
    // uint32_t packet_size = PKT_SIZE;

    semtech_packet_create_rxpk(rx_packet, sizeof(rx_packet), &packet_size, packet);

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, packet_size, PBUF_RAM);
    // for (size_t i = 0; i < PKT_SIZE; i++)
    // {
    //     tmp_pkg[i] = i;
    // }

    // memcpy(p->payload, tmp_pkg, packet_size);
    memcpy(p->payload, rx_packet, packet_size);
    err_t er = udp_sendto(pcb_rxpt, p, &addr, UDP_PORT);
    pbuf_free(p);
    if (er != ERR_OK)
    {
        printf("Failed to send_lora_package! lora_package error=%d", er);
    }
    else
    {
        printf("message: %s \n", &rx_packet[12]);
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
        printf("fail to add something to queue :lora_rx_packet_queue");
    }
    else
    {
        printf("packet_forwarder_task_send_upstream triggered");
    }
}

static void time_set_cb()
{
    time_set = true;
}

static void sending_task(void *pvParameters)
{
    gateway_config.mac_address = mac;
    semtech_packet_init(gateway_config);

    lora_rx_packet_queue = xQueueCreate(QUEUE_LENGTH,
                                        sizeof(lora_rx_packet_t));

    if (lora_rx_packet_queue == NULL)
    {
        printf("fail: lora_rx_packet_queue = xQueueCreate()\n");
    }

    if (cyw43_arch_init())
    {
        printf("fail: cyw43_arch_init\n");
    }
    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("fail cyw43_arch_wifi_connect_timeout_ms\n");
    }
    else
    {
        printf("Connected.\n");
    }

    // get timepbuf_alloc
    // printf("Get NTP Time.\n");
    time_npt_set_time(time_set_cb);

    static int wait_ms = 0;
    pcb_status = udp_new();
    if (pcb_status == NULL)
    {
        printf("error creating pcb_status.\n");
    }
    else
    {
        printf("udp_new pcb_status ok.\n");
    }
    pcb_rxpt = udp_new();
    if (pcb_rxpt == NULL)
    {
        printf("error creating pcb_rxpt.\n");
    }
    else
    {
        printf("pcb_rxpt pcb_status ok.\n");
    }
    ipaddr_aton(BEACON_TARGET, &addr);

    lora_rx_packet_t lora_rx_packet;
    int i = 1;
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
        // if (i++ % 5000 == 0)
        // {
        //     send_lora_package(&lora_rx_packet);
        // }
        // absolute_time_t wait_time;
        // wait_time._private_us_since_boot 1000;
        cyw43_arch_poll();
        // cyw43_arch_wait_for_work_until(100);
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
            xSemaphoreGive(send_status_sem);
        }
        vTaskDelay(pdTICKS_TO_MS(BEACON_INTERVAL_MS));
    }
}
#define TASK_STACK_SIZE 1024 * 8
StaticTask_t xTaskBuffer;
StackType_t xStack[TASK_STACK_SIZE];
TaskHandle_t xHandle = NULL;
void packet_forwarder_task_init(void)
{

    send_status_sem = xSemaphoreCreateBinary();

    TaskHandle_t sending_task_handle;
    TaskHandle_t status_task_handle;

#if 1
    BaseType_t ret = xTaskCreate(sending_task,
                                 "PFW_TASK",
                                 1024 * 8,
                                 NULL,
                                 1,
                                 &sending_task_handle);

    if (ret != pdPASS)
    {
        printf("error creating sending_task.\n");
    }
#else
    xHandle = xTaskCreateStatic(sending_task,
                                "PFW_TASK",
                                TASK_STACK_SIZE,
                                NULL,
                                1,
                                xStack,
                                &xTaskBuffer);

#endif

    BaseType_t ret2 = xTaskCreate(status_task,
                                  "STATUS_TASK",
                                  128,
                                  NULL,
                                  1,
                                  &status_task_handle);
    if (ret2 != pdPASS)
    {
        printf("error creating status_task.\n");
    }
}