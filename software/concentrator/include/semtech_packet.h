#ifndef C92D7B2D_4328_450F_B2CF_72A8CC3BB473
#define C92D7B2D_4328_450F_B2CF_72A8CC3BB473

#include <stdint.h>
#include <stdbool.h>
#include "concentrator_types.h"

bool semtech_packet_init(gateway_config_t gateway_config);

bool semtech_packet_create_rxpk(char *message, uint32_t max_size, uint32_t *msg_size, lora_rx_packet_t *lora_rx_packet);
bool semtech_packet_create_stat(char *message, uint32_t max_size, uint32_t *msg_size);

bool semtech_packet_encode_rxpk(char *message, uint32_t msg_size, char *encoded_message, uint32_t enocded_message_size, uint32_t encoded_size);
bool semtech_packet_encode_stat(char *message, uint32_t msg_size, char *encoded_message, uint32_t enocded_message_size, uint32_t encoded_size);

#endif /* C92D7B2D_4328_450F_B2CF_72A8CC3BB473 */
