#include "semtech_packet.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include "loragw_hal.h"
#include "base64.h"

static gateway_config_t this_gateway_config;
static gateway_stats_t this_gateway_stats;

#define PROTOCOL_VERSION 2
#define PUSH_DATA 0
#define HEADER_SIZE 12

static void set_packet_header(char *msg, uint32_t max_size, uint32_t *msg_size)
{
    ENSURE(msg_size);
    ENSURE(msg);

    uint32_t net_mac_h = this_gateway_config.mac_address >> 32;
    uint32_t net_mac_l = (uint32_t)this_gateway_config.mac_address;
    memset(msg, 0, max_size);
    msg[0] = PROTOCOL_VERSION;
    msg[1] = rand() % 127; // random token
    msg[2] = rand() % 127; // random token
    msg[3] = PUSH_DATA;
    msg[4] = net_mac_h >> 24;
    msg[5] = net_mac_h >> 16;
    msg[6] = net_mac_h >> 8;
    msg[7] = net_mac_h;
    msg[8] = net_mac_l >> 24;
    msg[9] = net_mac_l >> 16;
    msg[10] = net_mac_l >> 8;
    msg[11] = net_mac_l;
    // *msg_size = snprintf(msg + HEADER_SIZE, max_size, "{\"stat\": {\"time\": \"2024-07-16 00:00:00 UTC\", \"rxnb\": 1, \"rxok\": 1, \"rxfw\": 1, \"ackr\": 100.0, \"dwnb\": 0, \"txnb\": 0}}");
    *msg_size += HEADER_SIZE;
}

static void create_stat_message(char *msg, uint32_t max_size, uint32_t *msg_size)
{
    ENSURE(msg);
    ENSURE(msg_size);

    uint32_t stats_msg_size = snprintf(msg, max_size, "{\"stat\": {\"time\": \"2024-07-16 00:00:00 UTC\", \"rxnb\": 1, \"rxok\": 1, \"rxfw\": 1, \"ackr\": 100.0, \"dwnb\": 0, \"txnb\": 0}}");
    *msg_size += stats_msg_size;
}

#define RXPK_ENTRY_VAL(x, y)                                          \
    sprintf_size = snprintf(msg + msg_cnt, max_size - msg_cnt, x, y); \
    msg_cnt += sprintf_size
#define RXPK_ENTRY(x)                                              \
    sprintf_size = snprintf(msg + msg_cnt, max_size - msg_cnt, x); \
    msg_cnt += sprintf_size

static void create_rxpk_message(char *msg, uint32_t max_size, uint32_t *msg_size, lora_rx_packet_t *lora_rx_packet)
{
    ENSURE(msg);
    ENSURE(msg_size);
    uint32_t msg_cnt = 0;
    uint32_t sprintf_size = 0;
    // uint32_t stats_msg_size = \
    // snprintf(msg, max_size, "{\"rxpk\":[\
    // {\"tmst\":3029183950,\ 
    // \"time\":\"2024-07-19 06:24:18.960168Z\",\
    // \"chan\":0,\
    // \"rfch\":1,\
    // \"freq\":868.100000,\
    // \"stat\":1,\
    // \"modu\":\"LORA\",\
    // \"datr\":\"SF10BW125\",\
    // \"codr\":\"4/5\",\
    // \"lsnr\":11.5,\
    // \"rssi\":-52,\
    // \"size\":29,\
    // \"data\":\"QM/t4QEAAQABuws7394jsn9mmjw2X2l/U4oUscI=\"\
    // }]}");

    RXPK_ENTRY("{\"rxpk\":[");
    RXPK_ENTRY_VAL("\"tmst\":%" PRIu32 ",", lora_rx_packet->count_us);
    RXPK_ENTRY_VAL("\"ftime\":%" PRIu32 ",", lora_rx_packet->ftime);
    RXPK_ENTRY_VAL("\"chan\":%" PRIu8 ",", lora_rx_packet->if_chain);
    RXPK_ENTRY_VAL("\"rfch\":%" PRIu8 ",", lora_rx_packet->rf_chain);
    RXPK_ENTRY_VAL("\"freq\":%.6lf,", ((double)lora_rx_packet->freq_hz / 1e6));
    RXPK_ENTRY_VAL("\"mid\":%" PRIu8 ",", lora_rx_packet->modem_id);

    switch (lora_rx_packet->status)
    {
    case STAT_CRC_OK:
        RXPK_ENTRY_VAL("\"stat\":%" PRIu8 ",", 1);
        break;
    case STAT_CRC_BAD:
        RXPK_ENTRY_VAL("\"stat\":%" PRId8 ",", -1);
        break;
    case STAT_NO_CRC:
        RXPK_ENTRY_VAL("\"stat\":%" PRIu8 ",", 0);
        break;
    default:
        RXPK_ENTRY_VAL("\"stat\":%s,", "?");
        break;
    }

    if (lora_rx_packet->modulation == MOD_LORA)
    {

        RXPK_ENTRY_VAL("\"modu\":%s,", "\"LORA\"");

        switch (lora_rx_packet->datarate)
        {
        case DR_LORA_SF5:
            RXPK_ENTRY_VAL("\"datr\":%s", "\"SF5");
            break;
        case DR_LORA_SF6:
            RXPK_ENTRY_VAL("\"datr\":%s", "\"SF6");
            break;
        case DR_LORA_SF7:
            RXPK_ENTRY_VAL("\"datr\":%s", "\"SF7");
            break;
        case DR_LORA_SF8:
            RXPK_ENTRY_VAL("\"datr\":%s", "\"SF8");
            break;
        case DR_LORA_SF9:
            RXPK_ENTRY_VAL("\"datr\":%s", "\"SF9");
            break;
        case DR_LORA_SF10:
            RXPK_ENTRY_VAL("\"datr\":%s", "\"SF10");
            break;
        case DR_LORA_SF11:
            RXPK_ENTRY_VAL("\"datr\":%s", "\"SF11");
            break;
        case DR_LORA_SF12:
            RXPK_ENTRY_VAL("\"datr\":%s", "\"SF12");
            break;
        default:
            RXPK_ENTRY_VAL("\"datr\":%s", "?");
            break;
        }
        switch (lora_rx_packet->bandwidth)
        {
        case BW_125KHZ:
            RXPK_ENTRY("BW125\"");
            break;
        case BW_250KHZ:
            RXPK_ENTRY("BW250\"");
            break;
        case BW_500KHZ:
            RXPK_ENTRY("BW500\"");
            break;
        default:
            RXPK_ENTRY("BW?\"");
            break;
        }

        switch (lora_rx_packet->coderate)
        {
        case CR_LORA_4_5:
            RXPK_ENTRY_VAL("\"codr\":%s,", "\"4/5\"");
            break;
        case CR_LORA_4_6:
            RXPK_ENTRY_VAL("\"codr\":%s,", "\"4/6\"");
            break;
        case CR_LORA_4_7:
            RXPK_ENTRY_VAL("\"codr\":%s,", "\"4/7\"");
            break;
        case CR_LORA_4_8:
            RXPK_ENTRY_VAL("\"codr\":%s,", "\"4/8\"");
            break;
        case 0: /* treat the CR0 case (mostly false sync) */
            RXPK_ENTRY_VAL("\"codr\":%s,", "\"OFF\"");
            break;
        default:
            RXPK_ENTRY_VAL("\"codr\":%s,", "\"?\"");
            break;
        }

        RXPK_ENTRY_VAL("\"rssis\":%.0f,", roundf(lora_rx_packet->rssis));
        RXPK_ENTRY_VAL("\"lsnr\":%.1f,", roundf(lora_rx_packet->snr));
        RXPK_ENTRY_VAL("\"foff\":" PRIu32 ",", roundf(lora_rx_packet->freq_offset));
    }

    RXPK_ENTRY_VAL("\"rssi\":%.0f,", roundf(lora_rx_packet->rssic));
    RXPK_ENTRY_VAL("\"size\":%" PRIu16 ",", lora_rx_packet->size);

    RXPK_ENTRY("\"data\":\"");


    int32_t nr_encoded = bin_to_b64(lora_rx_packet->payload,lora_rx_packet->size,msg + msg_cnt,max_size - msg_cnt);
    msg_cnt += nr_encoded;

    RXPK_ENTRY("\"");

    RXPK_ENTRY("}]}");

    *msg_size += msg_cnt;
}

bool semtech_packet_init(gateway_config_t gateway_config)
{
    this_gateway_config = gateway_config;
    return true;
}

bool semtech_packet_create_rxpk(char *message, uint32_t max_size, uint32_t *msg_size, lora_rx_packet_t *lora_rx_packet)
{
    ENSURE_RET(message, false);
    ENSURE_RET(msg_size, false);
    set_packet_header(message, max_size, msg_size);
    create_rxpk_message(message + (*msg_size), max_size - (*msg_size), msg_size, lora_rx_packet);
    return true;
}

bool semtech_packet_create_stat(char *message, uint32_t max_size, uint32_t *msg_size)
{
    ENSURE_RET(message, false);
    ENSURE_RET(msg_size, false);

    set_packet_header(message, max_size, msg_size);
    create_stat_message(message + (*msg_size), max_size - (*msg_size), msg_size);

    return true;
}
