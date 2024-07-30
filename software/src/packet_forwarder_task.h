#ifndef EA384AC8_1D4E_47E7_86EE_9348607B7A78
#define EA384AC8_1D4E_47E7_86EE_9348607B7A78

#include "concentrator_types.h"

#define UDP_PORT 1700
#define BEACON_MSG_LEN_MAX 1024
#define LORA_LNS_IP "52.212.223.226"
#define STATUS_INTERVAL_MS 1000*30
#define TIME_INTERVAL_S 60*1

/**
 * @brief Send packket upstream to the LNS
 * @param packet Packet will be copied.
 */
void packet_forwarder_task_send_upstream(lora_rx_packet_t *packet);

void packet_forwarder_task_init(void);

void packet_forwarder_print_task_stats(void);

#endif /* EA384AC8_1D4E_47E7_86EE_9348607B7A78 */
