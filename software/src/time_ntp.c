#include "time_ntp.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "log.h"

#define NTP_SERVER "pool.ntp.org"
#define NTP_MSG_LEN 48
#define NTP_PORT 123
#define NTP_DELTA 2208988800 // seconds between 1 Jan 1900 and 1 Jan 1970
#define NTP_TEST_TIME (30 * 1000)
#define NTP_RESEND_TIME (10 * 1000)

typedef struct NTP_T_
{
    ip_addr_t ntp_server_address;
    bool dns_request_sent;
    struct udp_pcb *ntp_pcb;
    absolute_time_t ntp_test_time;
    alarm_id_t ntp_resend_alarm;
} NTP_T;

static time_set_cb_t this_time_set_cb;
struct tm utc_time;
// Called with results of operation
static void ntp_result(NTP_T *state, int status, time_t *result)
{
    if (status == 0 && result)
    {
        struct timeval now;
        int rc;
        struct tm *utc = gmtime(result);
        memcpy(&utc_time, utc, sizeof(struct tm));
        this_time_set_cb(utc_time);
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
        log_error("invalid ntp response");
        ntp_result(state, -1, NULL);
    }
    pbuf_free(p);
}

static int64_t ntp_failed_handler(alarm_id_t id, void *user_data);

static NTP_T *ntp_init(void)
{
    NTP_T *state = (NTP_T *)calloc(1, sizeof(NTP_T));
    if (!state)
    {
        log_error("failed to allocate state");
        return NULL;
    }
    state->ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (!state->ntp_pcb)
    {
        log_error("failed to create pcb");
        free(state);
        return NULL;
    }
    udp_recv(state->ntp_pcb, ntp_recv, state);
    return state;
}

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
        log_info("ntp server address: %s", ipaddr_ntoa(ipaddr));
        ntp_request(state);
    }
    else
    {
        log_error("ntp dns request failed");
        ntp_result(state, -1, NULL);
    }
}

static void get_ntp_time()
{
    static NTP_T *ntp_state = NULL;
    if (!ntp_state)
    {
        ntp_state = ntp_init();
    }
    if (!ntp_state)
        return;
    if (absolute_time_diff_us(get_absolute_time(), ntp_state->ntp_test_time) < 0 && !ntp_state->dns_request_sent)
    {
        // Set alarm in case udp requests are lost
        ntp_state->ntp_resend_alarm = add_alarm_in_ms(NTP_RESEND_TIME, ntp_failed_handler, ntp_state, true);
        cyw43_arch_lwip_begin();
        int err = dns_gethostbyname(NTP_SERVER, &ntp_state->ntp_server_address, ntp_dns_found, ntp_state);
        cyw43_arch_lwip_end();

        ntp_state->dns_request_sent = true;
        if (err == ERR_OK)
        {
            ntp_request(ntp_state); // Cached result
        }
        else if (err != ERR_INPROGRESS)
        { // ERR_INPROGRESS means expect a callback
            log_error("dns request failed");
            ntp_result(ntp_state, -1, NULL);
        }
    }
}

static int64_t ntp_failed_handler(alarm_id_t id, void *user_data)
{
    NTP_T *state = (NTP_T *)user_data;
    log_error("ntp request failed");
    ntp_result(state, -1, NULL);
    return 0;
}

void time_npt_set_time(time_set_cb_t time_set_cb)
{
    this_time_set_cb = time_set_cb;
    get_ntp_time();
}