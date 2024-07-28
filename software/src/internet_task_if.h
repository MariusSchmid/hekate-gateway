#ifndef AB0FE73E_DB3E_4E6A_9797_3182CC25FD6A
#define AB0FE73E_DB3E_4E6A_9797_3182CC25FD6A
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

typedef bool (*set_time_callback_t)(struct tm time);

bool internet_task_init(void);
bool internet_task_connect(const char *dst_ip, uint16_t port);
bool internet_task_disconnect(void);
bool internet_task_send_udp(uint8_t *message, uint32_t size);
bool internet_task_get_time(struct tm *time);

void internet_task_print_task_stats(void);

#endif /* AB0FE73E_DB3E_4E6A_9797_3182CC25FD6A */
