#ifndef AB0FE73E_DB3E_4E6A_9797_3182CC25FD6A
#define AB0FE73E_DB3E_4E6A_9797_3182CC25FD6A
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

typedef bool (*set_time_callback_t)(struct tm time);

bool internet_task_init(void);
bool internet_task_send_udp(uint8_t *message, uint32_t size, const char *dst_ip, uint16_t port);
bool internet_task_register_time_callback(set_time_callback_t time_callback);
bool internet_task_trigger_get_time(void);

#endif /* AB0FE73E_DB3E_4E6A_9797_3182CC25FD6A */
