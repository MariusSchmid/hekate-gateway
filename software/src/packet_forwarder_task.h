#ifndef EA384AC8_1D4E_47E7_86EE_9348607B7A78
#define EA384AC8_1D4E_47E7_86EE_9348607B7A78

#include "concentrator_types.h"


/**
 * @brief Send packket upstream to the LNS
 * @param packet Packet will be copied.
 */
void packet_forwarder_task_send_upstream(lora_rx_packet_t *packet);

void packet_forwarder_task_init(void);

void packet_forwarder_print_task_stats(void);

#endif /* EA384AC8_1D4E_47E7_86EE_9348607B7A78 */
