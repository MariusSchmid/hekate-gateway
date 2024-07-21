#ifndef A846C95B_6805_4C11_A45A_BCC1915C20E8
#define A846C95B_6805_4C11_A45A_BCC1915C20E8
#include <time.h>

typedef void(*time_set_cb_t)(struct tm time);
void time_npt_set_time(time_set_cb_t time_set_cb);

#endif /* A846C95B_6805_4C11_A45A_BCC1915C20E8 */
