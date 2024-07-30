#include "sim7020.h"
#include "sim7020_hal.h"
#include "log.h"
#include "hekate_utils.h"

#include <string.h>
#include <stdlib.h>

static uint8_t current_socket_id = 0;

static bool set_apn(void)
{
    sim7020_hal_send_cmd_check_recv("AT+CFUN=0\r\n", "READY", 10000);
    sim7020_hal_send_cmd_check_recv("AT*MCGDEFCONT=\"IP\",\"iot.1nce.net\"\r\n", "OK", 10000); // set apn settings
    sim7020_hal_send_cmd_check_recv("AT+CFUN=1\r\n", "READY", 10000);
    return true;
}

bool sim7020_initialize_sim_module()
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

bool sim7020_connect(const char *dst_ip, uint16_t port)
{
    ENSURE_RET(dst_ip, false);
    char connection_string[256] = {0};
    sprintf(connection_string, "AT+CIPSTART=\"UDP\",\"%s\",\"%d\"\r\n", dst_ip, port);

/* create UDP Socket*/
#if 0 // not needed when using  CIPSTART
    if (!sim7020_hal_send_cmd_check_recv("AT+CSOC=1,2,1\r\n", "+CSOC:", 10000))
    {
        sim7020_hal_send_cmd_check_recv("AT+CIPCLOSE=0\r\n", "OK", 5000);
        return false;
    }
#endif

    if (!sim7020_hal_send_cmd_check_recv(connection_string, "CONNECT OK", 60000))
    {
        return false;
    }
    if (!sim7020_hal_send_cmd_check_recv("AT+CIPCHAN\r\n", "CONNECT", 10000)) // enter transparent mode
    {
        sim7020_disconnect();
        return false;
    }

    return true;
}

// not possible in transparent mode
bool sim7020_is_connected(void)
{
    // AT+CIPSTATUS

    return false;
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

    time->tm_year = strtol(year_str, NULL, 10) + 100;
    time->tm_mon = strtol(month_str, NULL, 10) - 1;
    time->tm_mday = strtol(day_str, NULL, 10);
    time->tm_hour = strtol(hour_str, NULL, 10);
    time->tm_min = strtol(minute_str, NULL, 10);
    time->tm_sec = strtol(second_str, NULL, 10);

    return true;
}

bool sim7020_get_time_ntp(struct tm *time)
{
    bool result = false;
    char response[128];
#if CHINESE_NTP == 1
    sim7020_hal_send_cmd_check_recv("AT+CSNTPSTART=\"jp.ntp.org.cn\",\"+32\"\r\n", "OK", 20000);
#else
    if (!sim7020_hal_send_cmd_check_recv("AT+CSNTPSTART=\"pool.ntp.org\",\"+48\"\r\n", "+CSNTP:", 20000))
    {
        return false;
    }
#endif

    if (!sim7020_hal_send_cmd_get_recv("AT+CCLK?\r\n", "+CCLK:", 5000, response, sizeof(response)))
    {
        log_error("can not get ntp time");
        return false;
    }
    else
    {
        if (!parse_ntp_string(response, time))
        {
            log_error("parse_ntp_string failed: %s", response);
        }
        else
        {
            result = true;
        }
    }

    sim7020_hal_send_cmd_check_recv("AT+CSNTPSTOP\r\n", "OK", 5000);
    return result;
}

bool sim7020_disconnect(void)
{
    char exit_transparent_mode_sequence[] = "+++";
    sim7020_hal_sleep_ms(1500);
    sim7020_hal_send(exit_transparent_mode_sequence, 3);
    sim7020_hal_sleep_ms(1500);
    sim7020_hal_send_cmd_check_recv("AT+CIPCLOSE=0\r\n", "OK", 5000);
}

bool sim7020_send_udp(uint8_t *message, uint32_t size)
{
    return sim7020_hal_send(message, size);
}

bool sim7020_is_attached_gprs(void)
{
    char response[128];
    if (!sim7020_hal_send_cmd_get_recv("AT+CGATT?\r\n", "+CGATT:", 5000, response, sizeof(response)))
    {
        log_error("can not get gprs state");
        return false;
    }
    if (strstr(response, "+CGATT: 1") != NULL)
    {
        return true;
    }
    return false;
}

bool sim7020_is_PDP_active(void)
{
    char response[128];
    if (!sim7020_hal_send_cmd_get_recv("AT+CGACT?\r\n", "+CGACT:", 5000, response, sizeof(response)))
    {
        log_error("can not get PDP active or not");
        return false;
    }
    if (strstr(response, "+CGACT: 1,1") != NULL)
    {
        return true;
    }
    return false;
}

#include <inttypes.h>
#include <string.h>

static bool get_info(char *cmd, char *exp_resp, char *dst, uint16_t dst_size)
{
    if (!sim7020_hal_send_cmd_get_recv(cmd, exp_resp, 1000 * 20, dst, dst_size))
    {
        log_error("%s failed", cmd);
        return false;
    }
    hekate_utils_strremove(dst, exp_resp);
    hekate_utils_remove_character(dst, '\"');
    return true;
}

#define RXPK_ENTRY_VAL(x, y)                                                 \
    sprintf_size = snprintf(dst_buffer + msg_cnt, dst_size - msg_cnt, x, y); \
    msg_cnt += sprintf_size
#define RXPK_ENTRY(x)                                                     \
    sprintf_size = snprintf(dst_buffer + msg_cnt, dst_size - msg_cnt, x); \
    msg_cnt += sprintf_size

/*
    sim7020 status:
    Current Operator Selection  | AT+COPS?
    Signal Quality              | AT+CSQ
    Network registration        | AT+CREG?
    PLMN List                   | AT+CPLS?
    PDP Address                 | AT+IPCONFIG
    EPS Network Registration    | AT+CEREG?
    Mobile Operation Band       | AT+CBAND?
    Report Network State        | AT+CENG

    Home Network Information    | AT+CHOMENW?

    Preferred Operator List     | AT+CPOL?
    Available Operator List     | AT+COPS=?
    SIM card ID                 | AT+CIMI
*/
char response[512];
bool sim7020_get_information_json(char *dst_buffer, uint16_t dst_size, uint16_t *size)
{
    uint32_t sprintf_size = 0;
    uint32_t msg_cnt = 0;
    RXPK_ENTRY("{\"sim_status\":{");

    // current operator selected
    if (get_info("AT+COPS?\r\n", "+COPS: ", response, sizeof(response)))
    {
        RXPK_ENTRY_VAL("\"COPS\":\"%s\",", response);
    }
#if 0
    if (get_info("AT+COPS=?\r\n", "+COPS: ", response, sizeof(response)))
    {
        RXPK_ENTRY_VAL("\"ALL_COPS\":\"%s\",", response);
    }
#endif

    // Signal quality
    if (get_info("AT+CSQ\r\n", "+CSQ: ", response, sizeof(response)))
    {

        RXPK_ENTRY_VAL("\"CSQ\":\"%s\",", response);
    }

    // Preferred Operator List
#if 0 // not supported
    get_info("AT+CPOL?\r\n", "+CPOL: ", response, sizeof(response));
    RXPK_ENTRY_VAL("\"CPOL\":\"%s\",", response);
#endif

    // Network registration
    if (get_info("AT+CREG?\r\n", "+CREG: ", response, sizeof(response)))
    {
        RXPK_ENTRY_VAL("\"CREG\":\"%s\",", response);
    }

    // PLMN List
    if (get_info("AT+CPLS?\r\n", "+CPLS: ", response, sizeof(response)))
    {
        RXPK_ENTRY_VAL("\"CPLS\":\"%s\",", response);
    }

    // PDP Address
    if (get_info("AT+IPCONFIG\r\n", "+IPCONFIG: ", response, sizeof(response)))
    {
        RXPK_ENTRY_VAL("\"IPCONFIG\":\"%s\",", response);
    }

    // EPS Network Registration
    if (get_info("AT+CEREG?\r\n", "+CEREG: ", response, sizeof(response)))
    {
        RXPK_ENTRY_VAL("\"CEREG\":\"%s\",", response);
    }

    // Mobile Operation Band
    if (get_info("AT+CBAND?\r\n", "+CBAND: ", response, sizeof(response)))
    {
        RXPK_ENTRY_VAL("\"CBAND\":\"%s\",", response);
    }

    // Report Network State
    if (get_info("AT+CENG?\r\n", "+CENG: ", response, sizeof(response)))
    {
        RXPK_ENTRY_VAL("\"CENG\":\"%s\",", response);
    }

    // Home Network Information
    if (get_info("AT+CHOMENW?\r\n", "+CHOMENW: ", response, sizeof(response)))
    {
        RXPK_ENTRY_VAL("\"CHOMENW\":\"%s\",", response);
    }
    time_t current_time = time((time_t *)0);

    RXPK_ENTRY_VAL("\"time\":%ld", current_time);
    RXPK_ENTRY("}");
    return true;
}
