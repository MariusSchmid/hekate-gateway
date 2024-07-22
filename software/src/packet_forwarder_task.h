#ifndef EA384AC8_1D4E_47E7_86EE_9348607B7A78
#define EA384AC8_1D4E_47E7_86EE_9348607B7A78

#include "concentrator_types.h"

#define UDP_PORT 1700
#define BEACON_MSG_LEN_MAX 1024
// #define LORA_LNS_IP "192.168.0.113"
#define LORA_LNS_IP "52.212.223.226"
#define STATUS_INTERVAL_MS 10000
#define TIME_INTERVAL_S 60


/**
 * @brief Send packket upstream to the LNS
 * @param packet Packet will be copied.
 */
void packet_forwarder_task_send_upstream(lora_rx_packet_t *packet);

void packet_forwarder_task_init(void);


#endif /* EA384AC8_1D4E_47E7_86EE_9348607B7A78 */
