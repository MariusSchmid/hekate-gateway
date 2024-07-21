
#include "concentrator_types.h"
#include "semtech_packet.h"
#include <stdio.h>
#include <string.h>

char stat_packet[265];
char rx_packet[1024];
gateway_config_t gw_config;
lora_rx_packet_t lora_rx_packet;

uint8_t example_data[29] = {1, 2, 3, 4};

int main(int argc, char const *argv[])
{
    gw_config.mac_address = 0xA84041FDFEEFBE63;
    semtech_packet_init(gw_config);
    uint32_t packet_size = 0;
    semtech_packet_create_stat(stat_packet, sizeof(stat_packet), &packet_size);

    for (size_t i = 0; i < packet_size; i++)
    {
        printf("%c", stat_packet[i]);
    }
    printf("\n");
    printf("\n");
    printf("\n");
    lora_rx_packet.bandwidth = 0;
    lora_rx_packet.modulation = 0x10; // LORA
    lora_rx_packet.datarate = 7;
    lora_rx_packet.bandwidth = 0x5; // 250
    lora_rx_packet.size = sizeof(example_data);
    memcpy(&lora_rx_packet.payload, example_data, sizeof(example_data));
    // lora_rx_packet.payload = example_data;

    semtech_packet_create_rxpk(rx_packet, sizeof(rx_packet), &packet_size, &lora_rx_packet);

    printf("msg (%d):", packet_size);
    for (size_t i = 0; i < packet_size; i++)
    {
        printf("%c", rx_packet[i]);
    }

    

    return 0;
}
