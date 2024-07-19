#include "concentrator.h"
#include "lora_config.h"
#include "concentrator_spi.h"
#include "loragw_hal.h"
#define max_rx_pkt 16
static uint32_t nb_pkt = 0;
// static received_msg_cb_t this_recv_cb;

bool concentrator_init(concentrator_spi_if_t spi_if)
{

    if (!concentrator_spi_init(spi_if))
    {
        return false;
    }
    if (!lora_config_init())
    {
        return false;
    }
    return true;
}

struct lgw_pkt_rx_s rxpkt[max_rx_pkt];
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
bool concentrator_receive(received_msg_cb_t recv_cb, uint32_t *packets_received)
{
    nb_pkt = lgw_receive(ARRAY_SIZE(rxpkt), rxpkt);
    if (nb_pkt > 0)
    {
        for (int i = 0; i < nb_pkt; i++)
        {
            lora_rx_packet_t rx_packet;
            rx_packet.freq_hz = rxpkt[i].freq_hz;
            rx_packet.freq_offset = rxpkt[i].freq_offset;
            rx_packet.if_chain = rxpkt[i].if_chain;
            rx_packet.status = rxpkt[i].status;
            rx_packet.count_us = rxpkt[i].count_us;
            rx_packet.rf_chain = rxpkt[i].rf_chain;
            rx_packet.modem_id = rxpkt[i].modem_id;
            rx_packet.modulation = rxpkt[i].modulation;
            rx_packet.bandwidth = rxpkt[i].bandwidth;
            rx_packet.datarate = rxpkt[i].datarate;
            rx_packet.coderate = rxpkt[i].coderate;
            rx_packet.rssic = rxpkt[i].rssic;
            rx_packet.rssis = rxpkt[i].rssis;
            rx_packet.snr = rxpkt[i].snr;
            rx_packet.snr_min = rxpkt[i].snr_min;
            rx_packet.snr_max = rxpkt[i].snr_max;
            rx_packet.crc = rxpkt[i].crc;
            rx_packet.size = rxpkt[i].size;
            rx_packet.ftime_received = rxpkt[i].ftime_received;
            rx_packet.ftime = rxpkt[i].ftime;

            for (int j = 0; j < rxpkt[i].size; j++)
            {
                rx_packet.payload[j] = rxpkt[i].payload[j];
            }

            recv_cb(&rx_packet);
        }
    }
    return true;
}

bool concentrator_start(received_msg_cb_t recv_cb)
{
    int x = lgw_start();
    return x == 0;
}
