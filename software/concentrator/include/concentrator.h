#ifndef D8F60895_1C50_44D4_8A26_F53BEC7641D9
#define D8F60895_1C50_44D4_8A26_F53BEC7641D9

#include "concentrator_types.h"
bool concentrator_init(concentrator_spi_if_t spi_if);

typedef bool (*received_msg_cb_t)();

bool concentrator_start(received_msg_cb_t recv_cb);

#endif /* D8F60895_1C50_44D4_8A26_F53BEC7641D9 */
