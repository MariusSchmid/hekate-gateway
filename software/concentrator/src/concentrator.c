#include "concentrator.h"
#include "lora_config.h"
#include "spi.h"
#include "loragw_hal.h"

static received_msg_cb_t this_recv_cb;

bool concentrator_init(concentrator_spi_if_t spi_if)
{

    if(!spi_init(spi_if)){
        return false;
    }
    if(!lora_config_init()){
        return false;
    }
    return true;
}

bool concentrator_start(received_msg_cb_t recv_cb)
{
    this_recv_cb = recv_cb;
    int x = lgw_start();
    return x == 0;
}
