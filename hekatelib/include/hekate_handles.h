#ifndef A0004072_350F_4D20_A00E_CD99510F1040
#define A0004072_350F_4D20_A00E_CD99510F1040


#include "stdint.h"


typedef void (*sleep_us_t)(uint32_t us);
typedef uint64_t (*get_time_ms_t)();
typedef int (*printf_t)(const char * restrict, ...);

/**
 * @brief hardware abstraction layer for system specific functions
 * 
 */
typedef struct sys_hal {
    sleep_us_t sleep_us;
    get_time_ms_t get_time_ms;
    printf_t print;
} sys_hal_t;


/**
 * @brief hardware abstraction layers for the hekate gateway.
 * 
 */
typedef struct hekate_hal {
    sys_hal_t sys_hal;
    //gateway send
} hekate_hal_t;


#endif /* A0004072_350F_4D20_A00E_CD99510F1040 */
