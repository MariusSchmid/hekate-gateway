#ifndef D8F60895_1C50_44D4_8A26_F53BEC7641D9
#define D8F60895_1C50_44D4_8A26_F53BEC7641D9

#include "concentrator_types.h"
bool concentrator_init(concentrator_spi_if_t spi_if);

typedef bool (*received_msg_cb_t)(lora_rx_packet_t *rx_packet);

bool concentrator_start();

bool concentrator_receive(received_msg_cb_t recv_cb,uint32_t *packets_received);

#endif /* D8F60895_1C50_44D4_8A26_F53BEC7641D9 */
