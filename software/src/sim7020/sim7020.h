#ifndef FFFFAE05_6C49_4335_80BE_975B42CF1803
#define FFFFAE05_6C49_4335_80BE_975B42CF1803
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>

bool sim7020_initialize_sim_module(void);

bool sim7020_connect(const char *dst_ip, uint16_t port);
bool sim7020_disconnect(void);
bool sim7020_is_connected(void);

bool sim7020_get_time();

bool sim7020_get_time_ntp(struct tm *time);

bool sim7020_send_udp(uint8_t *message, uint32_t size);

bool sim7020_is_attached_gprs(void);
bool sim7020_is_PDP_active(void);

/*
    sim7020 status:
    SIM card ID                 | AT+CIMI
    Current Operator Selection  | AT+COPS?
    All Operator                | AT+COPS=?
    Preferred Operator List     | AT+CPOL?
    Network registration        | AT+CREG?
    Signal Quality              | AT+CSQ
    PLMN List                   | AT+CPLS?
    PDP Address                 | AT+IPCONFIG
    EPS Network Registration    | AT+CEREG?
    Mobile Operation Band       | AT+CBAND?
    Report Network State        | AT+CENG
    Home Network Information    | AT+CHOMENW?
*/
bool sim7020_get_information_json(char *dst_buffer, uint16_t dst_size, uint16_t *size);

#endif /* FFFFAE05_6C49_4335_80BE_975B42CF1803 */
