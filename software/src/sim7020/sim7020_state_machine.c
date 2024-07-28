#include "sim7020_state_machine.h"
#include "sim7020.h"
#include "sim7020_hal.h"
#include "free_rtos_memory.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "log.h"

#include <string.h>

#define FOREACH_STATE(STATE)  \
    STATE(STATE_INITIALIZE)   \
    STATE(STATE_ENABLE_GPRS)  \
    STATE(STATE_GET_TIME)     \
    STATE(STATE_CONNECT)      \
    STATE(STATE_SENDING_DATA) \
    STATE(STATE_DISCONNECT)   \
    STATE(STATE_WAIT)         \
    STATE(STATE_ERROR)        \
    STATE(NUM_STATES)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum
{
    FOREACH_STATE(GENERATE_ENUM)
} state_t;
static state_t next_state;

static const char *STATE_STRING[] = {
    FOREACH_STATE(GENERATE_STRING)};

TaskHandle_t state_machine_task_handle;
#define STATE_MACHINE_TASK_STACK_SIZE_WORDS 1024
StackType_t state_machine_task_stack[STATE_MACHINE_TASK_STACK_SIZE_WORDS];
StaticTask_t state_machine_task_buffer;

static SemaphoreHandle_t state_machine_mutex;

/**
 * @brief data exchanged inside state machine. In case of modification from external mutex needed.
 */
typedef struct instance_data_s
{
    state_t current_state;
} instance_data_t;
static instance_data_t instance_data = {0};

typedef struct
{
    uint8_t *udp_message;
    uint32_t message_size;
} send_udp_msg_request_t;
static send_udp_msg_request_t udp_request;
static SemaphoreHandle_t udp_request_sem;

typedef struct
{
    bool result;
} send_udp_msg_response_t;
static send_udp_msg_response_t udp_response;
static SemaphoreHandle_t udp_response_sem;

typedef struct
{
    const char *ip;
    uint16_t port;
} connect_request_t;
static connect_request_t connect_request;
static SemaphoreHandle_t connect_request_sem;

typedef struct
{
    bool result;
} connect_response_t;
static connect_response_t connect_respone;
static SemaphoreHandle_t connect_response_sem;

typedef struct
{
    bool result;
} disconnect_response_t;
static connect_response_t disconnect_respone;
static SemaphoreHandle_t disconnect_request_sem;
static SemaphoreHandle_t disconnect_respone_sem;

typedef struct
{
    struct tm *time;
} get_time_request_t;
static get_time_request_t get_time_request;
typedef struct
{

    bool result;
} get_time_response_t;
static get_time_response_t get_time_response;
static SemaphoreHandle_t get_time_request_sem;
static SemaphoreHandle_t get_time_response_sem;

typedef state_t state_func_t(instance_data_t *data);

static state_t do_state_initialize(instance_data_t *data)
{
    instance_data.current_state = STATE_INITIALIZE;
    if (!sim7020_hal_uart_init())
    {
        log_error("sim7020_hal_uart_init failed");
        return STATE_ERROR;
    }
    if (!sim7020_hal_sim_gpio_init())
    {
        log_error("sim7020_hal_sim_gpio_init failed");
        return STATE_ERROR;
    }
    if (!sim7020_hal_enable_sim_module())
    {
        log_error("sim7020_hal_sim_gpio_init failed");
        return STATE_ERROR;
    }
    if (!sim7020_initialize_sim_module())
    {
        log_error("sim7020_hal_sim_gpio_init failed");
        return STATE_ERROR;
    }

    // state transition to:
    return STATE_ENABLE_GPRS;
}

static state_t do_state_enable_gprs(instance_data_t *data)
{
    state_t current_state = instance_data.current_state;
    if (current_state != STATE_INITIALIZE && current_state != STATE_ENABLE_GPRS)
    {
        log_error("Cant transition from %s to %s", STATE_STRING[current_state], STATE_STRING[STATE_ENABLE_GPRS]);
        return STATE_ERROR;
    }
    instance_data.current_state = STATE_ENABLE_GPRS;
    // check if connected, if not, wait for signall
    if (!sim7020_is_attached_gprs())
    {
        log_warn("GPRS not attached, retry");
        vTaskDelay(pdMS_TO_TICKS(5000));
        return STATE_ENABLE_GPRS;
    }
    if (!sim7020_is_PDP_active())
    {
        log_warn("PDP not active, retry");
        vTaskDelay(pdMS_TO_TICKS(5000));
        return STATE_ENABLE_GPRS;
    }
    // AT+COPS? // shows the operators in the network
    return STATE_WAIT;
}

static state_t do_state_get_time(instance_data_t *data)
{
    state_t current_state = instance_data.current_state;
    if (current_state != STATE_WAIT)
    {
        log_error("Cant transition from %s to %s", STATE_STRING[current_state], STATE_STRING[STATE_CONNECT]);
        get_time_response.result = false;
        xSemaphoreGive(get_time_response_sem);
        return STATE_ERROR;
    }
    instance_data.current_state = STATE_GET_TIME;

    if (!sim7020_get_time_ntp(get_time_request.time))
    {
        log_warn("sim7020_get_time_ntp failed");
        get_time_response.result = false;
    }
    else
    {
        get_time_response.result = true;
    }
    xSemaphoreGive(get_time_response_sem);
    return STATE_WAIT;
}

static state_t do_state_connecting(instance_data_t *data)
{
    state_t current_state = instance_data.current_state;
    if (current_state != STATE_ENABLE_GPRS && current_state != STATE_WAIT)
    {
        log_error("Cant transition from %s to %s", STATE_STRING[current_state], STATE_STRING[STATE_CONNECT]);
        connect_respone.result = false;
        xSemaphoreGive(connect_response_sem);
        return STATE_ERROR;
    }

    instance_data.current_state = STATE_CONNECT;

    /* if PDP is not active -> go to enable GPRS... */
    if (!sim7020_is_PDP_active())
    {
        log_warn("PDP not active, go to state  STATE_ENABLE_GPRS");
        connect_respone.result = false;
        xSemaphoreGive(connect_response_sem);
        return STATE_ENABLE_GPRS;
    }

#if 0 // not possible with transparent mode
    /* Already connected*/
    if (sim7020_is_connected())
    {
        connect_respone.result = false;
        xSemaphoreGive(connect_response_sem);
        return STATE_WAIT;
    }
#endif

    if (!sim7020_connect(connect_request.ip, connect_request.port))
    {
        log_error("sim7020_connect failed");
        connect_respone.result = false;
        xSemaphoreGive(connect_response_sem);
        return STATE_ERROR;
    }

    connect_respone.result = true;
    xSemaphoreGive(connect_response_sem);
    return STATE_WAIT;
}

static state_t do_state_sending_data(instance_data_t *data)
{
    state_t current_state = instance_data.current_state;
    if (current_state != STATE_CONNECT && current_state != STATE_SENDING_DATA && current_state != STATE_WAIT)
    {
        log_error("Cant transition from %s to %s", STATE_STRING[current_state], STATE_STRING[STATE_CONNECT]);
        udp_response.result = false;
        xSemaphoreGive(udp_response_sem);
        return STATE_ERROR;
    }

    instance_data.current_state = STATE_SENDING_DATA;
    if (sim7020_send_udp(udp_request.udp_message, udp_request.message_size))
    {
        udp_response.result = true;
    }
    else
    {
        log_error("sim7020_send_udp failed");
        udp_response.result = false;
    }
    xSemaphoreGive(udp_response_sem);
    return STATE_WAIT;
}

static state_t do_state_disconnecting(instance_data_t *data)
{
    state_t current_state = instance_data.current_state;
    if (current_state != STATE_CONNECT && current_state != STATE_WAIT)
    {
        log_warn("Cant transition from %s to %s", STATE_STRING[current_state], STATE_STRING[STATE_CONNECT]);
    }
    instance_data.current_state = STATE_DISCONNECT;
    if (!sim7020_disconnect())
    {
        log_warn("sim7020_disconnect failed");
        disconnect_respone.result = false;
    }
    xSemaphoreGive(disconnect_respone_sem);
    return STATE_WAIT;
}

static state_t do_state_error(instance_data_t *data)
{
    uint32_t delay_time_ms = 20000;
    state_t current_state = instance_data.current_state;
    log_error("Error in %s. Next state will be STATE_INITIALIZE in %d seconds", STATE_STRING[current_state], delay_time_ms / 1000);
    instance_data.current_state = STATE_ERROR;
    vTaskDelay(pdMS_TO_TICKS(delay_time_ms));
    return STATE_INITIALIZE;
}

static state_t do_state_wait(instance_data_t *data)
{
    instance_data.current_state = STATE_WAIT;
    if (pdTRUE == xSemaphoreTake(connect_request_sem, 0))
    {
        return STATE_CONNECT;
    }
    if (pdTRUE == xSemaphoreTake(udp_request_sem, 0))
    {
        return STATE_SENDING_DATA;
    }
    if (pdTRUE == xSemaphoreTake(disconnect_request_sem, 0))
    {
        return STATE_DISCONNECT;
    }
    if (pdTRUE == xSemaphoreTake(get_time_request_sem, 0))
    {
        return STATE_GET_TIME;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    return STATE_WAIT;
}

state_func_t *const state_table[NUM_STATES] = {
    do_state_initialize,
    do_state_enable_gprs,
    do_state_get_time,
    do_state_connecting,
    do_state_sending_data,
    do_state_disconnecting,
    do_state_wait,
    do_state_error,
};

state_t run_state(state_t cur_state, instance_data_t *data)
{
    return state_table[cur_state](data);
};

static void state_machine_task(void *pvParameters)
{
    next_state = STATE_INITIALIZE;
    instance_data.current_state = STATE_INITIALIZE;
    while (1)
    {

        log_info("next_state: %s", STATE_STRING[next_state]);
        next_state = run_state(next_state, &instance_data);
    }
}

bool internet_task_send_udp(uint8_t *message, uint32_t size)
{
    bool result = false;
    if (xSemaphoreTake(state_machine_mutex, pdMS_TO_TICKS(1000 * 10)) == pdTRUE)
    {

        udp_request.udp_message = message;
        udp_request.message_size = size;
        if (pdTRUE != xSemaphoreGive(udp_request_sem))
        {
            log_warn("xSemaphoreGive event_received_sem failed");
        }

        if (pdTRUE == xSemaphoreTake(udp_response_sem, pdMS_TO_TICKS(1000 * 10)))
        {
            result = udp_response.result;
        }
        else
        {
            log_warn("xSemaphoreTake wait_for_response_sem failed");
            result = false;
        }

        xSemaphoreGive(state_machine_mutex);
    }
    else
    {
        log_warn("xSemaphoreTake failed");
    }

    return result;
}

bool internet_task_connect(const char *dst_ip, uint16_t port)
{
    bool result = false;
    if (xSemaphoreTake(state_machine_mutex, pdMS_TO_TICKS(1000 * 10)) == pdTRUE)
    {
        connect_request.ip = dst_ip;
        connect_request.port = port;
        if (pdTRUE != xSemaphoreGive(connect_request_sem))
        {
            log_warn("xSemaphoreGive connect_request_sem failed");
        }

        if (pdTRUE == xSemaphoreTake(connect_response_sem, pdMS_TO_TICKS(1000 * 10)))
        {
            result = connect_respone.result;
        }
        else
        {
            log_warn("xSemaphoreTake connect_request_sem failed");
            result = false;
        }
        xSemaphoreGive(state_machine_mutex);
    }
    else
    {
        log_error("failed to get state_machine_mutex");
    }
    return result;
}

bool internet_task_disconnect(void)
{
    bool result = false;
    if (xSemaphoreTake(state_machine_mutex, pdMS_TO_TICKS(1000 * 10)) == pdTRUE)
    {
        if (pdTRUE != xSemaphoreGive(disconnect_request_sem))
        {
            log_warn("xSemaphoreGive disconnect_request_sem failed");
        }
        if (pdTRUE == xSemaphoreTake(disconnect_respone_sem, pdMS_TO_TICKS(1000 * 10)))
        {
            result = connect_respone.result;
        }
        else
        {
            log_warn("xSemaphoreTake disconnect_respone_sem failed");
            result = false;
        }
        xSemaphoreGive(state_machine_mutex);
    }
    return result;
}

bool internet_task_get_time(struct tm *time)
{
    bool result = false;
    if (xSemaphoreTake(state_machine_mutex, pdMS_TO_TICKS(1000 * 10)) == pdTRUE)
    {
        get_time_request.time = time;
        if (pdTRUE != xSemaphoreGive(get_time_request_sem))
        {
            log_warn("xSemaphoreGive get_time_request_sem failed");
        }
        if (pdTRUE == xSemaphoreTake(get_time_response_sem, pdMS_TO_TICKS(1000 * 60)))
        {
            result = get_time_response.result;
        }
        else
        {
            log_error("xSemaphoreTake get_time_response_sem failed");
            result = false;
        }
        xSemaphoreGive(state_machine_mutex);
    }
    else
    {
        log_error("Cant get state_machine_mutex");
    }
    return result;
}

#define CREATE_SEMAPHORE(name)                                \
    name = xSemaphoreCreateBinary();                          \
    if (name == NULL)                                         \
    {                                                         \
        log_error("xSemaphoreCreateBinary failed: %s", name); \
        return false;                                         \
    }

void internet_task_print_task_stats(void)
{
    free_rtos_memory_print_usage(state_machine_task_handle, "internet_task", STATE_MACHINE_TASK_STACK_SIZE_WORDS * sizeof(UBaseType_t));
}

bool internet_task_init()
{
    next_state = STATE_INITIALIZE;

    state_machine_mutex = xSemaphoreCreateMutex();
    if (!state_machine_mutex)
    {
        log_error("xSemaphoreCreateMutex failed: state_machine_mutex");
        return false;
    }

    CREATE_SEMAPHORE(udp_request_sem);
    CREATE_SEMAPHORE(udp_response_sem);
    CREATE_SEMAPHORE(connect_request_sem);
    CREATE_SEMAPHORE(connect_response_sem);
    CREATE_SEMAPHORE(disconnect_request_sem);
    CREATE_SEMAPHORE(disconnect_respone_sem);
    CREATE_SEMAPHORE(get_time_request_sem);
    CREATE_SEMAPHORE(get_time_response_sem);

    state_machine_task_handle = xTaskCreateStatic(state_machine_task,
                                                  "STATE_MACHINE_TASK",
                                                  STATE_MACHINE_TASK_STACK_SIZE_WORDS,
                                                  NULL,
                                                  1,
                                                  state_machine_task_stack,
                                                  &state_machine_task_buffer);

    if (state_machine_task_handle == NULL)
    {
        log_error("xTaskCreate failed: sending_task");
        return false;
    }

    if (!sim7020_hal_init())
    {
        log_error("sim7020_hal_init failed");
        return false;
    }
}
