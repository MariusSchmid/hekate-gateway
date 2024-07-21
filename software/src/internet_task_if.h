#ifndef AB0FE73E_DB3E_4E6A_9797_3182CC25FD6A
#define AB0FE73E_DB3E_4E6A_9797_3182CC25FD6A
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

typedef bool(*set_timer_callback_t)(struct tm time);

bool internet_task_init(void);

bool internet_task_send_udp(uint8_t *message, uint32_t size);
bool internet_task_register_timer_callback(set_timer_callback_t callback);

#endif /* AB0FE73E_DB3E_4E6A_9797_3182CC25FD6A */
