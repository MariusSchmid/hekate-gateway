#include <string.h>
#include <stdio.h>

#include "lora_config.h"
#include "loragw_hal.h"

static lgw_radio_type_t radio_type = LGW_RADIO_TYPE_NONE;
uint8_t clocksource = 0; // Radio A
bool single_input_mode = false;
#define DEFAULT_FREQ_HZ 868500000U
uint32_t fa = DEFAULT_FREQ_HZ;
uint32_t fb = DEFAULT_FREQ_HZ;
struct lgw_conf_board_s boardconf;
struct lgw_conf_rxrf_s rfconf;
struct lgw_conf_rxif_s ifconf;
#define max_rx_pkt 16
lgw_com_type_t com_type = LGW_COM_SPI;
#define COM_PATH_DEFAULT "/dev/spidev0.0"
const char com_path_default[] = COM_PATH_DEFAULT;
const char *com_path = com_path_default;
float rssi_offset = 0.0;
bool full_duplex = false;
unsigned long nb_pkt_crc_ok = 0, nb_loop = 10, cnt_loop;
int nb_pkt;
uint8_t channel_mode = 0; /* LoRaWAN-like */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

bool lora_config_init(void)
{

    const int32_t channel_if_mode0[9] = {
        -400000,
        -200000,
        0,
        -400000,
        -200000,
        0,
        200000,
        400000,
        -200000 /* lora service */
    };

    const int32_t channel_if_mode1[9] = {
        -400000,
        -400000,
        -400000,
        -400000,
        -400000,
        -400000,
        -400000,
        -400000,
        -400000 /* lora service */
    };

    const uint8_t channel_rfchain_mode0[9] = {1, 1, 1, 0, 0, 0, 0, 0, 1};
    const uint8_t channel_rfchain_mode1[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    radio_type = LGW_RADIO_TYPE_SX1250;
    // radio_type = LGW_RADIO_TYPE_SX1250;

    /* Configure the gateway */
    memset(&boardconf, 0, sizeof boardconf);
    boardconf.lorawan_public = true;
    boardconf.clksrc = clocksource;
    boardconf.full_duplex = full_duplex;
    boardconf.com_type = com_type;
    strncpy(boardconf.com_path, com_path, sizeof boardconf.com_path);
    boardconf.com_path[sizeof boardconf.com_path - 1] = '\0'; /* ensure string termination */
    if (lgw_board_setconf(&boardconf) != LGW_HAL_SUCCESS)
    {
        printf("ERROR: failed to configure board\n");
        return false;
    }

    /* set configuration for RF chains */
    memset(&rfconf, 0, sizeof rfconf);
    rfconf.enable = true;
    rfconf.freq_hz = fa;
    rfconf.type = radio_type;
    rfconf.rssi_offset = rssi_offset;
    rfconf.tx_enable = false;
    rfconf.single_input_mode = single_input_mode;
    if (lgw_rxrf_setconf(0, &rfconf) != LGW_HAL_SUCCESS)
    {
        printf("ERROR: failed to configure rxrf 0\n");
        return false;
    }

    memset(&rfconf, 0, sizeof rfconf);
    rfconf.enable = true;
    rfconf.freq_hz = fb;
    rfconf.type = radio_type;
    rfconf.rssi_offset = rssi_offset;
    rfconf.tx_enable = false;
    rfconf.single_input_mode = single_input_mode;
    if (lgw_rxrf_setconf(1, &rfconf) != LGW_HAL_SUCCESS)
    {
        printf("ERROR: failed to configure rxrf 1\n");
        return false;
    }

    /* set configuration for LoRa multi-SF channels (bandwidth cannot be set) */
    memset(&ifconf, 0, sizeof(ifconf));
    int i;
    for (i = 0; i < 8; i++)
    {
        ifconf.enable = true;
        if (channel_mode == 0)
        {
            ifconf.rf_chain = channel_rfchain_mode0[i];
            ifconf.freq_hz = channel_if_mode0[i];
        }
        else if (channel_mode == 1)
        {
            ifconf.rf_chain = channel_rfchain_mode1[i];
            ifconf.freq_hz = channel_if_mode1[i];
        }
        else
        {
            printf("ERROR: channel mode not supported\n");
            return false;
        }
        ifconf.datarate = DR_LORA_SF7;
        if (lgw_rxif_setconf(i, &ifconf) != LGW_HAL_SUCCESS)
        {
            printf("ERROR: failed to configure rxif %d\n", i);
            return false;
        }
    }

    /* set configuration for LoRa Service channel */
    memset(&ifconf, 0, sizeof(ifconf));
    ifconf.rf_chain = channel_rfchain_mode0[i];
    ifconf.freq_hz = channel_if_mode0[i];
    ifconf.datarate = DR_LORA_SF7;
    ifconf.bandwidth = BW_250KHZ;
    if (lgw_rxif_setconf(8, &ifconf) != LGW_HAL_SUCCESS)
    {
        printf("ERROR: failed to configure rxif for LoRa service channel\n");
        return false;
    }

    return true;
}