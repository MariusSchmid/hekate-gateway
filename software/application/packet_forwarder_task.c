#include "packet_forwarder_task.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "FreeRTOS.h"
#include "task.h"

#define HEADER_SIZE 12

static uint32_t net_mac_h = 0xA84041FD;
static uint32_t net_mac_l = 0xFEEFBE63;
static uint64_t mac = 0xA84041FDFEEFBE63;

static void create_status_packet(char *msg, uint32_t *msg_size, uint32_t max_size)
{
    memset(msg, 0, max_size);
    msg[0] = 0x2;
    msg[1] = rand() % 127;
    msg[2] = rand() % 127;
    msg[3] = 0x00;
    msg[4] = net_mac_h >> 24;
    msg[5] = net_mac_h >> 16;
    msg[6] = net_mac_h >> 8;
    msg[7] = net_mac_h;
    msg[8] = net_mac_l >> 24;
    msg[9] = net_mac_l >> 16;
    msg[10] = net_mac_l >> 8;
    msg[11] = net_mac_l;
    *msg_size = snprintf(msg + HEADER_SIZE, max_size, "{\"stat\": {\"time\": \"2024-07-16 00:00:00 UTC\", \"rxnb\": 1, \"rxok\": 1, \"rxfw\": 1, \"ackr\": 100.0, \"dwnb\": 0, \"txnb\": 0}}");
    *msg_size += HEADER_SIZE;
}
struct udp_pcb *pcb;
static ip_addr_t addr;
static void send_status_packet()
{
    char data_to_send[BEACON_MSG_LEN_MAX];
    uint32_t msg_size = 0;
    create_status_packet(data_to_send, &msg_size, sizeof(data_to_send));
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, msg_size + 1, PBUF_RAM);
    create_status_packet((char *)p->payload, &msg_size, msg_size + 1);
    p->tot_len = p->tot_len - 1;
    err_t er = udp_sendto(pcb, p, &addr, UDP_PORT);
    pbuf_free(p);
    if (er != ERR_OK)
    {
        printf("Failed to send UDP packet! error=%d", er);
    }
}

void packet_forwarder_task_send_upstream(lora_rx_packet_t *packet)
{
}

static void sending_task(void *pvParameters)
{
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
    static int wait_ms = 0;
    pcb = udp_new();
    ipaddr_aton(BEACON_TARGET, &addr);
    while (1)
    {
        send_status_packet();
        cyw43_arch_poll();
        vTaskDelay(pdTICKS_TO_MS(BEACON_INTERVAL_MS));
    }
    cyw43_arch_deinit();
}

static void status_task(void *pvParameters)
{
    // put something in status queue
    while (1)
    {

        vTaskDelay(pdTICKS_TO_MS(10));
    }
}

void packet_forwarder_task_init(void)
{

    TaskHandle_t sending_task_handle;
    TaskHandle_t status_task_handle;
    // data_sending_task(NULL);
    BaseType_t ret = xTaskCreate(sending_task,
                                 "PFW_TASK",
                                 1024 * 16,
                                 NULL,
                                 1,
                                 &sending_task_handle);

    ret = xTaskCreate(status_task,
                      "STATUS_TASK",
                      128,
                      NULL,
                      1,
                      &status_task_handle);
}