#include "internet_task_if.h"
#include "hekate_utils.h"
#include "time_ntp.h"

#include "FreeRTOS.h"
#include "task.h"
#include "log.h"

#include "pico/cyw43_arch.h"
static volatile bool time_set = false;

static struct udp_pcb *pcb_rxpt;
static struct udp_pcb *pcb_status;

static set_time_callback_t this_time_callback;

bool internet_task_send_udp(uint8_t *message, uint32_t size, const char *dst_ip, uint16_t port)
{
    ip_addr_t addr;
    if (!ipaddr_aton(dst_ip, &addr))
    {
        log_error("can not convert %s into ip address", dst_ip);
        return false;
    }
    else
    {
        log_info("LORA_LNS_IP: %s", dst_ip);
    }
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_RAM);
    memcpy(p->payload, message, size);
    err_t er = udp_sendto(pcb_status, p, &addr, port);
    pbuf_free(p);
    if (er != ERR_OK)
    {
        log_error("Failed to send send_status_packet packet! error=%d", er);
    }
}

static void time_set_cb(struct tm time)
{
    ENSURE(this_time_callback);
    this_time_callback(time);
}

static void wifi_task(void *pvParameters)
{
    log_info("Initialize  cyw43_arch");
    if (cyw43_arch_init())
    {
        log_error("fail: cyw43_arch_init");
    }
    cyw43_arch_enable_sta_mode();

    log_info("Connecting to Wi-Fi %s", WIFI_SSID);
    log_info("Connecting to Wi-Fi Password %s", WIFI_PASSWORD);
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
    while (1)
    {
        cyw43_arch_poll();
        vTaskDelay(pdTICKS_TO_MS(1));
    }
    cyw43_arch_deinit();
}

bool internet_task_register_time_callback(set_time_callback_t time_callback)
{
    ENSURE_RET(time_callback, false);
    this_time_callback = time_callback;
}

bool internet_task_trigger_get_time(void)
{
    time_npt_set_time(time_set_cb);
    return true;
}

bool internet_task_init(void)
{
    BaseType_t ret = xTaskCreate(wifi_task,
                                 "WIFI_TASK",
                                 1024 * 8,
                                 NULL,
                                 1,
                                 NULL);
    if (ret != pdPASS)
    {
        log_error("xTaskCreate failed: sending_task");
        return false;
    }
}