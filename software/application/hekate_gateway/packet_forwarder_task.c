#include "packet_forwarder_task.h"
#include "semtech_packet.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define HEADER_SIZE 12

static SemaphoreHandle_t send_status_sem;
static uint64_t mac = 0xA84041FDFEEFBE63;

static gateway_config_t gateway_config;
static gateway_stats_t gateway_stats;

static char stat_packet[265];

typedef struct NTP_T_
{
    ip_addr_t ntp_server_address;
    bool dns_request_sent;
    struct udp_pcb *ntp_pcb;
    absolute_time_t ntp_test_time;
    alarm_id_t ntp_resend_alarm;
} NTP_T;

#define NTP_SERVER "pool.ntp.org"
#define NTP_MSG_LEN 48
#define NTP_PORT 123
#define NTP_DELTA 2208988800 // seconds between 1 Jan 1900 and 1 Jan 1970
#define NTP_TEST_TIME (30 * 1000)
#define NTP_RESEND_TIME (10 * 1000)

static bool time_set = false;

// Called with results of operation
static void ntp_result(NTP_T *state, int status, time_t *result)
{
    if (status == 0 && result)
    {
        struct tm *utc = gmtime(result);
        gateway_stats.time = *result;
        printf("got ntp response: %02d/%02d/%04d %02d:%02d:%02d\n", utc->tm_mday, utc->tm_mon + 1, utc->tm_year + 1900,
               utc->tm_hour, utc->tm_min, utc->tm_sec);
        time_set = true;
    }

    if (state->ntp_resend_alarm > 0)
    {
        cancel_alarm(state->ntp_resend_alarm);
        state->ntp_resend_alarm = 0;
    }
    state->ntp_test_time = make_timeout_time_ms(NTP_TEST_TIME);
    state->dns_request_sent = false;
}

// NTP data received
static void ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    NTP_T *state = (NTP_T *)arg;
    uint8_t mode = pbuf_get_at(p, 0) & 0x7;
    uint8_t stratum = pbuf_get_at(p, 1);

    // Check the result
    if (ip_addr_cmp(addr, &state->ntp_server_address) && port == NTP_PORT && p->tot_len == NTP_MSG_LEN &&
        mode == 0x4 && stratum != 0)
    {
        uint8_t seconds_buf[4] = {0};
        pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
        uint32_t seconds_since_1900 = seconds_buf[0] << 24 | seconds_buf[1] << 16 | seconds_buf[2] << 8 | seconds_buf[3];
        uint32_t seconds_since_1970 = seconds_since_1900 - NTP_DELTA;
        time_t epoch = seconds_since_1970;
        ntp_result(state, 0, &epoch);
    }
    else
    {
        printf("invalid ntp response\n");
        ntp_result(state, -1, NULL);
    }
    pbuf_free(p);
}

static int64_t ntp_failed_handler(alarm_id_t id, void *user_data);

// Perform initialisation
static NTP_T *ntp_init(void)
{
    NTP_T *state = (NTP_T *)calloc(1, sizeof(NTP_T));
    if (!state)
    {
        printf("failed to allocate state\n");
        return NULL;
    }
    state->ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (!state->ntp_pcb)
    {
        printf("failed to create pcb\n");
        free(state);
        return NULL;
    }
    udp_recv(state->ntp_pcb, ntp_recv, state);
    return state;
}

// Make an NTP request
static void ntp_request(NTP_T *state)
{
    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
    uint8_t *req = (uint8_t *)p->payload;
    memset(req, 0, NTP_MSG_LEN);
    req[0] = 0x1b;
    udp_sendto(state->ntp_pcb, p, &state->ntp_server_address, NTP_PORT);
    pbuf_free(p);
    cyw43_arch_lwip_end();
}

static void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg)
{
    NTP_T *state = (NTP_T *)arg;
    if (ipaddr)
    {
        state->ntp_server_address = *ipaddr;
        printf("ntp address %s\n", ipaddr_ntoa(ipaddr));
        ntp_request(state);
    }
    else
    {
        printf("ntp dns request failed\n");
        ntp_result(state, -1, NULL);
    }
}

static void get_ntp_time()
{
    NTP_T *state = ntp_init();
    if (!state)
        return;
    if (absolute_time_diff_us(get_absolute_time(), state->ntp_test_time) < 0 && !state->dns_request_sent)
    {
        // Set alarm in case udp requests are lost
        state->ntp_resend_alarm = add_alarm_in_ms(NTP_RESEND_TIME, ntp_failed_handler, state, true);

        // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
        // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
        // these calls are a no-op and can be omitted, but it is a good practice to use them in
        // case you switch the cyw43_arch type later.
        cyw43_arch_lwip_begin();
        int err = dns_gethostbyname(NTP_SERVER, &state->ntp_server_address, ntp_dns_found, state);
        cyw43_arch_lwip_end();

        state->dns_request_sent = true;
        if (err == ERR_OK)
        {
            ntp_request(state); // Cached result
        }
        else if (err != ERR_INPROGRESS)
        { // ERR_INPROGRESS means expect a callback
            printf("dns request failed\n");
            ntp_result(state, -1, NULL);
        }
    }
}

static int64_t ntp_failed_handler(alarm_id_t id, void *user_data)
{
    NTP_T *state = (NTP_T *)user_data;
    printf("ntp request failed\n");
    ntp_result(state, -1, NULL);
    return 0;
}

struct udp_pcb *pcb;
static ip_addr_t addr;
static void send_status_packet()
{
    uint32_t packet_size = 0;

    semtech_packet_create_stat(stat_packet, sizeof(stat_packet), &packet_size, &gateway_stats);
    char data_to_send[BEACON_MSG_LEN_MAX];
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, packet_size + 1, PBUF_RAM);
    memcpy(p->payload, stat_packet, packet_size);
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
    gateway_config.mac_address = mac;
    semtech_packet_init(gateway_config);

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

    // get time
    printf("Get NTP Time.\n");
    get_ntp_time();

    static int wait_ms = 0;
    pcb = udp_new();
    ipaddr_aton(BEACON_TARGET, &addr);
    while (1)
    {
        if (uxSemaphoreGetCount(send_status_sem) > 0)
        {
            xSemaphoreTake(send_status_sem, 0);
            if (time_set)
            {
                send_status_packet();
            }
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
        xSemaphoreGive(send_status_sem);
        vTaskDelay(pdTICKS_TO_MS(BEACON_INTERVAL_MS));
    }
}

void packet_forwarder_task_init(void)
{

    send_status_sem = xSemaphoreCreateBinary();

    TaskHandle_t sending_task_handle;
    TaskHandle_t status_task_handle;

    BaseType_t ret = xTaskCreate(sending_task,
                                 "PFW_TASK",
                                 1024 * 32,
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