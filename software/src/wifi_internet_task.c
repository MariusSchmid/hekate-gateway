#include "internet_task_if.h"


bool packet_forwarder_task_init(void)
{
    #if 0
    init_gateway_config();

    send_status_sem = xSemaphoreCreateBinary();
    if (!send_status_sem)
    {
        log_error("xSemaphoreCreateBinary failed");
    }

    lora_rx_packet_queue = xQueueCreate(QUEUE_LENGTH, sizeof(lora_rx_packet_t));
    if (!lora_rx_packet_queue)
    {
        log_error("create lora_rx_packet_queue failed");
    }

    TaskHandle_t sending_task_handle;
    TaskHandle_t status_task_handle;

    BaseType_t ret = xTaskCreate(sending_task,
                                 "PFW_TASK",
                                 1024 * 8,
                                 NULL,
                                 1,
                                 &sending_task_handle);

    if (ret != pdPASS)
    {
        log_error("xTaskCreate failed: sending_task");
    }

    ret = xTaskCreate(status_task,
                      "STATUS_TASK",
                      128,
                      NULL,
                      1,
                      &status_task_handle);
    if (ret != pdPASS)
    {
        log_error("xTaskCreate failed: status_task");
    }
    #endif
}